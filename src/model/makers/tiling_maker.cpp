////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly tileful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#include "model/makers/tiling_maker.h"
#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_magnitude.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
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

static QString tm_states[] =
    {
        E2STR(TM_EMPTY),
        E2STR(TM_SINGLE),
        E2STR(TM_MULTI)
    };

TilingMaker::TilingMaker()
{
    //qDebug() << "TilingMaker::TilingMaker";
    setPropagate(true);
    loadUnit = new LoadUnit(LT_TILING);
}

TilingMaker::~TilingMaker()
{
    auto edit = getEdgePolyEditor();
    if (edit)
    {
        delete edit;
    }
}
void TilingMaker::init()
{
    prototypeMaker  = Sys::prototypeMaker;

    Sys::tilingMakerView->setMaker(this);

    connect(this, &TilingMaker::sig_updateView,      Sys::viewController, &SystemViewController::slot_updateView);
    connect(this, &TilingMaker::sig_reconstructView, Sys::viewController, &SystemViewController::slot_reconstructView);
    connect(this, &TilingMaker::sig_raiseMenu,       Sys::controlPanel,   &ControlPanel::slot_raisePanel);

    _tilingMakerMouseMode = TM_NO_MOUSE_MODE;
    _epolyEdit            = nullptr;
    poly_side_count       = Sys::config->polySides;
    poly_rotation         = 0.0;
    vectorState           = VEC_READY;

    unload();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    TilingEvent tilingEvent;
    tilingEvent.event = TILM_LOAD_EMPTY;
    tilingEvent.tiling = tiling;

    Sys::viewController->setSelectedPrimaryLayer(tiling);

    sm_takeUp(tilingEvent);

    emit sig_menuRefresh(TMR_ALL_CLEAR);
}

void TilingMaker::unload()
{
    tilings.clear();
    selectedTiling.reset();
    deselectTile();
    unifyBase.reset();
    resetClickedSelector();
    tmStack.clear();
}

TilingPtr TilingMaker::loadTiling(VersionedFile vfile, eTILM_Event event)
{
    qDebug().noquote() << "TilingMaker::loadTiling" <<  sTILM_Events[event] << vfile.getVersionedName().get();

    Sys::debugView->unloadLayerContent();

    loadUnit->start(vfile);

    TilingManager tm;
    TilingPtr tiling = tm.loadTiling(vfile,event);
    if (tiling)
    {
        Sys::imageViewer->unloadLayerContent();

        TilingEvent tilingEvent;
        tilingEvent.event  = event;
        tilingEvent.tiling = tiling;
        if (event == TILM_LOAD_REPLACE)
            tilingEvent.lastTiling = getSelected();
        sm_takeUp(tilingEvent);

        emit sig_reconstructView();
        emit sig_menuRefresh(TMR_ALL_CLEAR);
        emit sig_tilingLoaded(vfile);

        loadUnit->end(LS_LOADED);
    }
    else
    {
        loadUnit->end(LS_FAILED);
    }

    return tiling;
}

void TilingMaker::reload()
{
    VersionedFile vf = loadUnit->getLoadFile();
    if (!vf.isEmpty())
        loadTiling(vf,TILM_RELOAD);
}

void TilingMaker::saveTiling(TilingPtr tiling, bool forceOverwrite)
{
    VersionedFile retSavedFile;
    TilingManager tm;
    bool rv = tm.saveTiling(tiling,retSavedFile,forceOverwrite); // Tiling Manager haandles OK/FAIL status
    if (rv)
    {
        emit sig_tilingWritten();
        emit sig_tilingLoaded(retSavedFile);
    }

    tiling->getSaveStatus()->init();    // now it does not need saving
}

QString TilingMaker::getStatus()
{
    QString s = sTilingMakerMouseMode[getTilingMakerMouseMode()];

    auto maction = Sys::tilingMakerView->getMouseInteraction();
    if (maction)
    {
        s += " ";
        s += maction->desc;
    }

    s += QString("  in_tiling: %1  all: %2 " ).arg(selectedTiling->unit().numIncluded()).arg(selectedTiling->unit().numAll());
    s += tmStack.getStackStatus();
    return s;
}

TilingPtr TilingMaker::findTilingByName(QString name)
{
    for (const auto & tiling : std::as_const(tilings))
    {
        if (tiling->getVName().get() == name)
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

// called by menu to clear tiling
void TilingMaker::removeTiling(TilingPtr tp)
{
    qDebug().noquote() << "TilingMaker::removeTiling" << tp->info();

    if (tilings.size() >  1)
    {
        TilingEvent tilingEvent;
        tilingEvent.event  = TILM_UNLOAD_TILING;
        tilingEvent.tiling = tp;
        sm_takeUp(tilingEvent);
    }
    else
    {
        // there is always at least an emtpy tiling
        // propagates an empty
        tilings.removeOne(tp);
        TilingPtr tiling = make_shared<Tiling>();

        TilingEvent tilingEvent;
        tilingEvent.event  = TILM_LOAD_EMPTY;
        tilingEvent.tiling = tiling;
        sm_takeUp(tilingEvent);
    }
}

/* there are three possible actions
   1. reset all and add
   2. erase current and add
   3. add
   (future - delete)
*/

void TilingMaker::sm_resetAllAndAdd(TilingPtr tiling)
{
   unload();
   tilings.push_front(tiling);
   select(tiling);
}

void TilingMaker::sm_replaceCurrent(TilingPtr tiling)
{
    int i = tilings.indexOf(selectedTiling);
    tilings.replace(i,tiling);
    select(tiling);
}

void TilingMaker::sm_replace(TilingPtr tiling, TilingPtr old)
{
    int i = tilings.indexOf(old);
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

    if (Sys::config->tm_loadFill)
    {
        Sys::tilingMakerView->setFill(true);
    }

    emit sig_menuRefresh(TMR_ALL_CLEAR);
}

void TilingMaker::sm_takeUp(const TilingEvent & tilingEvent)
{
    eTMState state = sm_getState();
    qDebug().noquote() << "TilingMaker::sm_takeUp" << "state:" << tm_states[state]  << "event:" <<  sTILM_Events[tilingEvent.event];

    auto event  = tilingEvent.event;
    auto tiling = tilingEvent.tiling;

    if (tiling->getBkgdImage())
        Sys::currentBkgImage = BKGD_IMAGE_TILING;
    else
        Sys::currentBkgImage =  BKGD_IMAGE_NONE;

    ProtoEvent protoEvent;
    switch(event)
    {
    case TILM_LOAD_EMPTY:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        protoEvent.event  = PROM_LOAD_EMPTY;
        protoEvent.tiling = tiling;
        prototypeMaker->sm_takeUp(protoEvent);
        break;

    case TILM_LOAD_SINGLE:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        protoEvent.event = PROM_LOAD_SINGLE;
        protoEvent.tiling = tiling;
        prototypeMaker->sm_takeUp(protoEvent);
        slot_stack_save();
        break;

    case TILM_LOAD_MULTI:
        if (state == TM_EMPTY)
        {
            clearConstructionLines();
            sm_resetAllAndAdd(tiling);
            protoEvent.event = PROM_LOAD_SINGLE;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        else if (state == TM_SINGLE)
        {
            clearConstructionLines();
            sm_add(tiling);
            protoEvent.event = PROM_LOAD_MULTI;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        else if (state == TM_MULTI)
        {
            sm_add(tiling);
            protoEvent.event = PROM_LOAD_MULTI;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        slot_stack_save();
        break;

    case TILM_LOAD_REPLACE:
        sm_replace(tiling, tilingEvent.lastTiling);

        protoEvent.event      = PROM_REPLACE;
        protoEvent.tiling     = tiling;
        protoEvent.oldTilings.push_back(tilingEvent.lastTiling);
        prototypeMaker->sm_takeUp(protoEvent);
        break;

    case TILM_REPLACE_ALL:
        sm_resetAllAndAdd(tiling);

        protoEvent.event      = PROM_REPLACE_ALL;
        protoEvent.tiling     = tiling;
        protoEvent.oldTilings = getTilings();
        prototypeMaker->sm_takeUp(protoEvent);
        break;

    case TILM_RELOAD:
        if (state == TM_EMPTY)
        {
            qWarning("Trying to modify when tiling not loaded");
            sm_resetAllAndAdd(tiling);
            protoEvent.event = PROM_LOAD_SINGLE;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        else if (state == TM_SINGLE)
        {
            sm_resetAllAndAdd(tiling);
            protoEvent.event = PROM_RELOAD_SINGLE;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        else if (state == TM_MULTI)
        {
            sm_replaceCurrent(tiling);
            protoEvent.event = PROM_RELOAD_MULTI;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }
        slot_stack_save();
        break;

    case TILM_LOAD_FROM_STEPPER:
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        // does not propagate
        break;

    case TILM_LOAD_FROM_MOSAIC:
        qCritical("use takeDown for TILM_LOAD_FROM_MOSAIC");
        break;

    case TILM_UNLOAD_TILING:
        tilings.removeOne(tilingEvent.tiling);
        select(tilings.first());
        protoEvent.event  = PROM_TILING_DELETED;
        protoEvent.tiling = tilingEvent.tiling;
        prototypeMaker->sm_takeUp(protoEvent);
        break;

    default:
        qCritical().noquote() << "TilingMaker::sm_takeUp" << "unsuported event" << sTILM_Events[event];
        break;
    }

    if (Sys::config->tm_loadFill)
    {
        Sys::tilingMakerView->setFill(true);
    }

    CanvasSettings csettings = tiling->hdr().getCanvasSettings();
    QSize size  = csettings.getViewSize();

    Canvas & canvas = Sys::viewController->getCanvas();
    canvas.setCanvasSize(csettings.getCanvasSize());

    if (Sys::config->splitScreen)
    {
        Sys::viewController->setFixedSize(size);
    }
    else
    {
        Sys::viewController->setSize(size);
    }

    emit Sys::viewController->sig_resetLayers();
    Sys::viewController->setSelectedPrimaryLayer(tiling);
}

void TilingMaker::select(TilingPtr tiling)
{
    qDebug() << "TilingMaker::select" << tiling->getVName().get();

    if (tiling == selectedTiling)
        return;

    if (!tilings.contains(tiling))
    {
        qWarning() << "TilingMaker::select" << "tiling selection not valid";
        return;
    }

    selectedTiling = tiling;

    deselectTile();
    resetClickedSelector();

    Sys::tilingMakerView->clearViewData();
    Sys::tilingMakerView->setTiling(tiling);
    if (Sys::viewController->isEnabled(VIEW_TILING_MAKER))
    {
        Sys::tilingMakerView->forceLayerRecalc(true);
    }

    tiling->setTilingViewChanged();

    tiling->getSaveStatus()->init();    // this is a newly loaded tiling and does not need saving
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

void TilingMaker::removeExcludeds()
{
    selectedTiling->unit().removeExcludeds();

    emit sig_menuRefresh(TMR_TILING_UNIT);
}

bool TilingMaker::verifyTiling()
{
    TilingPtr tp = getSelected();

    if (tp->unit().numAll() == 0)
    {
        qWarning("There are no polygons");
        return false;
    }

    if (tp->unit().numIncluded() == 0)
    {
        qWarning("No included tiles");
        return false;
    }

    const FillData & fd = tp->hdr().getCanvasSettings().getFillData();
    int minX, maxX, minY, maxY;
    bool singleton;
    fd.get(singleton,minX, maxX, minY, maxY);

    if (!singleton)
    {
        if (tp->hdr().getTrans1().isNull() || tp->hdr().getTrans2().isNull())
        {
            qWarning("Translation vectors not defined");
            return false;
        }
    }

    return true;
}

void TilingMaker::setTilingMakerMouseMode(eTilingMakerMouseMode mode)
{
    qDebug() << "tiling maker mouse mode" << sTilingMakerMouseMode[mode];

    _tilingMakerMouseMode = mode;

    unifyBase.reset();
    Sys::tilingMakerView->getAccumW().clear();

    if (mode != TM_EDIT_TILE_MODE)
    {
        auto editor = getEdgePolyEditor();
        if (editor)
        {
            emit sig_close_editor();
        }
    }

    Sys::tilingMakerView->resetEditPlacedTile();  // true for all cases

    switch (mode)
    {
    case TM_EDIT_TILE_MODE:
        decomposedTiling.reset();
        break;

    case TM_TRANSLATION_VECTOR_MODE:
        decomposedTiling.reset();
        clearTranslationVectors();
        vectorState = VEC_SETTING_T1;
        break;

    case TM_DECOMPOSE_MODE:
        break;

    default:
        decomposedTiling.reset();
        break;
    }

    forceRedraw();
}

/////////////////////////////////////////////////
//
// Tile Management
//
/////////////////////////////////////////////////

// Called by MapEditor with placed tiles created from a map.
// There is no possibility of a merge.
// The editing of the single tiles is done directly here in the TilingMaker.
// These are placed tiles made by extracting faces from a map,
//and then each face becomes a placed tile
void TilingMaker::replaceTilingUnit(TilingUnit & tilingUnit)
{
    // remove all tilings except the current
    qDebug() << "TilingMaker::replaceTilingUnit" << "using" << tilingUnit.numAll() << "placed tiles";

    QVector<TilingPtr> todelete;
    TilingPtr current = selectedTiling; // current can change
    for (auto & tiling : tilings)
    {
        if (tiling != current)
            todelete.push_back(tiling);
    }
    for (auto & tiling : todelete)
    {
        TilingEvent tilingEvent;
        tilingEvent.event  = TILM_UNLOAD_TILING;
        tilingEvent.tiling = tiling;
        sm_takeUp(tilingEvent);
    }
    Q_ASSERT(current = selectedTiling);

    // replace the placed tile
    selectedTiling->unit().replaceUnitData(tilingUnit);

    // it's a new ball game
    ProtoEvent protoEvent;
    protoEvent.event = PROM_LOAD_SINGLE;
    protoEvent.tiling  = selectedTiling;
    prototypeMaker->sm_takeUp(protoEvent);

    forceRedraw();
    emit sig_menuRefresh(TMR_MAKER_TILINGS);
}

PlacedTileSelectorPtr TilingMaker::addTileSelectionPointer(PlacedTileSelectorPtr sel)
{
    PlacedTileSelectorPtr ret;
    if (!sel)
        return ret;

    PlacedTilePtr pf    = sel->getPlacedTile();
    if (!pf)
        return ret;

    PlacedTilePtr pfnew = make_shared<PlacedTile>(pf->getTile(), pf->getPlacement());
    addPlacedTile(pfnew);
    
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

    emit sig_menuRefresh(TMR_TILING_UNIT);

    return ret;
}

void TilingMaker::slot_deleteTile()
{
    if (clickedSelector())
    {
        deleteTile(clickedSelector());
        resetClickedSelector();
    }
}

void TilingMaker::deleteTile(PlacedTileSelectorPtr sel)
{
    if (!sel) return;

    PlacedTilePtr tile = sel->getPlacedTile();
    auto tselect = Sys::tilingMakerView->tileSelector();
    if (tselect &&  tselect->getPlacedTile() == tile)
    {
        Sys::tilingMakerView->resetTileSelector();
    }

    if (tile)
    {
        deletePlacedTile(tile);
    }
}

void TilingMaker::deletePlacedTile(PlacedTilePtr ptp)
{
    if (ptp == selectedTile())
    {
        deselectTile();
    }
    selectedTiling->unit().removePlacedTile(ptp);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_UNIT_CHANGED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    emit sig_menuRefresh(TMR_TILING_UNIT);

    forceRedraw();
}

void TilingMaker::unifyTile(PlacedTilePtr pf)
{
    if (!selectedTiling->unit().getIncluded().contains(pf))
        return;

    if (!unifyBase)
    {
        // start
        unifyBase = pf;
    }
    else
    {
        // make new tile at this position same as the unify base
        QTransform transform = pf->getPlacement();
        TilePtr    tile      = unifyBase->getTile();
        PlacedTilePtr newPT  = make_shared<PlacedTile>(tile,transform);

        // remove old, add new
        deletePlacedTile(pf);
        addPlacedTile(newPT);

        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_UNIT_CHANGED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    emit sig_menuRefresh(TMR_TILING_UNIT);
}

void TilingMaker::uniquifyTile(PlacedTilePtr pf)
{
    auto tiling = selectedTiling;
    tiling->unit().removePlacedTile(pf);

    TilePtr tile = pf->getTile();
    TilePtr tile2 = tile->recreate();  // creates a new tile same as other
    pf->setTile(tile2);
    tiling->unit().addPlacedTile(pf);  // modifies tiling unit

    ProtoEvent protoEvent;
    protoEvent.event  = PROM_TILING_UNIT_CHANGED;
    protoEvent.tiling = tiling;
    prototypeMaker->sm_takeUp(protoEvent);

    emit sig_menuRefresh(TMR_TILING_UNIT);
}

void TilingMaker::decomposeTile(PlacedTilePtr pf)
{
    qDebug() << "TilingMaker::decomposeTile";

    if (!selectedTiling->unit().getIncluded().contains(pf))
        return;

    // remove from old
    auto selected = selectedTiling;

    selected->unit().removePlacedTile(pf);

    ProtoEvent pevent;
    pevent.event  = PROM_TILING_UNIT_CHANGED;
    pevent.tiling = selected;
    prototypeMaker->sm_takeUp(pevent);

    // add to new
    if (!decomposedTiling)
    {
        // start
        decomposedTiling = make_shared<Tiling>();
        decomposedTiling->copy(selected);

        VersionedName vname = FileServices::getNextVersion(FILE_TILING,decomposedTiling->getVName());
        decomposedTiling->setVName(vname);

        decomposedTiling->unit().clearPlacedTiles();
        decomposedTiling->unit().addPlacedTile(pf->copy());

        TilingEvent tevent;
        tevent.event  = TILM_LOAD_MULTI;
        tevent.tiling = decomposedTiling;
        sm_takeUp(tevent);
    }
    else
    {
        decomposedTiling->unit().addPlacedTile(pf->copy());

        ProtoEvent pevent;
        pevent.event  = PROM_TILING_UNIT_CHANGED;
        pevent.tiling = decomposedTiling;
        prototypeMaker->sm_takeUp(pevent);
    }

    if (getTilingMakerMouseMode() != TM_DECOMPOSE_MODE)
    {
        setTilingMakerMouseMode(TM_DECOMPOSE_MODE); // restore
    }

    if (getSelected() != selected)
    {
        select(selected);      // restore
    }

    emit sig_menuRefresh(TMR_MAKER_TILINGS);

    Q_ASSERT(getSelected() == selected);
    Q_ASSERT(getTilingMakerMouseMode() == TM_DECOMPOSE_MODE);
}

void TilingMaker::addPlacedTile(PlacedTilePtr placedTile)
{
    selectedTiling->unit().addPlacedTile(placedTile);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_UNIT_CHANGED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();

    emit sig_menuRefresh(TMR_TILING_UNIT);
}

void TilingMaker::duplicateSelectedTiling()
{
    TilingPtr tp = make_shared<Tiling>();
    tp->copy(selectedTiling);

    VersionedName vname = FileServices::getNextVersion(FILE_TILING,tp->getVName());
    tp->setVName(vname);

    TilingEvent tilingEvent;
    tilingEvent.event  = TILM_LOAD_MULTI;
    tilingEvent.tiling = tp;
    sm_takeUp(tilingEvent);

    emit sig_menuRefresh(TMR_MAKER_TILINGS);
}

void TilingMaker::mergeTilings()
{
    // assemble new tiling
    TilingPtr newTiling = make_shared<Tiling>(*tilings.first());
    for (int i=1; i < tilings.size(); i++)
    {
        auto old = tilings[i];
        TilingUnit tu = old->unit();
        newTiling->unit().addUnitData(tu);
    }

    // give it a name
    VersionedName vname = FileServices::getNextVersion(FILE_TILING,tilings.first()->getVName());
    newTiling->setVName(vname);

    // replace the tilings with the new merged tiling
    TilingEvent tevent;
    tevent.event = TILM_REPLACE_ALL;
    tevent.tiling = newTiling;
    sm_takeUp(tevent);

    emit sig_menuRefresh(TMR_MAKER_TILINGS);
}

void TilingMaker::updateVectors()
{
    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_CHANGED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    MapEditorDb * db = Sys::mapEditor->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        Sys::mapEditor->loadTilingRepeated();
    }
}

void TilingMaker::updateReps()
{
    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_CHANGED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    MapEditorDb * db = Sys::mapEditor->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        Sys::mapEditor->loadTilingRepeated();
    }
}

void TilingMaker::resetOverlaps()
{
   selectedTiling->resetOverlaps();  // sets UNDEFINED
}

void TilingMaker::slot_propagate_changed(bool checked)
{
    qDebug() << "TilingMaker::slot_propagate_changed" << "checked" << checked;

    setPropagate(checked);
    if (checked)
    {
        auto tilings = getTilings();
        for (auto & tiling : tilings)
        {
            ProtoEvent protoEvent;
            protoEvent.event = PROM_TILING_CHANGED;
            protoEvent.tiling = tiling;
            Sys::prototypeMaker->sm_takeUp(protoEvent);
        }
    }
}

void TilingMaker::slot_showTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();
        pf->setShow(true);
        emit sig_menuRefresh(TMR_TILING_UNIT);
        forceRedraw();
        resetClickedSelector();
    }
}

void TilingMaker::slot_hideTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();
        pf->setShow(false);
        emit sig_menuRefresh(TMR_TILING_UNIT);
        forceRedraw();
        resetClickedSelector();
    }
}

void TilingMaker::slot_includeTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();
        if (!pf->isIncluded())
        {
            pf->include();
            if (getPropagate())
            {
                ProtoEvent protoEvent;
                protoEvent.event = PROM_TILING_UNIT_CHANGED;
                protoEvent.tiling = selectedTiling;
                prototypeMaker->sm_takeUp(protoEvent);
            }
            emit sig_menuRefresh(TMR_TILING_UNIT);
            forceRedraw();
            resetClickedSelector();
        }
    }
}

void TilingMaker::slot_excludeTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();
        if (pf->isIncluded())
        {
            pf->exclude();
            if (getPropagate())
            {
                ProtoEvent protoEvent;
                protoEvent.event = PROM_TILING_UNIT_CHANGED;
                protoEvent.tiling = selectedTiling;
                prototypeMaker->sm_takeUp(protoEvent);
            }
            resetClickedSelector();
            forceRedraw();
            emit sig_menuRefresh(TMR_TILING_UNIT);
        }
    }
}

void TilingMaker::slot_view_menu_editTile()
{
    // called by menu from click on tile in view
    if (clickedSelector())
    {
        setTilingMakerMouseMode(TM_EDIT_TILE_MODE);
        PlacedTilePtr ptp = clickedSelector()->getPlacedTile();
        editTile(ptp);
    }
}

void TilingMaker::editTile(PlacedTilePtr ptp)
{
    if (!ptp)
    {
        setTilingMakerMouseMode(TM_EDIT_TILE_MODE);
        return;
    }

    TilePtr tile = ptp->getTile();
    QTransform t = ptp->getPlacement();

    DlgEdgePolyEdit * epedit = getEdgePolyEditor();
    if (epedit)
    {
        delete epedit;
    }

    epedit = new DlgEdgePolyEdit(getSelected(),tile,t,Sys::controlPanel);
    epedit->show();
    epedit->raise();
    epedit->activateWindow();

    Sys::tilingMakerView->setEditPlacedTile(ptp);
    setEdgePolyEditor(epedit);

    connect(this, &TilingMaker::sig_menuRefresh, epedit, &DlgEdgePolyEdit::display);
}

void TilingMaker::slot_copyMoveTile()
{
    if (clickedSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyMovePolygon>(clickedSelector(), clickedSpt));
        resetClickedSelector();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinPoint()
{
    if (clickedSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinPoint>(clickedSelector(), clickedSpt));
        resetClickedSelector();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinMidPoint()
{
    if (clickedSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinMidPoint>(clickedSelector(), clickedSpt));
        resetClickedSelector();
        clickedSpt = QPointF();
    }
}
void TilingMaker::slot_copyJoinEdge()
{
    if (clickedSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinEdge>(clickedSelector(), clickedSpt));
        resetClickedSelector();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();

        uniquifyTile(pf);

        forceRedraw();
        emit sig_menuRefresh(TMR_PLACED_TILE);

    }
}

void TilingMaker::slot_convertTile()
{
    if (clickedSelector())
    {
        PlacedTilePtr pf = clickedSelector()->getPlacedTile();
        TilePtr tile = pf->getTile();

        flipTileRegularity(tile);

        emit sig_menuRefresh(TMR_PLACED_TILE);
    }
}

void TilingMaker::moveTileTo(QPointF pt)
{
    if (clickedSelector())
    {
        selectTile(clickedSelector()->getPlacedTile());
        placedTileSetTranslate(pt.x(), pt.y());

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
            protoEvent.tiling = selectedTiling;
            protoEvent.ptile  = selectedTile();
            prototypeMaker->sm_takeUp(protoEvent);
        }

        emit sig_menuRefresh(TMR_PLACED_TILE);
        forceRedraw();
    }
}

void TilingMaker::flipTileRegularity(TilePtr tile)
{
    tile->flipRegularity();

    ProtoEvent protoEvent;
    protoEvent.event  = PROM_TILE_UNIQUE_REGULARITY_CHANGED;
    protoEvent.tiling = selectedTiling;
    protoEvent.tile   = tile;
    prototypeMaker->sm_takeUp(protoEvent);

    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaX(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    PlacedTiles placements = selectedTiling->unit().getAll();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_MODIFED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaY(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    PlacedTiles placements = selectedTiling->unit().getAll();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_MODIFED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);

    PlacedTiles placements = selectedTiling->unit().getAll();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        qDebug() << "t0" << Transform::info(t);
        QTransform t1 = t.scale(scale,scale);

        t = pfp->getPlacement();
        QTransform t2 = t *QTransform::fromScale(scale,scale);

        qDebug() << "t1" << Transform::info(t1);
        qDebug() << "t2" << Transform::info(t2);

        t = pfp->getPlacement();
        // scales position too
        t *= QTransform::fromScale(scale,scale);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_MODIFED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;

    PlacedTiles placements = selectedTiling->unit().getAll();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILING_MODIFED;
        protoEvent.tiling = selectedTiling;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::placedTileDeltaX(qreal delta)
{
    if (!selectedTile())
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = selectedTile()->getPlacement();
    t *= QTransform::fromTranslate(qdelta,0.0);
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.ptile  = selectedTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::placedTileDeltaY(qreal delta)
{
    if (!selectedTile())
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = selectedTile()->getPlacement();
    t *= QTransform::fromTranslate(0.0,qdelta);
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.ptile  = selectedTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::placedTileSetTranslate(qreal x, qreal y)
{
    if (!selectedTile())
        return;

    QTransform t = selectedTile()->getPlacement();

    Xform xf(t);
    xf.setTranslateX(x);
    xf.setTranslateY(y);
    t = xf.getTransform();
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.ptile  = selectedTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::placedTileDeltaScale(int delta)
{
    qreal scale = 0.01 * delta;
    placedTileDeltaScale(scale);
}

void TilingMaker::placedTileDeltaScale(qreal scale)
{
    if (!selectedTile())
        return;

    QTransform t = selectedTile()->getPlacement();
    Xform xf(t);
    qreal tscale = xf.getScale();
    tscale += scale;
    placedTileSetScale(tscale);
}

void TilingMaker::placedTileSetScale(qreal scale)
{
    if (!selectedTile())
        return;

    QTransform t = selectedTile()->getPlacement();

    Xform xf(t);
    xf.setScale(scale);
    t = xf.getTransform();
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.ptile  = selectedTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }
    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::placedTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    placedTileDeltaRotate(qdelta);
}

void TilingMaker::placedTileDeltaRotate(qreal rotate)
{
    if (!selectedTile())
        return;

    QTransform t = selectedTile()->getPlacement();
    Xform xf(t);
    qreal rotation = xf.getRotateDegrees();
    rotation += rotate;
    placedTileSetRotate(rotation);
}

void TilingMaker::placedTileSetRotate(qreal rotate)
{
    if (!selectedTile())
        return;

    QTransform t = selectedTile()->getPlacement();

    Xform xf(t);
    xf.setRotateDegrees(rotate);
    t = xf.getTransform();
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.ptile  = selectedTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::uniqueTileDeltaScale(int delta)
{
    qreal scale = 0.01 * delta;
    uniqueTileDeltaScale(scale);
}

void TilingMaker::uniqueTileDeltaScale(qreal scale)
{
    if (!selectedTile())
        return;

    TilePtr tile = selectedTile()->getTile();

    if (!tile)
        return;

    tile->deltaScale(scale);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::uniqueTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    uniqueTileDeltaRotate(qdelta);
}

void TilingMaker::uniqueTileDeltaRotate(qreal rotate)
{
    if (!selectedTile())
        return;

    TilePtr tile = selectedTile()->getTile();
    if (!tile)
        return;

    tile->deltaRotation(rotate);

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
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

    QPointF t1 = selectedTiling->hdr().getTrans1();
    QPointF t2 = selectedTiling->hdr().getTrans2();

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

    selectedTiling->hdr().setTranslationVectors(t1,t2,origin);
    selectedTiling->setTilingViewChanged();

    ProtoEvent protoEvent;
    protoEvent.event  = PROM_TILING_CHANGED;
    protoEvent.tiling = selectedTiling;
    prototypeMaker->sm_takeUp(protoEvent);

    forceRedraw();
    emit sig_menuRefresh(TMR_TILING_HEADER);
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
        }
        else
        {
            pf->include();
        }

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILING_UNIT_CHANGED;
            protoEvent.tiling = selectedTiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }

        emit sig_menuRefresh(TMR_TILING_HEADER);
        forceRedraw();
    }
}

void TilingMaker::clearTranslationVectors()
{
    if (selectedTiling)
    {
        selectedTiling->hdr().setTranslationVectors(QPointF(),QPointF(),QPointF());
        selectedTiling->setTilingViewChanged();
        forceRedraw();
        emit sig_menuRefresh(TMR_TILING_HEADER);
    }
}

void TilingMaker::updatePolygonSides(int number)
{
    poly_side_count        = number;
    Sys::config->polySides = number;
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

        f = make_shared<Tile>(ep);
        f->setRotate(poly_rotation);
    }

    QTransform t;
    addPlacedTile(make_shared<PlacedTile>(f,t));

    forceRedraw();
    emit sig_menuRefresh(TMR_TILING_UNIT);
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

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
            protoEvent.tiling = selectedTiling;
            protoEvent.tile   = tile;
            prototypeMaker->sm_takeUp(protoEvent);
        }

        forceRedraw();
        emit sig_menuRefresh(TMR_PLACED_TILE);
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

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
            protoEvent.tiling = selectedTiling;
            protoEvent.tile   = tile;
            prototypeMaker->sm_takeUp(protoEvent);
        }

        forceRedraw();
        emit sig_menuRefresh(TMR_PLACED_TILE);
    }
}

bool TilingMaker::reflectPolygon(PlacedTileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTileSelectorPtr sel2 = Sys::tilingMakerView->findEdge(Sys::tilingMakerView->getMousePos());
        if (sel2)
        {
            if (sel2->getType() == MID_POINT || sel2->getType() == EDGE)
            {
                PlacedTilePtr ptp = sel->getPlacedTile();
                auto tile         = ptp->getTile();
                EdgePoly & ep     = tile->getEdgePolyRW();
                QPolygonF poly    = ep.getPolygon();
                QLineF line       = sel2->getModelLine();
                QPolygonF poly2   = Geo::reflectPolygon(poly,line);
                ep.set(poly2);
                tile->decompose();

                if (getPropagate())
                {
                    ProtoEvent protoEvent;
                    protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
                    protoEvent.tiling = selectedTiling;
                    protoEvent.tile   = tile;
                    prototypeMaker->sm_takeUp(protoEvent);
                }

                forceRedraw();
                emit sig_menuRefresh(TMR_PLACED_TILE);
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
        addPlacedTile(make_shared<PlacedTile>(pf->getTile(), pf->getPlacement())); // pushes

        forceRedraw();
        emit sig_menuRefresh(TMR_TILING_UNIT);
    }
}

void TilingMaker::forceRedraw()
{
    if (Sys::viewController->isEnabled(VIEW_TILING_MAKER))
    {
        emit sig_updateView();
    }
}

void TilingMaker::slot_createConvex()
{
    createCurveFromEdge(CURVE_CONVEX);
}

void TilingMaker::slot_createConcave()
{
    createCurveFromEdge(CURVE_CONCAVE);
}

void TilingMaker::createCurveFromEdge(eCurveType ctype)
{
    PlacedTileSelectorPtr tsel  = Sys::tilingMakerView->tileSelector();

    EdgePtr edege   = tsel->getModelEdge();
    edege->convertToCurve(ctype);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    Q_ASSERT(tile->epolyContains(edege));
    tile->decompose();

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_TILING_UNIT);
    qDebug() << "edge converted to CURVE";
}

void TilingMaker::slot_flatenCurve()
{
    auto tsel    = Sys::tilingMakerView->tileSelector();
    EdgePtr edge = tsel->getModelEdge();
    edge->resetCurveToLine();

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
    qDebug() << "edge converted to LINE";
}

void TilingMaker::slot_makeConvex()
{
    auto tsel    = Sys::tilingMakerView->tileSelector();
    EdgePtr edge = tsel->getModelEdge();
    Q_ASSERT(edge->isCurve());

    edge->changeCurveType(CURVE_CONVEX);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_makeConcave()
{
    auto tsel    = Sys::tilingMakerView->tileSelector();
    EdgePtr edge = tsel->getModelEdge();
    Q_ASSERT(edge->isCurve());

    edge->changeCurveType(CURVE_CONCAVE);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tile;
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_moveArcCenter()
{
    auto tsel  = Sys::tilingMakerView->tileSelector();
    Sys::tilingMakerView->setMouseInteraction(make_shared<EditEdge>(tsel,QPointF()));

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tsel->getPlacedTile()->getTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_editMagnitude()
{
    auto tsel  = Sys::tilingMakerView->tileSelector();
    Sys::tilingMakerView->resetMouseInteraction();
    DlgMagnitude dlg(tsel);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();

    if (getPropagate())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = selectedTiling;
        protoEvent.tile   = tsel->getPlacedTile()->getTile();
        prototypeMaker->sm_takeUp(protoEvent);
    }

    forceRedraw();
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool TilingMaker::procKeyEvent(QKeyEvent * k)
{
    if (!Sys::viewController->isEnabled(VIEW_TILING_MAKER))
    {
        return false;
    }

    switch (k->key())
    {
        // actions
        case 'A': addRegularPolygon(); break;
        case 'C': addTileSelectionPointer(Sys::tilingMakerView->findTileUnderMouse()); break;
        case 'D': deleteTile(Sys::tilingMakerView->findTileUnderMouse()); break;
        case 'F': Sys::tilingMakerView->setFill(!Sys::tm_fill); emit sig_updateView(); break;
        case 'I': toggleInclusion(Sys::tilingMakerView->findTileUnderMouse()); break;
        case 'M': emit sig_raiseMenu(); break;
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

        case Qt::Key_F1: Shortcuts::popup(VIEW_TILING_MAKER); break;

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

    TilingUnit current = tiling->unit().uniqueCopy();

    tmStack.add(current);
}

void TilingMaker::slot_stack_undo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    TilingUnit td = tiling->unit().uniqueCopy();

    deselectTile(); // selection no longer needed

    bool rv = tmStack.pop(td);
    if (rv)
    {
        TilingUnit copy = td.uniqueCopy();
        tiling->unit().replaceUnitData(copy);
        tiling->setTilingViewChanged();

        Sys::tilingMakerView->setTiling(tiling);

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILING_CHANGED;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }

        emit sig_updateView();
        emit sig_menuRefresh(TMR_TILING_UNIT);
    }
}

void TilingMaker::slot_stack_redo()
{
    auto tiling  = getSelected();
    if (!tiling) return;

    TilingUnit td(tiling.get());

    deselectTile(); // selection no longer needed

    bool rv = tmStack.redo(td);
    if (rv)
    {
        TilingUnit copy = td.uniqueCopy();
        tiling->unit().replaceUnitData(copy);
        tiling->setTilingViewChanged();

        Sys::tilingMakerView->setTiling(tiling);

        if (getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILING_CHANGED;
            protoEvent.tiling = tiling;
            prototypeMaker->sm_takeUp(protoEvent);
        }

        emit sig_updateView();
        emit sig_menuRefresh(TMR_TILING_UNIT);
    }
}

/////////////////////////////////////////////////////////////////
///
/// TMStack
///
/////////////////////////////////////////////////////////////////

bool TMStack::add(TilingUnit & td)
{
    if (stack.isEmpty())
    {
        Q_ASSERT(stackIndex == -1);
        stack.push_back(td);
        stackIndex = 0;
    }
    else
    {
        TilingUnit stackI  = stack[stackIndex];
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

bool TMStack::pop(TilingUnit & td)
{
    if (stack.size() == 0)
        return false;

    if (stackIndex < 0)
        return false;

    TilingUnit current = td;
    TilingUnit stackI  = stack[stackIndex];
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

bool TMStack::redo(TilingUnit & td)
{
    if (stack.isEmpty())
        return false;

    if ((stackIndex + 1) >= stack.size())
        return false;

    stackIndex++;
    td = stack[stackIndex];

    return true;
}
