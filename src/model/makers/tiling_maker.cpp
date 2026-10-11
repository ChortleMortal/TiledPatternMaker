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
#include "gui/top/view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_magnitude.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/mosaics/reader_base.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling_writer.h"
#include "sys/engine/image_engine.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"
#include "sys/sys/load_unit.h"

using std::make_shared;

static QString tm_states[] =
    {
        E2STR(Empty),
        E2STR(Single),
        E2STR(Multi)
    };

const QString sTMModelMode[]  =
    {
        E2STR(TM_MODE_XFORM_ALL),
        E2STR(TM_MODE_XFORM_TILING),
        E2STR(TM_MODE_XFORM_UNIQUE_TILE),
        E2STR(TM_MODE_XFORM_PLACED_TILE),
};

TilingMaker::TilingMaker()
{
    //qDebug() << "TilingMaker::TilingMaker";
    setPropagate(true);
    tilingLoadUnit = new LoadUnit(LT_TILING);
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
    motifMaker  = Sys::motifMaker;

    Sys::tilingMakerView->setMaker(this);

    connect(this, &TilingMaker::sig_updateView,      Sys::systemViewController, &ViewController::slot_updateView);
    connect(this, &TilingMaker::sig_reconstructView, Sys::systemViewController, &ViewController::slot_reconstructView);
    connect(this, &TilingMaker::sig_raiseMenu,       Sys::controlPanel,         &ControlPanel::slot_raisePanel);

    _tilingMakerMouseMode     = MouseMode::None;
    _tilingMakerKeyboardMode  = KeybdMode::XformAll;

    _epolyEdit            = nullptr;
    vectorState           = VEC_READY;

    unload();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    TilingEvent tilingEvent;
    tilingEvent.event  = Event::LoadEmpty;
    tilingEvent.tiling = tiling;

    Sys::systemViewController->setSelectedPrimaryLayer(tiling);

    sm_takeUp(tilingEvent);

    emit sig_menuRefresh(TMR_ALL_CLEAR);
}

void TilingMaker::unload()
{
    tilings.clear();
    selectedTiling.reset();
    selectTile(nullptr);
    unifyBase.reset();
    if (Sys::tilingMakerView)
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    tmStack.clear();
    anchors.clear();
    originAnchor = TransAnchor();
    t1Anchor     = TransAnchor();
    t2Anchor     = TransAnchor();
}

void TilingMaker::loadTiling(VersionedFile vfile, Event event)
{
    qDebug().noquote() << "TilingMaker::loadTiling" <<  sTILM_Events[(int)event] << vfile.getVersionedName().get();

    Sys::debugView->unloadLayerContent();
    Sys::imageViewer->unloadLayerContent();

    emit sig_tilingLoading(vfile);
    tilingLoadUnit->start(vfile);

    QString failMsg;
    TilingPtr newTiling = readTiling(vfile,failMsg);
    if (newTiling)
    {
        qDebug() << "new tiling :" << newTiling->getVName().get();
        tilingLoadUnit->end(LS_LOADED);

        TilingEvent tilingEvent;
        tilingEvent.event  = event;
        tilingEvent.tiling = newTiling;
        if (event == Event::LoadReplace)
            tilingEvent.lastTiling = getSelectedTiling();
        sm_takeUp(tilingEvent);

        emit sig_reconstructView();
        emit sig_menuRefresh(TMR_ALL_CLEAR);
        emit sig_tilingLoaded(vfile);

    }
    else
    {
        tilingLoadUnit->end(LS_FAILED);
        emit sig_messageBox(failMsg,QString(),QMessageBox::Critical,false);
    }
}

void TilingMaker::reload()
{
    VersionedFile vf = tilingLoadUnit->getLoadFile();
    if (!vf.isEmpty())
        loadTiling(vf,Event::Reload);
}

bool TilingMaker::saveTiling(TilingPtr tiling, VersionedFile vfile)
{
    // the system view controller's canvas is a shared pointer to the tiling's own
    // canvas (see Canvas::OBSOLETE_CANVAS), so it is already up to date here
    if (getSelectedTiling() == tiling)
    {
        Xform xf = Sys::tilingMakerView->getModelXform();
        tiling->setModelXform(xf,false,Sys::nextSigid());
    }

    TilingWriter writer;
    bool rv = writer.writeTilingXML(vfile,tiling);   // uses the name in the tiling
    if (rv)
    {
        tiling->getSaveStatus()->init();    // now it does not need saving

        // note: sig_tilingLoaded is NOT emitted here - the in-memory tiling is unchanged by a
        // save, only its file name/version may have. See MosaicMaker::save() for the same fix.
        emit sig_tilingWritten(vfile);
    }
    return rv;
}

TilingPtr TilingMaker::readTiling(VersionedFile & vfile, QString & failMsg)
{
    QString name = vfile.getVersionedName().get();

    TilingPtr tiling;

    if (vfile.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        failMsg = QString("No tiling found with name <%1>not found").arg(name);
        return tiling;
    }

    qInfo().noquote() << "TilingMaker::readTiling" << vfile.getPathedName();

    TilingReader tm(Sys::systemViewController);
    ReaderBase mrbase;
    tiling  = tm.readTilingXML(vfile,&mrbase);

    if (!tiling)
    {
        qWarning().noquote() << "Error loading" << name;
        failMsg = QString("Load Error: <%1>").arg(name);
        return tiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << tiling->getVName().get();

    return tiling;
}

bool TilingMaker::verifyTilingFiles()
{
    bool rv = true;
    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        QString failMsg;
        TilingPtr tiling = readTiling(file,failMsg);
        if (tiling->getVName().get() != file.getVersionedName().get())
        {
            qWarning() << "Error: name does not match filename =" << file.getVersionedName().get() <<"internal name= " << tiling->getVName().get();
            rv = false;
        }
        if (!FileServices::verifyTilingFile(file))
        {
            qWarning() << "Error: name does not match filename =" << file.getVersionedName().get();
            rv = false;
        }
    }
    return rv;
}

QString TilingMaker::getStatus()
{
    QString s = sTilingMakerMouseMode[(int)getMouseMode()];

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
        tilingEvent.event  = Event::UnloadTiling;
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
        tilingEvent.event  = Event::LoadEmpty;
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
   selectTiling(tiling);
}

void TilingMaker::sm_replaceCurrent(TilingPtr tiling)
{
    int i = tilings.indexOf(selectedTiling);
    tilings.replace(i,tiling);
    selectTiling(tiling);
}

void TilingMaker::sm_replace(TilingPtr tiling, TilingPtr old)
{
    qDebug() << "TilingMaker::sm_replace" << old->getVName().get() << "with" << tiling->getVName().get();
    int i = tilings.indexOf(old);
    tilings.replace(i,tiling);
    selectTiling(tiling);
}

void TilingMaker::sm_add(TilingPtr tiling)
{
    tilings.push_front(tiling);
    selectTiling(tiling);
}

void TilingMaker::sm_takeDown(QVector<TilingPtr> & proto_tilings, Event event)
{
    qDebug() << "TilingMaker::sm_takeDown tilings=" << proto_tilings.size() << sTILM_Events[(int)event];

    switch (event)
    {
    case Event::LoadFromMosaic:
        tilings = proto_tilings;
        if (tilings.count())
        {
            auto tiling = tilings.first();
            Q_ASSERT(tiling);
            selectTiling(tiling);
            sm_syncConstructionLinesOnLoad();
            slot_stack_save();
            // unlike a direct tiling load, this never runs through LoadUnit, so
            // lastLoadedTiling must be synced here or it goes stale after a mosaic load
            Sys::config->lastLoadedTiling = tiling->getVName();
        }
        break;

    default:
        qCritical("bad call to TilingMaker::sm_takeDown");
    }

    if (Sys::config->tm_fillOnLoad)
        Sys::tilingMakerView->setFill(true);

    emit sig_menuRefresh(TMR_ALL_CLEAR);
}

void TilingMaker::sm_takeUp(const TilingEvent & tilingEvent)
{
    State state = sm_getState();
    qDebug().noquote() << "TilingMaker::sm_takeUp" << "state:" << tm_states[(int)state]  << "event:" <<  sTILM_Events[(int)tilingEvent.event];

    auto event  = tilingEvent.event;
    auto tiling = tilingEvent.tiling;

    // the tiling's background image becomes the one image; a tiling without one clears it,
    // unless it is only being added alongside/replacing others
    const BkgdRecord & rec = tiling->getBkgdRecord();
    if (!rec.isEmpty())
        Sys::backgroundImage->load(rec);
    else if (event != Event::LoadMulti && event != Event::LoadReplace)
        Sys::backgroundImage->unload();

    MotifEvent motifEvent;

    switch(event)
    {
    case Event::LoadEmpty:
        sm_resetAllAndAdd(tiling);
        sm_syncConstructionLinesOnLoad();
        motifEvent.event  = MotifMaker::Event::LoadEmpty;
        motifEvent.tiling = tiling;
        motifMaker->sm_takeUp(motifEvent);
        break;

    case TilingMaker::Event::LoadSingle:
        sm_resetAllAndAdd(tiling);
        sm_syncConstructionLinesOnLoad();
        motifEvent.event = MotifMaker::Event::LoadSingle;
        motifEvent.tiling = tiling;
        motifMaker->sm_takeUp(motifEvent);
        slot_stack_save();
        break;

    case TilingMaker::Event::LoadMulti:
        switch (state)
        {
        case State::Empty:
            sm_resetAllAndAdd(tiling);
            sm_syncConstructionLinesOnLoad();
            motifEvent.event = MotifMaker::Event::LoadSingle;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;

        case State::Single:
            sm_add(tiling);
            sm_syncConstructionLinesOnLoad();
            motifEvent.event = MotifMaker::Event::LoadMulti;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;

        case State::Multi:
            sm_add(tiling);
            // deliberately no construction-lines sync here - a multi-tiling stack's
            // construction lines aren't scoped to just the newest tiling added
            motifEvent.event = MotifMaker::Event::LoadMulti;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;
        }
        slot_stack_save();
        break;

    case Event::LoadReplace:
        sm_replace(tiling, tilingEvent.lastTiling);
        sm_syncConstructionLinesOnLoad();

        motifEvent.event      = MotifMaker::Event::LoadReplace;
        motifEvent.tiling     = tiling;
        motifEvent.oldTilings.push_back(tilingEvent.lastTiling);
        motifMaker->sm_takeUp(motifEvent);
        break;

    case Event::ReplaceAll:
        sm_resetAllAndAdd(tiling);
        sm_syncConstructionLinesOnLoad();

        motifEvent.event      = MotifMaker::Event::ReplaceAll;
        motifEvent.tiling     = tiling;
        motifEvent.oldTilings = getTilings();
        motifMaker->sm_takeUp(motifEvent);
        break;

    case Event::Reload:
        switch (state)
        {
        case State::Empty:
            qWarning("Trying to modify when tiling not loaded");
            sm_resetAllAndAdd(tiling);
            sm_syncConstructionLinesOnLoad();
            motifEvent.event = MotifMaker::Event::LoadSingle;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;

        case State::Single:
            sm_resetAllAndAdd(tiling);
            sm_syncConstructionLinesOnLoad();
            motifEvent.event = MotifMaker::Event::ReloadSingle;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;

        case State::Multi:
            sm_replaceCurrent(tiling);
            sm_syncConstructionLinesOnLoad();
            motifEvent.event = MotifMaker::Event::ReloadMulti;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
            break;
        }
        slot_stack_save();
        break;

    case Event::LoadFromStepper:
        sm_resetAllAndAdd(tiling);
        sm_syncConstructionLinesOnLoad();
        // does not propagate
        break;

    case Event::LoadFromMosaic:
         qCritical("use takeDown for TILM_LOAD_FROM_MOSAIC");
         break;

    case Event::UnloadTiling:
        tilings.removeOne(tilingEvent.tiling);
        selectTiling(tilings.first());
        motifEvent.event  =  MotifMaker::Event::TilingDeleted;
        motifEvent.tiling = tilingEvent.tiling;
        motifMaker->sm_takeUp(motifEvent);
        break;
    }

    if (Sys::config->tm_fillOnLoad)
    {
        Sys::tilingMakerView->setFill(true);
    }

    if (event == Event::LoadReplace && Sys::mosaicMaker->getMosaic()->getTilings().contains(tiling))
    {
        Sys::setActiveCanvas(ACTIVE_MOSAIC);
    }
    else
    {
        Sys::setActiveCanvas(ACTIVE_TILING);
    }

    Sys::systemViewController->setTakerViewSize(Sys::getActiveCanvas()->getViewSize());

    emit Sys::systemViewController->sig_resetLayers();

    if (event != Event::LoadReplace)
    {
        Sys::systemViewController->setSelectedPrimaryLayer(tiling);
    }
}

void TilingMaker::selectTiling(TilingPtr tiling)
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

    selectTile(nullptr);
    Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    Sys::tilingMakerView->clearViewData();
    Sys::tilingMakerView->setTiling(tiling);
    if (Sys::systemViewController->isEnabled(Layer::Type::TilingMaker))
    {
        Sys::tilingMakerView->forceLayerRecalc(true);
    }

    tiling->setTilingViewChanged();

    tiling->getSaveStatus()->init();    // this is a newly loaded tiling and does not need saving
}

TilingMaker::State TilingMaker::sm_getState()
{
    if (tilings.size() > 1)
    {
        return  State::Multi;
    }
    else if (tilings.size() == 0)
    {
        return State::Empty;
    }
    else
    {
        Q_ASSERT(tilings.size() == 1);
        return State::Single;
    }
}

void TilingMaker::removeExcludeds()
{
    selectedTiling->unit().removeExcludeds();

    emit sig_menuRefresh(TMR_TILING_UNIT);
}

bool TilingMaker::verifyTiling()
{
    TilingPtr tp = getSelectedTiling();

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

    const FillData & fd = tp->hdr().getTilingCanvas()->getFillData();
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

void TilingMaker::setMouseMode(eTilingMakerMouseMode mode)
{
    qDebug() << "tiling maker mouse mode" << sTilingMakerMouseMode[(int)mode];

    _tilingMakerMouseMode = mode;

    unifyBase.reset();
    Sys::tilingMakerView->getAccumW().clear();

    if (mode != MouseMode::Edit)
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
    case MouseMode::Edit:
        decomposedTiling.reset();
        break;

    case MouseMode::TranslationVectors:
        decomposedTiling.reset();
        clearTranslationVectors();
        vectorState = VEC_SETTING_T1;
        break;

    case MouseMode::Decompose:
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
void TilingMaker::replaceTilingUnit(TilingUnitController & tilingUnit)
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
        tilingEvent.event  = Event::UnloadTiling;
        tilingEvent.tiling = tiling;
        sm_takeUp(tilingEvent);
    }
    Q_ASSERT(current == selectedTiling);

    // replace the placed tile
    selectedTiling->unit().replaceUnitData(tilingUnit);

    // it's a new ball game
    MotifEvent motifEvent;
    motifEvent.event =  MotifMaker::Event::LoadSingle;
    motifEvent.tiling  = selectedTiling;
    motifMaker->sm_takeUp(motifEvent);

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


void TilingMaker::selectTile(PlacedTilePtr ptp, bool toggle)
{
    if (toggle)
    {
        if (ptp == _selectedPlacedTile)
        {
            //qDebug() << "deselect";
            _selectedPlacedTile = nullptr;;
        }
        else
        {
            //qDebug() << "select"  << ptp.get();
            _selectedPlacedTile = ptp;
        }
    }
    else
    {
        //qDebug() << "select"  << ptp.get();
        _selectedPlacedTile = ptp;
    }
}

void TilingMaker::slot_deleteTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        deleteTile(Sys::tilingMakerView->viewPlacedTileSelector());
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    }
}

void TilingMaker::deleteTile(PlacedTileSelectorPtr sel)
{
    if (!sel) return;

    PlacedTilePtr tile = sel->getPlacedTile();
    auto tselect = Sys::tilingMakerView->viewPlacedTileSelector();
    if (tselect &&  tselect->getPlacedTile() == tile)
    {
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
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
        selectTile(nullptr);
    }
    selectedTiling->unit().removePlacedTile(ptp);

    for (int i = anchors.size() - 1; i >= 0; i--)
    {
        if (anchors.at(i).tileA == ptp || anchors.at(i).tileB == ptp)
            anchors.removeAt(i);
    }

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingUnitChanged;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
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

        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingUnitChanged;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
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

    MotifEvent motifEvent;
    motifEvent.event  =  MotifMaker::Event::TilingUnitChanged;
    motifEvent.tiling = tiling;
    motifMaker->sm_takeUp(motifEvent);

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

    MotifEvent pevent;
    pevent.event  =  MotifMaker::Event::TilingUnitChanged;
    pevent.tiling = selected;
    motifMaker->sm_takeUp(pevent);

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
        tevent.event  = Event::LoadMulti;
        tevent.tiling = decomposedTiling;
        sm_takeUp(tevent);
    }
    else
    {
        decomposedTiling->unit().addPlacedTile(pf->copy());

        MotifEvent pevent;
        pevent.event  =  MotifMaker::Event::TilingUnitChanged;
        pevent.tiling = decomposedTiling;
        motifMaker->sm_takeUp(pevent);
    }

    if (getMouseMode() != MouseMode::Decompose)
    {
        setMouseMode(MouseMode::Decompose); // restore
    }

    if (getSelectedTiling() != selected)
    {
        selectTiling(selected);      // restore
    }

    emit sig_menuRefresh(TMR_MAKER_TILINGS);

    Q_ASSERT(getSelectedTiling()  == selected);
    Q_ASSERT(getMouseMode() ==  MouseMode::Decompose);
}

void TilingMaker::addPlacedTile(PlacedTilePtr placedTile)
{
    selectedTiling->unit().addPlacedTile(placedTile);

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event =  MotifMaker::Event::TilingUnitChanged;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
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
    tilingEvent.event  = Event::LoadMulti;
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
        TilingUnitController tu = old->unit();
        newTiling->unit().addUnitData(tu);
    }

    // give it a name
    VersionedName vname = FileServices::getNextVersion(FILE_TILING,tilings.first()->getVName());
    newTiling->setVName(vname);

    // replace the tilings with the new merged tiling
    TilingEvent tevent;
    tevent.event = Event::ReplaceAll;
    tevent.tiling = newTiling;
    sm_takeUp(tevent);

    emit sig_menuRefresh(TMR_MAKER_TILINGS);
}

void TilingMaker::updateVectors()
{
    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event = MotifMaker::Event::TilingChanged;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
    }

    MapEditor * editor = Sys::mapEditor;
    if (editor->getMapType(editor->getEditMap()) == MapEditorMap::MapType::TilingFull)
    {
        Sys::mapEditor->loadTilingRepeated();
    }
}

void TilingMaker::updateReps()
{
    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event =  MotifMaker::Event::TilingChanged;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
    }

    MapEditor * editor = Sys::mapEditor;
    if (editor->getMapType(editor->getEditMap()) == MapEditorMap::MapType::TilingFull)
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
            MotifEvent motifEvent;
            motifEvent.event =  MotifMaker::Event::TilingChanged;
            motifEvent.tiling = tiling;
            Sys::motifMaker->sm_takeUp(motifEvent);
        }
    }
}

void TilingMaker::slot_showTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr pf = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        pf->setShow(true);
        emit sig_menuRefresh(TMR_TILING_UNIT);
        forceRedraw();
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    }
}

void TilingMaker::slot_hideTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr pf = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        pf->setShow(false);
        emit sig_menuRefresh(TMR_TILING_UNIT);
        forceRedraw();
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    }
}

void TilingMaker::slot_deselectTile()
{
    selectTile(nullptr);
    Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
    forceRedraw();
}

void TilingMaker::slot_includeTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr ptp = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        if (ptp->getState().excluded.get() != Tristate::eTristate::False)
        {
            selectedTiling->unit().addPlacedTile(ptp);
            ptp->include();
            selectedTiling->setTilingViewChanged();
            if (getPropagate())
            {
                MotifEvent motifEvent;
                motifEvent.event = MotifMaker::Event::TilingUnitChanged;
                motifEvent.tiling = selectedTiling;
                motifMaker->sm_takeUp(motifEvent);
            }
            emit sig_menuRefresh(TMR_TILING_UNIT);
            forceRedraw();
            Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
        }
    }
}

void TilingMaker::slot_excludeTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr ptp = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        if (ptp->getState().excluded.get() != Tristate::eTristate::True)
        {
            selectedTiling->unit().addPlacedTile(ptp);
            ptp->exclude();
            selectedTiling->setTilingViewChanged();
            if (getPropagate())
            {
                MotifEvent motifEvent;
                motifEvent.event =  MotifMaker::Event::TilingUnitChanged;
                motifEvent.tiling = selectedTiling;
                motifMaker->sm_takeUp(motifEvent);
            }
            Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
            forceRedraw();
            emit sig_menuRefresh(TMR_TILING_UNIT);
        }
    }
}

void TilingMaker::slot_view_menu_editTile()
{
    // called by menu from click on tile in view
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr ptp = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        editTile(ptp);
    }
}

void TilingMaker::editTile(PlacedTilePtr ptp)
{
    if (!ptp)
    {
        return;
    }

    setMouseMode(MouseMode::Edit);

    TilePtr tile = ptp->getTile();
    QTransform t = ptp->getPlacement();

    DlgEdgePolyEdit * epedit = getEdgePolyEditor();
    if (epedit)
    {
        delete epedit;
    }

    epedit = new DlgEdgePolyEdit(getSelectedTiling(),tile,t,Sys::controlPanel);
    epedit->show();
    epedit->raise();
    epedit->activateWindow();

    Sys::tilingMakerView->setEditPlacedTile(ptp);
    setEdgePolyEditor(epedit);

    connect(this, &TilingMaker::sig_menuRefresh, epedit, &DlgEdgePolyEdit::display);
}

void TilingMaker::slot_copyMoveTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyMovePolygon>(Sys::tilingMakerView->viewPlacedTileSelector(), clickedSpt));
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinPoint()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinPoint>(Sys::tilingMakerView->viewPlacedTileSelector(), clickedSpt));
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinMidPoint()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinMidPoint>(Sys::tilingMakerView->viewPlacedTileSelector(), clickedSpt));
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_copyJoinEdge()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        Sys::tilingMakerView->setMouseInteraction(make_shared<CopyJoinEdge>(Sys::tilingMakerView->viewPlacedTileSelector(), clickedSpt));
        Sys::tilingMakerView->setViewPlacedTileSelector(nullptr);
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr pf = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();

        uniquifyTile(pf);

        forceRedraw();
        emit sig_menuRefresh(TMR_PLACED_TILE);

    }
}

void TilingMaker::slot_convertTile()
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        PlacedTilePtr pf = Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile();
        TilePtr tile = pf->getTile();

        flipTileRegularity(tile);

        emit sig_menuRefresh(TMR_PLACED_TILE);
    }
}

void TilingMaker::moveTileTo(QPointF pt)
{
    if (Sys::tilingMakerView->viewPlacedTileSelector())
    {
        selectTile(Sys::tilingMakerView->viewPlacedTileSelector()->getPlacedTile());
        placedTileSetTranslate(pt.x(), pt.y());

        if (getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
            motifEvent.tiling = selectedTiling;
            motifEvent.ptile  = selectedTile();
            motifMaker->sm_takeUp(motifEvent);
        }

        emit sig_menuRefresh(TMR_PLACED_TILE);
        forceRedraw();
    }
}

void TilingMaker::flipTileRegularity(TilePtr tile)
{
    tile->flipRegularity();

    MotifEvent motifEvent;
    motifEvent.event  = MotifMaker::Event::UniqueTileRegularityChanged;
    motifEvent.tiling = selectedTiling;
    motifEvent.tile   = tile;
    motifMaker->sm_takeUp(motifEvent);

    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaX(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    PlacedTileSet placements = selectedTiling->unit().getWithExcluded();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingModified;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaY(qreal delta)
{
    qreal qdelta = 0.01 * delta;
    PlacedTileSet placements = selectedTiling->unit().getWithExcluded();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingModified;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);

    PlacedTileSet placements = selectedTiling->unit().getWithExcluded();
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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingModified;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;

    PlacedTileSet placements = selectedTiling->unit().getWithExcluded();
    for (const auto & pfp : std::as_const(placements))
    {
        QTransform t = pfp->getPlacement();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setPlacement(t);
    }

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::TilingModified;
        motifEvent.tiling = selectedTiling;
        motifMaker->sm_takeUp(motifEvent);
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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.ptile  = selectedTile();
        motifMaker->sm_takeUp(motifEvent);
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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.ptile  = selectedTile();
        motifMaker->sm_takeUp(motifEvent);
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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.ptile  = selectedTile();
        motifMaker->sm_takeUp(motifEvent);
    }

    if (Sys::config->tm_tileAnchorMode)
        propagateAnchors(selectedTile());

    emit sig_menuRefresh(TMR_PLACED_TILE);
}

// re-translates a changed placement so the tile's drawn (distorted) centroid stays where it was
static QTransform pivotOnCenter(PlacedTilePtr ptp, const QTransform & before, const QTransform & after)
{
    QPointF c = Sys::getActiveDistortion().map(ptp->getTile()->getCenter());
    QPointF d = before.map(c) - after.map(c);
    return after * QTransform::fromTranslate(d.x(),d.y());
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
    t = pivotOnCenter(selectedTile(),t,xf.getTransform());
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.ptile  = selectedTile();
        motifMaker->sm_takeUp(motifEvent);
    }

    if (Sys::config->tm_tileAnchorMode)
        propagateAnchors(selectedTile());

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
    t = pivotOnCenter(selectedTile(),t,xf.getTransform());
    selectedTile()->setPlacement(t);

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::PlaceTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.ptile  = selectedTile();
        motifMaker->sm_takeUp(motifEvent);
    }

    if (Sys::config->tm_tileAnchorMode)
        propagateAnchors(selectedTile());

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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    if (Sys::config->tm_tileAnchorMode)
        propagateAnchors(selectedTile());

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
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileTransformChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    if (Sys::config->tm_tileAnchorMode)
        propagateAnchors(selectedTile());

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

////////////////////////////////////////////////////////////////////////////
//
// Tile anchors.
//
// An anchor is a persistent link between two placed tiles' edge midpoints,
// created by the user via a right-click on a touching midpoint. When anchor
// mode is enabled, adjusting the selected tile re-snaps every tile reachable
// through the anchor graph back into contact, using the same rotate+translate
// logic as the interactive midpoint snap (JoinMidPoint::snapTo).
//
////////////////////////////////////////////////////////////////////////////

// Resolves the current Edge at edgeIndex within tile's underlying Tile shape. Anchors store an
// index rather than an EdgePtr because scaling/rotating the shared Tile shape replaces every
// Edge object it owns (EdgePoly::compose()); the index survives that, a stored EdgePtr wouldn't.
EdgePtr TilingMaker::edgeAt(PlacedTilePtr tile, int edgeIndex)
{
    if (!tile || !tile->getTile())
        return EdgePtr();

    const EdgeSet & edges = tile->getTile()->getEdgePoly().get();
    if (edgeIndex < 0 || edgeIndex >= edges.size())
        return EdgePtr();

    return edges.at(edgeIndex);
}

void TilingMaker::slot_createAnchor()
{
    auto sel = Sys::tilingMakerView->viewPlacedTileSelector();
    if (!sel || sel->getType() != MID_POINT)
        return;

    auto tosel = Sys::tilingMakerView->findMidPoint(clickedSpt, sel);
    if (!tosel)
        return;     // nothing coincident here - no-op

    PlacedTilePtr tileA = sel->getPlacedTile();
    PlacedTilePtr tileB = tosel->getPlacedTile();
    int indexA = tileA->getTile()->getEdgePoly().get().indexOf(sel->getModelEdge());
    int indexB = tileB->getTile()->getEdgePoly().get().indexOf(tosel->getModelEdge());
    if (indexA < 0 || indexB < 0)
        return;

    anchors.push_back({tileA, indexA, tileB, indexB});
    Sys::config->tm_tileAnchorMode = true;     // an anchor is useless until propagation is on
    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMaker::slot_removeAnchor()
{
    auto sel = Sys::tilingMakerView->viewPlacedTileSelector();
    if (!sel || sel->getType() != MID_POINT)
        return;     // no-op if there is nothing to remove

    PlacedTilePtr tile  = sel->getPlacedTile();
    int           index = tile->getTile() ? tile->getTile()->getEdgePoly().get().indexOf(sel->getModelEdge()) : -1;
    if (index < 0)
        return;

    for (int i = 0; i < anchors.size(); i++)
    {
        const TileAnchor & a = anchors.at(i);
        if ((a.tileA == tile && a.edgeIndexA == index) || (a.tileB == tile && a.edgeIndexB == index))
        {
            anchors.removeAt(i);
            forceRedraw();
            return;
        }
    }
}

// Rotates and translates moveTile so moveEdge coincides with fixedEdge, mirroring
// the interactive snap in JoinMidPoint::snapTo.
void TilingMaker::snapEdgeToEdge(PlacedTilePtr moveTile, EdgePtr moveEdgeModel, PlacedTilePtr fixedTile, EdgePtr fixedEdgeModel)
{
    if (!moveTile || !moveEdgeModel || !fixedTile || !fixedEdgeModel)
        return;

    // moveTrans is the actual (canonical, undistorted) placement being accumulated below - only
    // the measurement edges need the distorted transform, so they angle/center-match what's
    // actually drawn on screen
    QTransform distortion = Sys::getActiveCanvas()->getDistorionTransform();

    QTransform moveTrans = moveTile->getPlacement();

    auto moveEdge  = make_shared<Edge>(moveEdgeModel, distortion * moveTrans);
    auto fixedEdge = make_shared<Edge>(fixedEdgeModel, distortion * fixedTile->getPlacement());

    if (!Sys::config->tm_snapOnly)
    {
        qreal fromAngle = qRadiansToDegrees(moveEdge->angle());
        qreal toAngle   = qRadiansToDegrees(fixedEdge->angle()) - 180.0;    // opposite directions by definition
        qreal angle     = toAngle - fromAngle;

        if (!Loose::zero(angle))
        {
            QTransform t0;
            t0.rotate(angle);
            moveTrans *= t0;
            moveTile->setPlacement(moveTrans);
        }
    }

    moveEdge      = make_shared<Edge>(moveEdgeModel, distortion * moveTrans);  // it could have moved
    QPointF delta = fixedEdge->getLine().center() - moveEdge->getLine().center();
    moveTrans    *= QTransform::fromTranslate(delta.x(), delta.y());
    moveTile->setPlacement(moveTrans);
}

// Walks the anchor graph outward from root (which stays fixed), re-snapping every
// reachable tile back into contact. BFS with a visited list means chains resolve
// outward in order and cycles/conflicting anchors simply stop once a tile is
// revisited (last write wins).
void TilingMaker::propagateAnchors(PlacedTilePtr root)
{
    updateAnchoredTranslationVectors();

    if (!root || anchors.isEmpty())
        return;

    QVector<PlacedTilePtr> visited;
    visited.push_back(root);

    QVector<PlacedTilePtr> queue;
    queue.push_back(root);

    while (!queue.isEmpty())
    {
        PlacedTilePtr current = queue.takeFirst();
        for (const TileAnchor & a : std::as_const(anchors))
        {
            PlacedTilePtr other;
            int otherIndex;
            int currentIndex;

            if (a.tileA == current)
            {
                other = a.tileB; otherIndex = a.edgeIndexB; currentIndex = a.edgeIndexA;
            }
            else if (a.tileB == current)
            {
                other = a.tileA; otherIndex = a.edgeIndexA; currentIndex = a.edgeIndexB;
            }
            else
            {
                continue;
            }

            if (visited.contains(other))
                continue;

            snapEdgeToEdge(other, edgeAt(other, otherIndex), current, edgeAt(current, currentIndex));
            visited.push_back(other);
            queue.push_back(other);
        }
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_PLACED_TILE);
}

// Binds an anchor to whatever tile sel points at, in that tile's local (pre-placement) frame.
// Returns an unanchored TransAnchor if sel (or its tile) is null.
TransAnchor TilingMaker::bindTranslationAnchor(PlacedTileSelectorPtr sel)
{
    TransAnchor a;
    if (!sel)
        return a;

    PlacedTilePtr tile = sel->getPlacedTile();
    if (!tile)
        return a;

    a.tile    = tile;
    a.localPt = tile->getPlacement().inverted().map(sel->getPlacedPoint());
    return a;
}

// (Re)binds all three translation-vector points against whatever tile currently occupies each
// spot on screen, using the same hit-testing already used for clicks. Called when Anchor Mode is
// turned on with vectors already in place (freshly drawn or loaded from file).
void TilingMaker::resolveTranslationAnchors()
{
    if (!selectedTiling)
        return;

    QPointF origin = selectedTiling->hdr().getTranslateOrigin();
    QPointF t1     = selectedTiling->hdr().getTrans1();
    QPointF t2     = selectedTiling->hdr().getTrans2();

    auto resolve = [](QPointF modelPt) -> TransAnchor
    {
        // modelPt is a stored T0/T1/T2-derived point, drawn via the undistorted transform -
        // find whatever tile currently renders at that same screen position.
        QPointF spt = Sys::tilingMakerView->undistortedModelToScreen(modelPt);
        PlacedTileSelectorPtr sel = Sys::tilingMakerView->findSelection(spt);
        TransAnchor a;
        if (sel && sel->getPlacedTile())
        {
            a.tile    = sel->getPlacedTile();
            a.localPt = sel->getPlacedTile()->getPlacement().inverted().map(modelPt);
        }
        return a;
    };

    originAnchor = resolve(origin);
    t1Anchor     = resolve(origin + t1);
    t2Anchor     = resolve(origin + t2);

    forceRedraw();
}

// Called when Anchor Mode is turned off - unlike tile anchors (persistent links created and
// removed explicitly), translation-vector anchors have no lifecycle of their own; they exist
// only as a byproduct of Anchor Mode, so switching it off drops them.
void TilingMaker::clearTranslationAnchors()
{
    originAnchor = TransAnchor();
    t1Anchor     = TransAnchor();
    t2Anchor     = TransAnchor();
    forceRedraw();
}

// Each anchored translation-vector point tracks its own tile's current placement; each
// unanchored point stays fixed wherever it last was in absolute space.
void TilingMaker::updateAnchoredTranslationVectors()
{
    if (!selectedTiling)
        return;
    if (!originAnchor.tile && !t1Anchor.tile && !t2Anchor.tile)
        return;

    QPointF oldOrigin = selectedTiling->hdr().getTranslateOrigin();
    QPointF oldT1     = selectedTiling->hdr().getTrans1();
    QPointF oldT2     = selectedTiling->hdr().getTrans2();

    // tile placements are true (undistorted) model points, but T0/T1/T2 render via the
    // undistorted transform like construction lines - pre-compensate so the vector stays
    // visually glued to its anchored tile(s) regardless of distortion.
    auto toCanvas = [](QPointF p)
    {
        return Sys::tilingMakerView->undistortedScreenToModel(Sys::tilingMakerView->modelToScreen(p));
    };

    QPointF newOrigin = originAnchor.tile ? toCanvas(originAnchor.tile->getPlacement().map(originAnchor.localPt)) : oldOrigin;
    QPointF newT1End  = t1Anchor.tile     ? toCanvas(t1Anchor.tile->getPlacement().map(t1Anchor.localPt))         : (oldOrigin + oldT1);
    QPointF newT2End  = t2Anchor.tile     ? toCanvas(t2Anchor.tile->getPlacement().map(t2Anchor.localPt))         : (oldOrigin + oldT2);

    selectedTiling->hdr().setTranslationVectors(newT1End - newOrigin, newT2End - newOrigin, newOrigin);
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

void TilingMaker::addToTranslate(QLineF mLine, QPointF origin, PlacedTileSelectorPtr originSel, PlacedTileSelectorPtr endSel)
{
    // T0/T1/T2 are just fixed canvas-space numbers, like construction lines - they don't keep
    // tracking whatever tile point they were drawn from (that's what Vector Anchor Mode is for).
    // A point that came from a tile selector is a true (undistorted) tile point, but the tile
    // renders distorted, so re-express it as wherever it actually appears on screen right now -
    // the same pre-compensation a construction line snapped to a tile gets in
    // findConstructionSnapCandidate(). A point with no selector (construction-line snap, or a
    // free click) is already canvas-space and must be used as-is.
    auto toCanvas = [](QPointF p)
    {
        return Sys::tilingMakerView->undistortedScreenToModel(Sys::tilingMakerView->modelToScreen(p));
    };
    if (originSel)
        origin = toCanvas(origin);
    QPointF endPt = endSel ? toCanvas(mLine.p2()) : mLine.p2();
    mLine         = QLineF(origin, endPt);

    QPointF tran = mLine.p2() - mLine.p1();

    QPointF t1 = selectedTiling->hdr().getTrans1();
    QPointF t2 = selectedTiling->hdr().getTrans2();

    Q_ASSERT(vectorState != VEC_READY);
    bool wasSettingT1 = (vectorState == VEC_SETTING_T1);
    if (vectorState == VEC_SETTING_T1)
    {
        t1 = tran;
        vectorState = VEC_SETTING_T2;
    }
    else if (vectorState == VEC_SETTING_T2)
    {
        t2 = tran;
        vectorState = VEC_READY;
        setMouseMode(MouseMode::None);
    }

    // a singleton tiling has no repeat by definition - defining a real translation vector means
    // it isn't one anymore
    Canvas * canvas = selectedTiling->hdr().getTilingCanvas();
    FillData  fd     = canvas->getFillData();
    bool singleton; int minX,maxX,minY,maxY;
    fd.get(singleton,minX,maxX,minY,maxY);
    if (singleton)
    {
        fd.set(false,-3,3,-3,3);
        canvas->setFillData(fd);
    }

    if (Sys::config->tm_vectorAnchorMode)
    {
        originAnchor = bindTranslationAnchor(originSel);
        (wasSettingT1 ? t1Anchor : t2Anchor) = bindTranslationAnchor(endSel);
    }
    else
    {
        originAnchor = TransAnchor();
        (wasSettingT1 ? t1Anchor : t2Anchor) = TransAnchor();
    }

    selectedTiling->hdr().setTranslationVectors(t1,t2,origin);
    selectedTiling->setTilingViewChanged();

    MotifEvent motifEvent;
    motifEvent.event  =  MotifMaker::Event::TilingChanged;
    motifEvent.tiling = selectedTiling;
    motifMaker->sm_takeUp(motifEvent);

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

        selectedTiling->setTilingViewChanged();

        if (getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::TilingUnitChanged;
            motifEvent.tiling = selectedTiling;
            motifMaker->sm_takeUp(motifEvent);
        }

        emit sig_menuRefresh(TMR_TILING_HEADER);
        forceRedraw();
    }
}

void TilingMaker::sm_syncConstructionLinesOnLoad()
{
    if (Sys::systemViewController->isEnabled(Layer::Type::ConstructionLines))
        loadConstructionLines();
    else
        clearConstructionLines();
}

void TilingMaker::saveConstructionLines()
{
    if (selectedTiling)
    {
        selectedTiling->setConstructionLines(Sys::tilingMakerView->getConstructionLines());
    }
}

void TilingMaker::loadConstructionLines()
{
    if (selectedTiling)
    {
        Sys::tilingMakerView->getConstructionLines() = selectedTiling->getConstructionLines();
        Sys::tilingMakerView->setSelectedConstructionLine(-1);
        forceRedraw();
    }
}

// view lines are only synced with the selected tiling while the layer is enabled
bool TilingMaker::constructionLinesNotSaved(TilingPtr tiling)
{
    return tiling && tiling == selectedTiling
        && Sys::systemViewController->isEnabled(Layer::Type::ConstructionLines)
        && (Sys::tilingMakerView->getConstructionLines() != tiling->getConstructionLines());
}

void TilingMaker::slot_deleteConstructionLine()
{
    int idx = Sys::tilingMakerView->getSelectedConstructionLine();
    if (idx >= 0)
    {
        Sys::tilingMakerView->getConstructionLines().remove(idx);
        Sys::tilingMakerView->setSelectedConstructionLine(-1);
        Sys::tilingMakerView->forceRedraw();
    }
}

void TilingMaker::clearTranslationVectors()
{
    if (selectedTiling)
    {
        selectedTiling->hdr().setTranslationVectors(QPointF(),QPointF(),QPointF());
        selectedTiling->setTilingViewChanged();
        clearTranslationAnchors();     // stale anchors would otherwise resurrect the vectors
        forceRedraw();
        emit sig_menuRefresh(TMR_TILING_HEADER);
    }
}

void TilingMaker::addRegularPolygon()
{
    TilePtr f;
    if (Sys::config->tm_poly_side_count > 2)
    {
        f = make_shared<Tile>(Sys::config->tm_poly_side_count,Sys::config->tm_poly_rotation);
    }
    else
    {
        Circle c(QPointF(0,0), 1.0);
        EdgePoly ep;
        ep.set(c);

        f = make_shared<Tile>(ep);
        f->setRotate(Sys::config->tm_poly_rotation);
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
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::UniqueTileTransformChanged;
            motifEvent.tiling = selectedTiling;
            motifEvent.tile   = tile;
            motifMaker->sm_takeUp(motifEvent);
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
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::UniqueTileTransformChanged;
            motifEvent.tiling = selectedTiling;
            motifEvent.tile   = tile;
            motifMaker->sm_takeUp(motifEvent);
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
                    MotifEvent motifEvent;
                    motifEvent.event  = MotifMaker::Event::UniqueTileTransformChanged;
                    motifEvent.tiling = selectedTiling;
                    motifEvent.tile   = tile;
                    motifMaker->sm_takeUp(motifEvent);
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
    if (Sys::systemViewController->isEnabled(Layer::Type::TilingMaker))
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
    PlacedTileSelectorPtr tsel  = Sys::tilingMakerView->viewPlacedTileSelector();

    EdgePtr edege   = tsel->getModelEdge();
    edege->convertToCurve(ctype);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    Q_ASSERT(tile->epolyContains(edege));
    tile->decompose();

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
    emit sig_menuRefresh(TMR_TILING_UNIT);
    qDebug() << "edge converted to CURVE";
}

void TilingMaker::slot_flatenCurve()
{
    auto tsel    = Sys::tilingMakerView->viewPlacedTileSelector();
    EdgePtr edge = tsel->getModelEdge();
    edge->resetCurveToLine();

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
    qDebug() << "edge converted to LINE";
}

void TilingMaker::slot_makeConvex()
{
    auto tsel    = Sys::tilingMakerView->viewPlacedTileSelector();
    EdgePtr edge = tsel->getModelEdge();
    Q_ASSERT(edge->isCurve());

    edge->changeCurveType(CURVE_CONVEX);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  = MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_makeConcave()
{
    auto tsel    = Sys::tilingMakerView->viewPlacedTileSelector();
    EdgePtr edge = tsel->getModelEdge();
    Q_ASSERT(edge->isCurve());

    edge->changeCurveType(CURVE_CONCAVE);

    auto ptile = tsel->getPlacedTile();
    Q_ASSERT(ptile);
    auto tile  = ptile->getTile();
    tile->decompose();

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_moveArcCenter()
{
    auto tsel  = Sys::tilingMakerView->viewPlacedTileSelector();
    Sys::tilingMakerView->setMouseInteraction(make_shared<EditEdge>(tsel,QPointF()));

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  =  MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tsel->getPlacedTile()->getTile();
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
}

void TilingMaker::slot_editMagnitude()
{
    auto tsel  = Sys::tilingMakerView->viewPlacedTileSelector();
    Sys::tilingMakerView->resetMouseInteraction();
    DlgMagnitude dlg(tsel);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();

    if (getPropagate())
    {
        MotifEvent motifEvent;
        motifEvent.event  = MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = selectedTiling;
        motifEvent.tile   = tsel->getPlacedTile()->getTile();
        motifMaker->sm_takeUp(motifEvent);
    }

    forceRedraw();
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

void TilingMaker::setKeybdMode(KeybdMode mode)
{
    if (mode != _tilingMakerKeyboardMode)
    {
        _tilingMakerKeyboardMode = mode;
        qDebug().noquote() << "Keyboard Mode is:" << getKeybdModeString();
        emit sig_TMKbdMode(mode);
    }
}

void TilingMaker::resetKeybdMode()
{
    setKeybdMode(KeybdMode::XformAll);
    emit sig_TMKbdMode(_tilingMakerKeyboardMode);
}

bool TilingMaker::isKeybdMode(KeybdMode mode)
{
    return (mode == _tilingMakerKeyboardMode);
}

QString TilingMaker::getKeybdModeString()
{
    return sTMModelMode[_tilingMakerKeyboardMode];
}

bool TilingMaker::procKeyEvent(QKeyEvent * k)
{
    if (!Sys::systemViewController->isEnabled(Layer::Type::TilingMaker))
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
        case 'T': setKeybdMode(KeybdMode::XformTiling); break;
        case 'V': setKeybdMode(KeybdMode::XformAll); break;

        case 'X': clearTranslationVectors(); break;
        case '0': Sys::config->tm_poly_side_count = 0; break;
        case '1': Sys::config->tm_poly_side_count = 1; break;
        case '2': Sys::config->tm_poly_side_count = 2; break;
        case '3': Sys::config->tm_poly_side_count = 3; break;
        case '4': Sys::config->tm_poly_side_count = 4; break;
        case '5': Sys::config->tm_poly_side_count = 5; break;
        case '6': Sys::config->tm_poly_side_count = 6; break;
        case '7': Sys::config->tm_poly_side_count = 7; break;
        case '8': Sys::config->tm_poly_side_count = 8; break;
        case '9': Sys::config->tm_poly_side_count = 9; break;

        case Qt::Key_F1: Shortcuts::popup(Layer::Type::TilingMaker); break;

        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if (getMouseMode() == MouseMode::ConstructionLines)
            {
                slot_deleteConstructionLine();
            }
            else
            {
                return false;   // propagate - not ours to handle
            }
            break;

        // modes
        case Qt::Key_Escape: setMouseMode(MouseMode::None); return false;     // propagate
        case Qt::Key_F3:     setMouseMode(MouseMode::TranslationVectors); break;
        case Qt::Key_F4:     setMouseMode(MouseMode::DrawPolygon); break;
        case Qt::Key_F5:     setMouseMode(MouseMode::Copy); break;
        case Qt::Key_F6:     setMouseMode(MouseMode::Delete); break;
        case Qt::Key_F7:     setMouseMode(MouseMode::Inclusion); break;
        case Qt::Key_F9:     setMouseMode(MouseMode::Measure); break;
        case Qt::Key_F10:    setMouseMode(MouseMode::Unify); break;
        case Qt::Key_F11:    setMouseMode(MouseMode::Edit); break;
        case Qt::Key_F12:    setMouseMode(MouseMode::CurveEdge); break;

        default: return false;
   }

   return true;
}

void TilingMaker::slot_stack_save()
{
    auto tiling  = getSelectedTiling();
    if (!tiling) return;

    TilingUnitController current = tiling->unit().uniqueCopy();

    tmStack.add(current);
}

void TilingMaker::slot_stack_undo()
{
    auto tiling  = getSelectedTiling();
    if (!tiling) return;

    TilingUnitController td = tiling->unit().uniqueCopy();

    selectTile(nullptr); // selection no longer needed

    bool rv = tmStack.pop(td);
    if (rv)
    {
        TilingUnitController copy = td.uniqueCopy();
        tiling->unit().replaceUnitData(copy);
        tiling->setTilingViewChanged();

        Sys::tilingMakerView->setTiling(tiling);

        if (getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::TilingChanged;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
        }

        emit sig_updateView();
        emit sig_menuRefresh(TMR_TILING_UNIT);
    }
}

void TilingMaker::slot_stack_redo()
{
    auto tiling  = getSelectedTiling();
    if (!tiling) return;

    TilingUnitController td(tiling.get());

    selectTile(nullptr); // selection no longer needed

    bool rv = tmStack.redo(td);
    if (rv)
    {
        TilingUnitController copy = td.uniqueCopy();
        tiling->unit().replaceUnitData(copy);
        tiling->setTilingViewChanged();

        Sys::tilingMakerView->setTiling(tiling);

        if (getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  =  MotifMaker::Event::TilingChanged;
            motifEvent.tiling = tiling;
            motifMaker->sm_takeUp(motifEvent);
        }

        emit sig_updateView();
        emit sig_menuRefresh(TMR_TILING_UNIT);
    }
}

const QString sTILM_Events[] =
    {
        E2STR(LoadEmpty),
        E2STR(LoadFromMosaic),
        E2STR(LoadSingle),
        E2STR(LoadMulti),
        E2STR(LoadReplace),
        E2STR(ReplaceAll),
        E2STR(Reload),
        E2STR(LoadFromStepper),
        E2STR(UnloadTiling)
};

const QString sTilingMakerMouseMode[] =
    {
        E2STR(None),
        E2STR(Copy),
        E2STR(Delete),
        E2STR(TranslationVectors),
        E2STR(DrawPolygon),
        E2STR(Inclusion),
        E2STR(Position),
        E2STR(Measure),
        E2STR(Edit),
        E2STR(CurveEdge),
        E2STR(MirrorX),
        E2STR(MirrorY),
        E2STR(ConstructionLines),
        E2STR(ReflectEdge),
        E2STR(Uniquify),
        E2STR(Unify),
        E2STR(Decompose),
};

/////////////////////////////////////////////////////////////////
///
/// TMStack
///
/////////////////////////////////////////////////////////////////

bool TMStack::add(TilingUnitController & td)
{
    if (stack.isEmpty())
    {
        Q_ASSERT(stackIndex == -1);
        stack.push_back(td);
        stackIndex = 0;
    }
    else
    {
        TilingUnitController stackI  = stack[stackIndex];
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

bool TMStack::pop(TilingUnitController & td)
{
    if (stack.size() == 0)
        return false;

    if (stackIndex < 0)
        return false;

    TilingUnitController current = td;
    TilingUnitController stackI  = stack[stackIndex];
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

bool TMStack::redo(TilingUnitController & td)
{
    if (stack.isEmpty())
        return false;

    if ((stackIndex + 1) >= stack.size())
        return false;

    stackIndex++;
    td = stack[stackIndex];

    return true;
}

QString TMStack::getStackStatus()
{
    return QString("Depth: %1 Index: %2").arg(stack.size()).arg(stackIndex);
}