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

#include "makers/tiling_maker/tiling_maker.h"
#include "engine/image_engine.h"
#include "geometry/edge.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tile_selection.h"
#include "makers/tiling_maker/tiling_monitor.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "misc/shortcuts.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/backgroundimage.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/backgroundimageview.h"
#include "viewers/debug_view.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_edgepoly_edit.h"
#include "widgets/dlg_magnitude.h"

using std::make_shared;

TilingMaker::TilingMaker()
{
    //qDebug() << "TilingMaker::TilingMaker";
    propagate = true;
}

TilingMaker::~TilingMaker()
{
    tilingMonitor->terminate();
    tilingMonitor->wait();
    delete tilingMonitor;
}

void TilingMaker::init()
{
    view            = Sys::view;
    viewControl     = Sys::viewController;
    prototypeMaker  = Sys::prototypeMaker;
    maped           = Sys::mapEditor;
    tmView          = Sys::tilingMakerView;
    controlPanel    = Sys::controlPanel;
    config          = Sys::config;
    tilingMonitor   = new TilingMonitor;      // runs in its own thread

    tilingMonitor->start();

    tmView->setMaker(this);

    connect(this, &TilingMaker::sig_monitor, tilingMonitor, &TilingMonitor::slot_monitor);

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
    undoStack.clear();
    redoStack.clear();
}

TilingPtr TilingMaker::loadTiling(QString name, eTILM_Event event)
{
    qDebug().noquote() << "TilingMaker::loadTiling" << name;

    Sys::debugView->clear();

    LoadUnit & loadUnit = view->getLoadUnit();
    loadUnit.setLoadState(LOADING_TILING,name);

    TilingManager tm;
    TilingPtr tiling = tm.loadTiling(name,event);

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
        {
            auto fd = csettings.getFillData();
            Canvas & canvas = viewControl->getCanvas();
            canvas.setModelAlignment(M_ALIGN_TILING);
            canvas.setFillData(fd);
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
            Sys::view->setFixedSize(size);
        }
        else
        {
            Sys::view->setSize(size);
        }

        viewControl->removeAllImages();
        viewControl->slot_reconstructView();
        emit sig_tilingLoaded(name);
    }

    loadUnit.resetLoadState();

    return tiling;
}

void TilingMaker::saveTiling(QString name)
{
    TilingPtr tiling = getSelected();

    TilingManager tm;
    bool rv = tm.saveTiling(name,tiling); // Tiling Manager haandles OK/FAIL status
    if (rv)
    {
        emit sig_monitor(true);     // resets has changed
    }
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

    int in_count = getInTiling().count();
    int all_count = tmView->getAllTiles().count();
    s+= QString("  in_tiling: %1  all: %2  stack: %3 %4" ).arg(in_count).arg(all_count).arg(undoStack.size()).arg(redoStack.size());
    return s;
}

TilingPtr TilingMaker::findTilingByName(QString name)
{
    for (const auto & tiling : std::as_const(tilings))
    {
        if (tiling->getTitle() == name)
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
    if (propagate)
        prototypeMaker->sm_takeUp(selectedTiling, event);
}

void TilingMaker::pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile)
{
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
    switch (event)
    {
    case TILM_LOAD_FROM_MOSAIC:
        tilings = proto_tilings;
        if (tilings.count())
        {
            select(tilings.first());
            slot_stack_save();
        }
        break;

    default:
        qCritical("bad call to TilingMaker::sm_takeDown");
    }

    if (config->tm_autofill)
    {
        slot_fillUsingTranslations();
    }
}

void TilingMaker::sm_takeUp(TilingPtr tiling, eTILM_Event event)
{
    eTMState state = sm_getState();
    qDebug().noquote() << "TilingMaker::take() state:" << tm_states[state]  << "event:" <<  sTILM_Events[event];

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

    case TILM_LOAD_FROM_MOSAIC:
        qCritical("use takeDown for TILM_LOAD_FROM_MOSAIC");
        break;
    }

    if (config->tm_autofill)
    {
        slot_fillUsingTranslations();
    }
}

void TilingMaker::select(TilingPtr tiling, bool force)
{
    if (!force && (tiling == selectedTiling))
       return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling:" << tiling->getTitle();

    clearMakerData();

    setupMaker(tiling);

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        tmView->forceLayerRecalc(true);
    }

    selectedTiling = tiling;
#if 0
    ProtoPtr proto = prototypeMaker->getPrototype(tiling);
    if (proto)
    {
        prototypeMaker->setSelectedPrototype(proto);
    }
#endif
}

void TilingMaker::select(ProtoPtr prototype)
{
    TilingPtr tiling = prototype->getTiling();

    if (tiling == selectedTiling)
        return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling from prototype:" << tiling->getTitle();

    clearMakerData();

    setupMaker(tiling);

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        tmView->forceLayerRecalc(true);
    }

    selectedTiling = tiling;
}

int  TilingMaker::numExcluded()
{
    return  tmView->getAllTiles().count() -  getInTiling().count();
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

void TilingMaker::setupMaker(TilingPtr tiling)
{
    tmView->setTiling(tiling);
    
    auto tilingCanvasSize = tiling->getData().getSettings().getCanvasSize();

    qDebug() << "TilingMaker::setupMaker" << tiling->getTitle() << tilingCanvasSize;
    
    viewControl->getCanvas().initCanvasSize(tilingCanvasSize);

    emit sig_monitor(true);
}

bool TilingMaker::verifyTiling()
{
    if (tmView->getAllTiles().size() <= 0)
    {
        qWarning("There are no polygons");
        return false;
    }

    if (getInTiling().size() <= 0)
    {
        qWarning("No selected tiles");
        return false;
    }

    TilingPtr tp = getSelected();
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

const PlacedTiles & TilingMaker::getInTiling() const
{
    return selectedTiling->getData().getInTiling();
}

void TilingMaker::addNewPlacedTile(PlacedTilePtr placedTile)
{
    tmView->addToAll(placedTile);   // push_front so it (the new) becomes selected for move
    addInTiling(placedTile);        // 03AUG21 - additions are always placed in tiling (they can be removed)
    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::replacePlacedTiles(PlacedTiles & placedTiles)
{
    tmView->clearAll();
    tmView->addToAll(placedTiles);
    replaceInTilings(placedTiles);      // 03AUG21 - additions are always placed in tiling (they can be removed)
    forceRedraw();
    emit sig_buildMenu();
}

TileSelectorPtr TilingMaker::addTileSelectionPointer(TileSelectorPtr sel)
{
    TileSelectorPtr ret;
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


QVector<TilePtr> TilingMaker::getUniqueTiles()
{
    UniqueQVector<TilePtr> fs;

    for (const auto & pfp : std::as_const(tmView->getAllTiles()))
    {
        TilePtr fp = pfp->getTile();
        fs.push_back(fp);
    }

    return static_cast<QVector<TilePtr>>(fs);
}

void TilingMaker::slot_deleteTile()
{
    if (clickedSelector)
    {
        deleteTile(clickedSelector);
        clickedSelector.reset();
    }
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::deleteTile(TileSelectorPtr sel)
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
        deleteTile(tile);

        emit sig_buildMenu();
        forceRedraw();
    }
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::deleteTile(PlacedTilePtr pf)
{
    if (pf == currentPlacedTile)
    {
        resetCurrentPlacedTile();
    }
    tmView->removeFromAll(pf);
    selectedTiling->remove(pf);
    forceRedraw();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::unifyTile(PlacedTilePtr pf)
{
    if (!getInTiling().contains(pf))
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
    deleteTile(pf);
    addNewPlacedTile(newPT);
}

void TilingMaker::addInTiling(PlacedTilePtr pf)
{
    selectedTiling->add(pf);
    pushTilingToPrototypeMaker(PROM_TILING_ADDED);
}

void TilingMaker::replaceInTilings(PlacedTiles & placedTiles)
{
    selectedTiling->replace(placedTiles);
    pushTilingToPrototypeMaker(PROM_TILING_ADDED);
}

void TilingMaker::removeFromInTiling(PlacedTilePtr pf)
{
    selectedTiling->remove(pf);
}

void TilingMaker::updateVectors()
{
    fillUsingTranslations();
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);

    MapEditorDb * db = maped->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        maped->loadTilingRepeated();
    }
}

void TilingMaker::updateReps()
{
   fillUsingTranslations();
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

void TilingMaker::slot_fillUsingTranslations()
{
    if (!verifyTiling())
        return;

    fillUsingTranslations();

    forceRedraw();

    emit sig_buildMenu();
}

void TilingMaker::fillUsingTranslations()
{
    // reset
    PlacedTiles intiles  = getInTiling();
    tmView->clearAll();
    tmView->addToAll(intiles);

    // Create copies of the tile to help visualise the result in the panel.
    const FillData fd = getSelected()->getCanvasSettings().getFillData();
    int minX, maxX, minY, maxY;
    bool singleton;
    fd.get(singleton,minX, maxX, minY, maxY);

    if (singleton)
    {
        return;
    }

    QPointF t1    = getSelected()->getData().getTrans1();
    QPointF t2    = getSelected()->getData().getTrans2();

    if (t1.isNull() || t2.isNull())
    {
        return;
    }

    for (const auto & placed : std::as_const(getInTiling()))
    {
        if (!placed->show())
        {
            continue;
        }

        TilePtr f    = placed->getTile();
        QTransform T = placed->getTransform();

        for( int y = minY; y <= maxY; ++y )
        {
            for( int x = minX; x <= maxX; ++x )
            {
                if ( y == 0 && x == 0 )
                {
                    continue;
                }
                QPointF pt = (t1*x) + (t2 * y);
                QTransform tt = QTransform::fromTranslate(pt.x(),pt.y());
                QTransform placement= T * tt;
                tmView->addToAll(make_shared<PlacedTile>(f, placement));
            }
        }
    }

    emit sig_monitor(false);
}

void TilingMaker::removeExcluded()
{
    PlacedTiles intiles  = getInTiling();
    tmView->clearAll();
    tmView->addToAll(intiles);

    tmView->resetTileSelector();
    resetCurrentPlacedTile();

    forceRedraw();
    emit sig_buildMenu();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);

    emit sig_monitor(false);
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
        if(!getInTiling().contains(pf))
        {
            addInTiling(pf);
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILING_ADDED);
        }
    }
}

void TilingMaker::slot_excludeTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        if (getInTiling().contains(pf))
        {
            removeFromInTiling(pf);
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILING_DELETED);
        }
    }
}

bool TilingMaker::isIncluded(PlacedTilePtr pfp)
{
    return getInTiling().contains(pfp);
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

void TilingMaker::slot_uniquifyTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        TilePtr tile2 = tile->recreate();  // creates a new tile same as other
        pf->setTile(tile2);
        emit sig_buildMenu();
        pushTilingToPrototypeMaker(PROM_TILING_ADDED);
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
    for (const auto & pfp : std::as_const(tmView->getAllTiles()))
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
    for (const auto & pfp : std::as_const(tmView->getAllTiles()))
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
    Q_ASSERT(view->getKbdMode(KBD_MODE_XFORM_TILING));
    qreal scale = 1.0 + (0.01 * delta);
    for (const auto & pfp : std::as_const(tmView->getAllTiles()))
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
    for (const auto & pfp : std::as_const(tmView->getAllTiles()))
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

void TilingMaker::toggleInclusion(TileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTilePtr pf = sel->getPlacedTile();
        if (!pf) return;
        if (getInTiling().contains(pf))
        {
            removeFromInTiling(pf); // pushed
        }
        else
        {
            addInTiling(pf);        // pushes
        }
        forceRedraw();
        emit sig_buildMenu();
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

void TilingMaker::mirrorPolygonX(TileSelectorPtr sel )
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

void TilingMaker::mirrorPolygonY(TileSelectorPtr sel )
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

bool TilingMaker::reflectPolygon(TileSelectorPtr sel)
{
    if (sel)
    {
        TileSelectorPtr sel2 = tmView->findEdge(tmView->getMousePos());
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

void TilingMaker::copyPolygon(TileSelectorPtr sel)
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
    if (view->isActiveLayer(tmView))
    {
        view->update();
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
        case 'F': slot_fillUsingTranslations(); break;
        case 'I': toggleInclusion(tmView->findTileUnderMouse()); break;
        case 'M': emit view->sig_raiseMenu(); break;
        case 'R': removeExcluded(); break;
        case 'Q': QApplication::quit(); break;
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

    undoStack.push(tiling->getData());

    redoStack.clear();
}

void TilingMaker::slot_stack_undo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    if (undoStack.size())
    {
        redoStack.push(tiling->getData());

        TilingData td;
        if (undoStack.size() > 1)
        {
            td = undoStack.pop();
        }
        else if (undoStack.size() == 1)
        {
            td = undoStack.top();
        }
        else
        {
            redoStack.pop();
            return;
        }
        tiling->setData(td);
        tmView->setTiling(tiling);

        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::slot_stack_redo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    if (redoStack.size())
    {
        undoStack.push(tiling->getData());

        TilingData td = redoStack.pop();
        tiling->setData(td);
        tmView->setTiling(tiling);

        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
}
