////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly tileful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#include <QMessageBox>
#include <QMenu>
#include <QKeyEvent>
#include <QApplication>

#include "model/makers/tiling_maker.h"
#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/backgroundimageview.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_magnitude.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "sys/engine/image_engine.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"
#include "sys/sys/load_unit.h"

using std::make_shared;

TilingMaker::TilingMaker()
{
    //qDebug() << "TilingMaker::TilingMaker";
    propagate = true;
}

TilingMaker::~TilingMaker()
{
}

void TilingMaker::init()
{
    viewControl     = Sys::viewController;
    prototypeMaker  = Sys::prototypeMaker;
    maped           = Sys::mapEditor;
    tmView          = Sys::tilingMakerView;
    controlPanel    = Sys::controlPanel;
    config          = Sys::config;

    tmView->setMaker(this);

    connect(this, &TilingMaker::sig_updateView,      Sys::view,         &View::slot_update);
    connect(this, &TilingMaker::sig_reconstructView, viewControl,       &ViewController::slot_reconstructView);
    connect(this, &TilingMaker::sig_raiseMenu,       Sys::controlPanel, &ControlPanel::slot_raisePanel);

    tilingMakerMouseMode = TM_NO_MOUSE_MODE;
    poly_side_count      = config->polySides;
    poly_rotation        = 0.0;
    vectorState          = VEC_READY;

    unload();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    sm_takeUp(tiling,TILM_LOAD_EMPTY);
}

void TilingMaker::unload()
{
    eraseTilings();
}

void TilingMaker::eraseTilings()
{
    tilings.clear();
    selectedTiling.reset();
    currentPlacedTile.reset();
    unifyBase.reset();
    clickedSelector.reset();
    tmStack.clear();
}

TilingPtr TilingMaker::loadTiling(VersionedFile vfile, eTILM_Event event)
{
    qDebug().noquote() << "TilingMaker::loadTiling" << vfile.getVersionedName().get();

    Sys::debugView->clear();

    Sys::loadUnit->setLoadState(LOADING_TILING,vfile);

    TilingManager tm;
    TilingPtr tiling = tm.loadTiling(vfile,event);

    useLoadedTiling(tiling,vfile,event);

    return tiling;
}

void TilingMaker::useLoadedTiling(TilingPtr tiling, VersionedFile & vfile, eTILM_Event event)
{
    if (tiling)
    {
        CanvasSettings csettings = tiling->getCanvasSettings();
        QSize size  = csettings.getViewSize();

        // tiling is loaded, now use it
        switch(event)
        {
        case TILM_LOAD_SINGLE:
        case TILM_LOAD_MULTI:
        case TILM_RELOAD:
        case TILM_LOAD_FROM_STEPPER:
        {
            Canvas & canvas = viewControl->getCanvas();
            canvas.setModelAlignment(M_ALIGN_TILING);
            canvas.initCanvasSize(csettings.getCanvasSize());
            sm_takeUp(tiling, event);
        }   break;

        case TILM_LOAD_FROM_MOSAIC:
            break;

        case TILM_LOAD_EMPTY:
        {
            Canvas & canvas = viewControl->getCanvas();
            canvas.setModelAlignment(M_ALIGN_TILING);
        }   break;
        }

        if (config->splitScreen)
        {
            viewControl->setFixedSize(size);
        }
        else
        {
            viewControl->setSize(size);
        }

        viewControl->removeAllImages();
        emit sig_reconstructView();
        emit sig_tilingLoaded(vfile);
    }

    Sys::loadUnit->resetLoadState();
}

void TilingMaker::saveTiling(TilingPtr tiling)
{
    TilingManager tm;

    tm.saveTiling(tiling); // Tiling Manager haandles OK/FAIL status
    _lastSelectedData = tiling->getDataCopy();
    _hasChanged = false;
}

QString TilingMaker::getStatus()
{
    QString s = sTilingMakerMouseMode[getTilingMakerMouseMode()];

    auto maction = tmView->getMouseInteraction();
    if (maction)
    {
        s += " ";
        s += maction->desc;
    }

    s += QString("  in_tiling: %1  all: %2" ).arg(selectedTiling->numIncluded()).arg(selectedTiling->numAll());
    s += tmStack.getStackStatus();
    return s;
}

TilingPtr TilingMaker::findTilingByName(QString name)
{
    for (const auto & tiling : std::as_const(tilings))
    {
        if (tiling->getName().get() == name)
        {
            return tiling;
        }
    }
    TilingPtr tp;
    return tp;
}

bool  TilingMaker::isLoaded(QString name)
{
    TilingPtr tp = findTilingByName(name);
    if (tp)
        return true;
    else
        return false;
}

bool TilingMaker::isValidTiling(TilingPtr tiling)
{
    return tilings.contains(tiling);
}

// called by menu to clear tiling
void TilingMaker::removeTiling(TilingPtr tp)
{
    qDebug().noquote() << __FUNCTION__ << tp->info();
    tilings.removeOne(tp);
    if (tp == selectedTiling)
    {
        if (tilings.size() == 0)
        {
            // there is always at least an emtpy tiling
            TilingPtr tiling = make_shared<Tiling>();
            sm_takeUp(tiling,TILM_LOAD_EMPTY);
        }
        else
        {
            select(tilings.first());
        }
    }
}

// call this when Prototype needs to be rebuilt
void TilingMaker::pushTilingToPrototypeMaker(ePROM_Event event)
{
    qDebug().noquote() << __FUNCTION__ << sPROM_Events[event];
    if (propagate)
        prototypeMaker->sm_takeUp(selectedTiling, event);
}

void TilingMaker::pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile)
{
    qDebug().noquote() << __FUNCTION__ << sPROM_Events[event] << tile->info();
    if (propagate)
        prototypeMaker->sm_takeUp(selectedTiling,event,tile);
}

/* there are three possible actions
   1. reset all and add
   2. erase current and add
   3. add
   (future - delete)
*/

void TilingMaker::sm_resetAllAndAdd(TilingPtr tiling)
{
   eraseTilings();
   tilings.push_front(tiling);
   select(tiling);
}

void TilingMaker::sm_resetCurrentAndAdd(TilingPtr tiling)
{
    int i = tilings.indexOf(selectedTiling);
    tilings.replace(i,tiling);
    select(tiling);
}

void TilingMaker::sm_add(TilingPtr tiling)
{
    tilings.push_front(tiling);
    select(tiling);
}

void TilingMaker::sm_takeDown(QVector<TilingPtr> & proto_tilings, eTILM_Event event)
{
    qDebug() << "TilingMaker::sm_takeDown tilings=" << proto_tilings.size() << sTILM_Events[event];
    switch (event)
    {
    case TILM_LOAD_FROM_MOSAIC:
        tilings = proto_tilings;
        if (tilings.count())
        {
            auto tiling = tilings.first();
            Q_ASSERT(tiling);
            select(tiling);
            slot_stack_save();
        }
        break;

    default:
        qCritical("bad call to TilingMaker::sm_takeDown");
    }

    if (config->tm_loadFill)
    {
        tmView->setFill(true);
    }
}

void TilingMaker::sm_takeUp(TilingPtr tiling, eTILM_Event event)
{
    eTMState state = sm_getState();
    qDebug().noquote() << "TilingMaker::sm_takeUp state:" << tm_states[state]  << "event:" <<  sTILM_Events[event];

    auto bip   = tiling->getBkgdImage();
    auto bview = Sys::backgroundImageView;
    if (bip)
    {
        bview->setImage(bip);
        bview->setModelXform(bip->getImageXform(),false);
    }

    switch(event)
    {
    case TILM_LOAD_EMPTY:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        prototypeMaker->sm_takeUp(tiling, PROM_LOAD_EMPTY);
        break;

    case TILM_LOAD_SINGLE:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        prototypeMaker->sm_takeUp(tiling, PROM_LOAD_SINGLE);
        slot_stack_save();
        break;

    case TILM_LOAD_MULTI:
        clearConstructionLines();
        if (state == TM_EMPTY)
        {
            sm_resetAllAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_LOAD_SINGLE);
        }
        else if (state == TM_SINGLE)
        {
            sm_add(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_LOAD_MULTI);
        }
        else if (state == TM_MULTI)
        {
            sm_add(tiling);
            prototypeMaker->sm_takeUp(tiling,PROM_LOAD_MULTI);
        }
        slot_stack_save();
        break;

    case TILM_RELOAD:
        clearConstructionLines();
        if (state == TM_EMPTY)
        {
            qWarning("Trying to modify when tiling not loaded");
            sm_takeUp(tiling,TILM_LOAD_SINGLE);
        }
        else if (state == TM_SINGLE)
        {
            sm_resetAllAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_RELOAD_SINGLE);
        }
        else if (state == TM_MULTI)
        {
            sm_resetCurrentAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling,PROM_RELOAD_MULTI);
        }
        slot_stack_save();
        break;

    case TILM_LOAD_FROM_STEPPER:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        break;

    case TILM_LOAD_FROM_MOSAIC:
        qCritical("use takeDown for TILM_LOAD_FROM_MOSAIC");
        break;
    }

    if (config->tm_loadFill)
    {
        tmView->setFill(true);
    }
}

void TilingMaker::select(TilingPtr tiling, bool force)
{
    if (!force && (tiling == selectedTiling))
       return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling:" << tiling->getName().get();

    clearMakerData();

    setupMaker(tiling);

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        tmView->forceLayerRecalc(true);
    }

    selectedTiling    = tiling;
    _lastSelectedData = tiling->getDataCopy();
    _hasChanged    = false;
#if 0
    ProtoPtr proto = prototypeMaker->getPrototype(tiling);
    if (proto)
    {
        prototypeMaker->setSelectedPrototype(proto);
    }
#endif
}

bool TilingMaker::tilingChanged()
{
    if (!selectedTiling)
        return false;
    if (_hasChanged)
        return true;
    _hasChanged = !(selectedTiling->getData() == _lastSelectedData);
    return _hasChanged;
}

eTMState TilingMaker::sm_getState()
{
    if (tilings.size() > 1)
    {
        return  TM_MULTI;
    }
    else if (tilings.size() == 0)
    {
        return TM_EMPTY;
    }
    else
    {
        Q_ASSERT(tilings.size() == 1);
        return TM_SINGLE;
    }
}

bool TilingMaker::sm_askAdd()
{
    // warn
    QMessageBox box(Sys::controlPanel);
    box.setIcon(QMessageBox::Question);
    box.setText("Loading a tiling in multi-mode adds an addtional tiling. Is this what you want?");
    QPushButton *addButton     = box.addButton("YES - Continue to add another tiling", QMessageBox::ActionRole);
    QPushButton *replaceButton = box.addButton("NO  - Replace existing tiling", QMessageBox::ActionRole);
    Q_UNUSED(replaceButton);
    box.exec();

    bool add = (box.clickedButton() == addButton);
    return add;
}

void TilingMaker::clearMakerData()
{
    qDebug() << "TilingMaker::clearMakerData";

    selectedTiling.reset();

    resetCurrentPlacedTile();
    clickedSelector.reset();

    tilingMakerMouseMode    = TM_NO_MOUSE_MODE;

    tmView->clearViewData();
}

void TilingMaker::removeExcludeds()
{
    selectedTiling->removeExcludeds();
}

void TilingMaker::setupMaker(TilingPtr tiling)
{
    tmView->setTiling(tiling);
    
    auto tilingCanvasSize = tiling->getData().getCanvasSettings().getCanvasSize();

    qDebug() << "TilingMaker::setupMaker" << tiling->getName().get() << tilingCanvasSize;
    
    viewControl->getCanvas().initCanvasSize(tilingCanvasSize);

    Sys::setTilingChange();
}

bool TilingMaker::verifyTiling()
{
    TilingPtr tp = getSelected();

    if (tp->numAll() == 0)
    {
        qWarning("There are no polygons");
        return false;
    }

    if (tp->numIncluded() == 0)
    {
        qWarning("No included tiles");
        return false;
    }

    const FillData & fd = tp->getCanvasSettings().getFillData();
    int minX, maxX, minY, maxY;
    bool singleton;
    fd.get(singleton,minX, maxX, minY, maxY);

    if (!singleton)
    {
        if (tp->getData().getTrans1().isNull() || tp->getData().getTrans2().isNull())
        {
            qWarning("Translation vectors not defined");
            return false;
        }
    }

    return true;
}

void TilingMaker::setTilingMakerMouseMode(eTilingMakerMouseMode mode)
{
    tilingMakerMouseMode = mode;

    if (mode == TM_EDIT_TILE_MODE)
    {
        tmView->setEditPlacedTile(currentPlacedTile);
    }
    else
    {
        tmView->resetEditPlacedTile();
    }

    if (mode == TM_TRANSLATION_VECTOR_MODE)
    {
        clearTranslationVectors();
        vectorState = VEC_SETTING_T1;
    }

    unifyBase.reset();
    tmView->getAccumW().clear();

    forceRedraw();
}

eTilingMakerMouseMode TilingMaker::getTilingMakerMouseMode()
{
    return tilingMakerMouseMode;
}

/////////////////////////////////////////////////
//
// Tile Management
//
/////////////////////////////////////////////////

TilingPlacements TilingMaker::getTilingUnitPlacements() const
{
    return selectedTiling->getTilingUnitPlacements();
}

void TilingMaker::addNewPlacedTile(PlacedTilePtr placedTile)
{
    addPlacedTile(placedTile);        // 03AUG21 - additions are always placed in tiling (they can be removed)
    forceRedraw();
    emit sig_buildMenu();
}

// called by MapEditor with placed tiles created from a map
// there is no possibility of a merge. editing of single tiles is done
// directly here in the TilingMaker.  These are placed tiles made by
// extracting faces from a map, and then each face becomes a placed tile
void TilingMaker::replaceTilingUnit(TilingUnit & tilingUnit)
{
    // remove all tilings except the current
    qDebug() << __FUNCTION__ << "using" << tilingUnit.numAll() << "placed tiles";

    QVector<TilingPtr> todelete;
    TilingPtr current = selectedTiling; // current can change
    for (auto & tiling : tilings)
    {
        if (tiling != current)
            todelete.push_back(tiling);
    }
    for (auto & tiling : todelete)
    {
        removeTiling(tiling);
        prototypeMaker->sm_takeUp(tiling,PROM_TILING_DELETED);
    }
    Q_ASSERT(current = selectedTiling);

    // replace the placed tile
    selectedTiling->replaceTilingUnit(tilingUnit);

    // it's a new ball game
    prototypeMaker->sm_takeUp(selectedTiling,PROM_LOAD_SINGLE);

    forceRedraw();
    emit sig_buildMenu();
}

PlacedTileSelectorPtr TilingMaker::addTileSelectionPointer(PlacedTileSelectorPtr sel)
{
    PlacedTileSelectorPtr ret;
    if (!sel)
        return ret;

    PlacedTilePtr pf    = sel->getPlacedTile();
    if (!pf)
        return ret;

    PlacedTilePtr pfnew = make_shared<PlacedTile>(pf->getTile(), pf->getTransform());
    addNewPlacedTile(pfnew);
    
    switch (sel->getType())
    {
    case ARC_POINT:
    case TILE_CENTER:
    case SCREEN_POINT:
        break;
    case INTERIOR:
        ret = make_shared<InteriorTilleSelector>(pfnew);
        break;
    case EDGE:
        ret = make_shared<EdgeTileSelector>(pfnew,sel->getModelEdge());
        break;
    case VERTEX:
        ret = make_shared<VertexTileSelector>(pfnew,sel->getModelPoint());
        break;
    case MID_POINT:
        ret = make_shared<MidPointTileSelector>(pfnew,sel->getModelEdge(),sel->getModelPoint());
        break;
    }

    emit sig_buildMenu();

    return ret;
}

void  TilingMaker::setCurrentPlacedTile(PlacedTilePtr pfp)
{
    currentPlacedTile = pfp;
    emit sig_current_tile(pfp);
}

void TilingMaker::slot_deleteTile()
{
    if (clickedSelector)
    {
        deleteTile(clickedSelector);
        clickedSelector.reset();
    }
    pushTilingToPrototypeMaker(PROM_TILES_DELETED);
}

void TilingMaker::deleteTile(PlacedTileSelectorPtr sel)
{
    if (!sel) return;

    PlacedTilePtr tile = sel->getPlacedTile();
    auto tselect = tmView->getTileSelector();
    if (tselect &&  tselect->getPlacedTile() == tile)
    {
        tmView->resetTileSelector();
    }

    if (tile)
    {
        deletePlacedTile(tile);

        emit sig_buildMenu();
        forceRedraw();
    }
    pushTilingToPrototypeMaker(PROM_TILES_DELETED);
}

void TilingMaker::deletePlacedTile(PlacedTilePtr pf)
{
    if (pf == currentPlacedTile)
    {
        resetCurrentPlacedTile();
    }
    selectedTiling->removePlacedTile(pf);
    forceRedraw();
    pushTilingToPrototypeMaker(PROM_TILES_DELETED);
}

void TilingMaker::unifyTile(PlacedTilePtr pf)
{
    if (!getTilingUnitPlacements().contains(pf))
        return;

    if (!unifyBase)
    {
        // start
        unifyBase = pf;
        return;
    }

    // make new tile
    QTransform transform = pf->getTransform();
    TilePtr    tile      = unifyBase->getTile();
    PlacedTilePtr newPT  = make_shared<PlacedTile>(tile,transform);

    // remove old, add new
    deletePlacedTile(pf);
    addNewPlacedTile(newPT);
}

void TilingMaker::addPlacedTile(PlacedTilePtr pf)
{
    selectedTiling->addPlacedTile(pf);
    pushTilingToPrototypeMaker(PROM_TILES_ADDED);
}

void TilingMaker::removePlacedTile(PlacedTilePtr pf)
{
    selectedTiling->removePlacedTile(pf);
}

void TilingMaker::duplicateSelectedTiling()
{
    TilingPtr tp = make_shared<Tiling>();
    tp->copy(selectedTiling);

    VersionedName vname = FileServices::getNextVersion(FILE_TILING,tp->getName());
    tp->setName(vname);

    VersionedFile vf;
    vf.updateFromVersionedName(vname);

    useLoadedTiling(tp,vf,TILM_LOAD_MULTI); // calls sm_add which calls select which sets _last used
    _hasChanged = true;                     // needs to be saved
}

void TilingMaker::updateVectors()
{
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);

    MapEditorDb * db = maped->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        maped->loadTilingRepeated();
    }
}

void TilingMaker::updateReps()
{
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);

    MapEditorDb * db = maped->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        maped->loadTilingRepeated();
    }
}

void TilingMaker::resetOverlaps()
{
   selectedTiling->resetOverlaps();  // sets UNDEFINED
}

void TilingMaker::slot_showTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        pf->setShow(true);
        emit sig_buildMenu();
        forceRedraw();
        clickedSelector.reset();
    }
}

void TilingMaker::slot_hideTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        pf->setShow(false);
        emit sig_buildMenu();
        forceRedraw();
        clickedSelector.reset();
    }
}

void TilingMaker::slot_includeTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        if (!pf->isIncluded())
        {
            pf->include();
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILES_ADDED);
        }
    }
}

void TilingMaker::slot_excludeTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        if (pf->isIncluded())
        {
            pf->exclude();
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILES_DELETED);
        }
    }
}

void TilingMaker::slot_editTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pfp = clickedSelector->getPlacedTile();
        TilePtr      tile = pfp->getTile();
        QTransform      t = pfp->getTransform();

        DlgEdgePolyEdit * fe  = new DlgEdgePolyEdit(tile ,t);
        fe->show();
        fe->raise();
        fe->activateWindow();

        connect(fe,   &DlgEdgePolyEdit::sig_currentPoint, tmView, &TilingMakerView::slot_setTileEditPoint);
        connect(this, &TilingMaker::sig_refreshMenu,      fe,     &DlgEdgePolyEdit::display);
    }
}

void TilingMaker::slot_copyMoveTile()
{
    if (clickedSelector)
    {
        tmView->setMouseInteraction(make_shared<CopyMovePolygon>(clickedSelector, clickedSpt));
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinPoint()
{
    if (clickedSelector)
    {
        tmView->setMouseInteraction(make_shared<CopyJoinPoint>(clickedSelector, clickedSpt));
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinMidPoint()
{
    if (clickedSelector)
    {
        tmView->setMouseInteraction(make_shared<CopyJoinMidPoint>(clickedSelector, clickedSpt));
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}
void TilingMaker::slot_copyJoinEdge()
{
    if (clickedSelector)
    {
        tmView->setMouseInteraction(make_shared<CopyJoinEdge>(clickedSelector, clickedSpt));
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        TilePtr tile2 = tile->recreate();  // creates a new tile same as other
        pf->setTile(tile2);
        emit sig_buildMenu();
        pushTilingToPrototypeMaker(PROM_TILES_ADDED);
    }
}

void TilingMaker::slot_convertTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        flipTileRegularity(tile);
        emit sig_buildMenu();
    }
}

void TilingMaker::slot_debugCompose()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        tile->compose();
        emit sig_buildMenu();
        forceRedraw();
        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }
}

void TilingMaker::slot_debugDecompose()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        tile->decompose();
        emit sig_buildMenu();
        forceRedraw();
        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }
}

void TilingMaker::flipTileRegularity(TilePtr tile)
{
    tile->flipRegularity();
    pushTileToPrototypeMaker(PROM_TILE_REGULARITY_CHANGED,tile);
}

void TilingMaker::tilingDeltaX(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    TilingPlacements tilingUnit = selectedTiling->getTilingUnitPlacements(true);
    for (const auto & pfp : std::as_const(tilingUnit))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaY(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    TilingPlacements tilingUnit = selectedTiling->getTilingUnitPlacements(true);
    for (const auto & pfp : std::as_const(tilingUnit))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaScale(int delta)
{
    Q_ASSERT(Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING));
    qreal scale = 1.0 + (0.01 * delta);

    TilingPlacements tilingUnit = selectedTiling->getTilingUnitPlacements(true);
    for (const auto & pfp : std::as_const(tilingUnit))
    {
        QTransform t = pfp->getTransform();
        qDebug() << "t0" << Transform::info(t);
        QTransform t1 = t.scale(scale,scale);

        t = pfp->getTransform();
        QTransform t2 = t *QTransform::fromScale(scale,scale);

        qDebug() << "t1" << Transform::info(t1);
        qDebug() << "t2" << Transform::info(t2);

        t = pfp->getTransform();
        // scales position too
        t *= QTransform::fromScale(scale,scale);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;

    TilingPlacements tilingUnit = selectedTiling->getTilingUnitPlacements(true);
    for (const auto & pfp : std::as_const(tilingUnit))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaX(qreal delta)
{
    if (!currentPlacedTile)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedTile->getTransform();
    t *= QTransform::fromTranslate(qdelta,0.0);
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaY(qreal delta)
{
    if (!currentPlacedTile)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedTile->getTransform();
    t *= QTransform::fromTranslate(0.0,qdelta);
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileSetTranslate(qreal x, qreal y)
{
    if (!currentPlacedTile)
        return;

    QTransform t = currentPlacedTile->getTransform();

    Xform xf(t);
    xf.setTranslateX(x);
    xf.setTranslateY(y);
    t = xf.toQTransform(QTransform());
    currentPlacedTile->setTransform(t);
}

void TilingMaker::placedTileDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);
    placedTileDeltaScale(scale);
}

void TilingMaker::placedTileDeltaScale(qreal scale)
{
    if (!currentPlacedTile)
        return;

    QPolygonF pts = currentPlacedTile->getPlacedPoints();
    QPointF center = Geo::center(pts);

    QTransform ts;
    ts.scale(scale,scale);
    Xform xf(ts);
    xf.setModelCenter(center);
    ts = xf.toQTransform(QTransform());

    QTransform t = currentPlacedTile->getTransform();
    t *= ts;
    qDebug() << Transform::info(t);
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileSetScale(qreal scale)
{
    if (!currentPlacedTile)
        return;

    QTransform t = currentPlacedTile->getTransform();

    Xform xf(t);
    xf.setScale(scale);
    t = xf.toQTransform(QTransform());
    currentPlacedTile->setTransform(t);
}

void TilingMaker::placedTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    placedTileDeltaRotate(qdelta);
}

void TilingMaker::placedTileDeltaRotate(qreal rotate)
{
    if (!currentPlacedTile)
        return;

    QPolygonF pts = currentPlacedTile->getPlacedPoints();
    QPointF center = Geo::center(pts);

    Xform xf(QTransform().rotate(rotate));
    xf.setModelCenter(center);
    QTransform tr = xf.toQTransform(QTransform());

    QTransform t = currentPlacedTile->getTransform();
    t *= tr;
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileSetRotate(qreal rotate)
{
    if (!currentPlacedTile)
        return;

    QTransform t = currentPlacedTile->getTransform();

    Xform xf(t);
    xf.setRotateDegrees(rotate);
    t = xf.toQTransform(QTransform());
    currentPlacedTile->setTransform(t);
}

void TilingMaker::uniqueTileDeltaScale(int delta)
{
    qreal scale = 0.01 * delta;
    uniqueTileDeltaScale(scale);
}

void TilingMaker::uniqueTileDeltaScale(qreal scale)
{
    if (!currentPlacedTile)
        return;

    TilePtr tile = currentPlacedTile->getTile();

    if (!tile)
        return;

    tile->deltaScale(scale);
    emit sig_refreshMenu();
}

void TilingMaker::uniqueTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    uniqueTileDeltaRotate(qdelta);
}

void TilingMaker::uniqueTileDeltaRotate(qreal rotate)
{
    if (!currentPlacedTile)
        return;

    TilePtr tile = currentPlacedTile->getTile();
    if (!tile)
        return;

    tile->deltaRotation(rotate);
    emit sig_refreshMenu();
}


////////////////////////////////////////////////////////////////////////////
//
// Tiling translation vector.
//
// We treat the two vector as a circular buffer so that each one will
// get over-written in turn, so that both can be alternatively changed
// by the end-user.
// Casper: given this reasoning a sheet of writing paper is a circlular buffer
// IMHO he was thinking of a ping-pong

void TilingMaker::addToTranslate(QLineF mLine, QPointF origin)
{
    QPointF tran = mLine.p2() - mLine.p1();

    QPointF t1 = selectedTiling->getData().getTrans1();
    QPointF t2 = selectedTiling->getData().getTrans2();

    Q_ASSERT(vectorState != VEC_READY);
    if (vectorState == VEC_SETTING_T1)
    {
        t1 = tran;
        vectorState = VEC_SETTING_T2;
    }
    else if (vectorState == VEC_SETTING_T2)
    {
        t2 = tran;
        vectorState = VEC_READY;
        setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
    }

    selectedTiling->setTranslationVectors(t1,t2,origin);

    emit sig_refreshMenu();
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
}

void TilingMaker::toggleInclusion(PlacedTileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTilePtr pf = sel->getPlacedTile();
        if (!pf) return;
        if (pf->isIncluded())
        {
            pf->exclude();
            emit sig_buildMenu();
            forceRedraw();
            pushTilingToPrototypeMaker(PROM_TILES_DELETED);
        }
        else
        {
            pf->include();
            emit sig_buildMenu();
            forceRedraw();
            pushTilingToPrototypeMaker(PROM_TILES_ADDED);
        }
    }
}

void TilingMaker::clearTranslationVectors()
{
    if (selectedTiling)
    {
        selectedTiling->setTranslationVectors(QPointF(),QPointF(),QPointF());
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::updatePolygonSides(int number)
{
    poly_side_count   = number;
    config->polySides = number;
}

void TilingMaker::updatePolygonRot(qreal angle)
{
    poly_rotation = angle;
}

void TilingMaker::addRegularPolygon()
{
    TilePtr f;
    if (poly_side_count > 2)
    {
        f = make_shared<Tile>(poly_side_count,poly_rotation);
    }
    else
    {
        Circle c(QPointF(0,0), 1.0);
        EdgePoly ep;
        ep.set(c);

        f = make_shared<Tile>(ep,poly_rotation,1.0);
    }

    QTransform t;
    addNewPlacedTile(make_shared<PlacedTile>(f,t));
    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::mirrorPolygonX(PlacedTileSelectorPtr sel )
{
    if (sel)
    {
        PlacedTilePtr ptp = sel->getPlacedTile();
        if (!ptp) return;

        auto tile     = ptp->getTile();
        EdgePoly & ep = tile->getEdgePolyRW();
        QTransform t  = QTransform::fromScale(-1,1);
        ep.mapD(t);
        tile->decompose();

        pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tile);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::mirrorPolygonY(PlacedTileSelectorPtr sel )
{
    if (sel)
    {
        PlacedTilePtr ptp = sel->getPlacedTile();
        if (!ptp) return;

        auto tile     = ptp->getTile();
        EdgePoly & ep = tile->getEdgePolyRW();
        QTransform t   = QTransform::fromScale(1,-1);
        ep.mapD(t);
        tile->decompose();

        pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tile);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

bool TilingMaker::reflectPolygon(PlacedTileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTileSelectorPtr sel2 = tmView->findEdge(tmView->getMousePos());
        if (sel2)
        {
            if (sel2->getType() == MID_POINT || sel2->getType() == EDGE)
            {
                PlacedTilePtr ptp = sel->getPlacedTile();
                auto tile         = ptp->getTile();
                EdgePoly & ep     = tile->getEdgePolyRW();
                QPolygonF poly    = ep.getPoly();
                QLineF line       = sel2->getModelLine();
                QPolygonF poly2   = Geo::reflectPolygon(poly,line);
                ep.set(poly2);
                tile->decompose();

                pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tile);
                forceRedraw();
                emit sig_refreshMenu();
                return true;
            }
            else
            {
                qInfo() << "reflection ignored - edge type = " << sel->getTypeString();
            }
        }
        else
        {
            qInfo() << "reflection ignored - not an edge";
        }
    }
    return false;
}

void TilingMaker::copyPolygon(PlacedTileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTilePtr pf = sel->getPlacedTile();
        addNewPlacedTile(make_shared<PlacedTile>(pf->getTile(), pf->getTransform())); // pushes
        forceRedraw();
        emit sig_buildMenu();
    }
}

void TilingMaker::forceRedraw()
{
    if (Sys::view->isActiveLayer(VIEW_TILING_MAKER))
    {
        emit sig_updateView();
    }
}

void TilingMaker::slot_createCurve()
{
    auto tsel    = tmView->getTileSelector();
    EdgePtr ep   = tsel->getModelEdge();
    ep->convertToConvexCurve();     // defaults to convex

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    ptile->getTile()->decompose();
    forceRedraw();
    qDebug() << "edge converted to CURVE";
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

void TilingMaker::slot_flatenCurve()
{
    auto tsel  = tmView->getTileSelector();
    EdgePtr ep = tsel->getModelEdge();
    ep->resetCurveToLine();

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    ptile->getTile()->decompose();
    forceRedraw();
    qDebug() << "edge converted to LINE";
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

void TilingMaker::slot_makeConvex()
{
    auto tsel  = tmView->getTileSelector();
    EdgePtr ep = tsel->getModelEdge();
    ep->setConvex(true);
    ep->getArcData().calcSpan(ep.get());

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    ptile->getTile()->decompose();
    forceRedraw();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

void TilingMaker::slot_makeConcave()
{
    auto tsel  = tmView->getTileSelector();
    EdgePtr ep = tsel->getModelEdge();
    ep->setConvex(false);
    ep->getArcData().calcSpan(ep.get());

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    ptile->getTile()->decompose();
    forceRedraw();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

void TilingMaker::slot_moveArcCenter()
{
    auto tsel  = tmView->getTileSelector();
    tmView->setMouseInteraction(make_shared<EditEdge>(tsel,QPointF()));
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

void TilingMaker::slot_editMagnitude()
{
    auto tsel  = tmView->getTileSelector();
    tmView->resetMouseInteraction();
    DlgMagnitude dlg(tsel);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tsel->getPlacedTile()->getTile());
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool TilingMaker::procKeyEvent(QKeyEvent * k)
{
    if (!viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        return false;
    }

    switch (k->key())
    {
        // actions
        case 'A': addRegularPolygon(); break;
        case 'C': addTileSelectionPointer(tmView->findTileUnderMouse()); break;
        case 'D': deleteTile(tmView->findTileUnderMouse()); break;
        case 'F': tmView->setFill(!Sys::tm_fill); break;
        case 'I': toggleInclusion(tmView->findTileUnderMouse()); break;
        case 'M': emit sig_raiseMenu(); break;
        case 'R': Sys::tm_fill = false;  emit sig_updateView()  ; break;
        case 'Q': emit sig_close(); break;
        case 'X': clearTranslationVectors(); break;
        case '0': updatePolygonSides(0); break;
        case '1': updatePolygonSides(1); break;
        case '2': updatePolygonSides(2); break;
        case '3': updatePolygonSides(3); break;
        case '4': updatePolygonSides(4); break;
        case '5': updatePolygonSides(5); break;
        case '6': updatePolygonSides(6); break;
        case '7': updatePolygonSides(7); break;
        case '8': updatePolygonSides(8); break;
        case '9': updatePolygonSides(9); break;
        case Qt::Key_F1:
            {
                QMessageBox  * box = new QMessageBox();
                box->setWindowTitle("Tiling Designer Shortcuts");
                box->setText(Shortcuts::getTilingMakerShortcuts());
                box->setModal(false);
                box->show();
                break;
            }

        // modes
        case Qt::Key_Escape: setTilingMakerMouseMode(TM_NO_MOUSE_MODE); return false;     // propagate
        case Qt::Key_F3: setTilingMakerMouseMode(TM_TRANSLATION_VECTOR_MODE); break;
        case Qt::Key_F4: setTilingMakerMouseMode(TM_DRAW_POLY_MODE); break;
        case Qt::Key_F5: setTilingMakerMouseMode(TM_COPY_MODE); break;
        case Qt::Key_F6: setTilingMakerMouseMode(TM_DELETE_MODE); break;
        case Qt::Key_F7: setTilingMakerMouseMode(TM_INCLUSION_MODE); break;
        case Qt::Key_F9: setTilingMakerMouseMode(TM_MEASURE_MODE); break;
        case Qt::Key_F10: setTilingMakerMouseMode(TM_UNIFY_MODE); break;
        case Qt::Key_F11: setTilingMakerMouseMode(TM_EDIT_TILE_MODE); break;
        case Qt::Key_F12: setTilingMakerMouseMode(TM_EDGE_CURVE_MODE); break;

        default: return false;
   }

   return true;
}

void TilingMaker::slot_stack_save()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    TilingData current = tiling->getDataCopy();

    tmStack.add(current);
}

void TilingMaker::slot_stack_undo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    TilingData td = tiling->getDataCopy();

    bool rv = tmStack.pop(td);
    if (rv)
    {
        tiling->setData(td.copy());

        tmView->setTiling(tiling);
        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        Sys::setTilingChange();
        emit sig_updateView();
        emit sig_refreshMenu();
    }
}

void TilingMaker::slot_stack_redo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    TilingData td;

    bool rv = tmStack.redo(td);
    if (rv)
    {
        tiling->setData(td.copy());

        tmView->setTiling(tiling);
        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        Sys::setTilingChange();
        emit sig_updateView();
        emit sig_refreshMenu();
    }
}

/////////////////////////////////////////////////////////////////
///
/// TMStack
///
/////////////////////////////////////////////////////////////////

bool TMStack::add(TilingData & td)
{
    if (stack.isEmpty())
    {
        Q_ASSERT(stackIndex == -1);
        stack.push_back(td);
        stackIndex = 0;
    }
    else
    {
        TilingData stackI  = stack[stackIndex];
        if (td == stackI)
        {
            // nothing to do
            return false;
        }

        if (stackIndex == (stack.size() -1))
        {
            stack.push_back(td);
            stackIndex++;
        }
        else
        {
            // remove tail, then push back
            int delcount = stack.size() - stackIndex;
            stack.remove(delcount,stackIndex);
            stack.push_back(td);
            stackIndex++;
        }
    }
    return true;
}

bool TMStack::pop(TilingData & td)
{
    if (stack.size() == 0)
        return false;

    if (stackIndex < 0)
        return false;

    TilingData current = td;
    TilingData stackI  = stack[stackIndex];
    if (current == stackI)
    {
        if (stackIndex > 0)
        {
            stackIndex--;
            td = stack[stackIndex];
        }
        else
        {
            return false;
        }
    }
    else
    {
        stack.push_back(current);
        td = stack[stackIndex];     // same index
    }
    return true;
}

bool TMStack::redo(TilingData & td)
{
    if (stack.isEmpty())
        return false;

    if ((stackIndex + 1) >= stack.size())
        return false;

    stackIndex++;
    td = stack[stackIndex];

    return true;
}
