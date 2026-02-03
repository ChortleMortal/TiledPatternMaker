#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_editor_selection.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/map_editor_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"

using std::make_shared;

MapEditor::MapEditor()
{
//qDebug() << "MapEditor::MapEditor";
}

void MapEditor::init()
{
    db              = new MapEditorDb();
    selector        = new MapEditorSelection(db);

    Sys::mapEditorView->init(db,selector);

    config          = Sys::config;
    mosaicMaker     = Sys::mosaicMaker;
    prototypeMaker  = Sys::prototypeMaker;
    tilingMaker     = Sys::tilingMaker;

    timer           = nullptr;

    unload();

    connect(this, &MapEditor::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);
    connect(this, &MapEditor::sig_raiseMenu,  Sys::controlPanel,   &ControlPanel::slot_raisePanel);
}

MapEditor::~MapEditor()
{
    delete selector;
    db->reset();
    delete db;
}

void MapEditor::unload()
{
    db->reset();
    setMapedMouseMode(MAPED_MOUSE_NONE);
}

////////////////////////////////////////////////////////////////
//
// Loaders
//
////////////////////////////////////////////////////////////////


bool MapEditor::loadFromMosaicStyle(StylePtr style)
{
    qDebug() << "MapEditor::loadFromMosaicStyle";

    MapPtr map  = style->getStyleMap();
    if (!map || map->isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Mosaic style has no content map");
        box.exec();
        return false;
    }

    eMapedLayer lindex = db->obtainEmptyLayer();
    if (lindex != COMPOSITE)
    {
        MapEditorLayer & layer = db->getLayer(lindex);
        layer.setLayer(map,style);
        db->setEditSelect(lindex);
        db->setViewSelect(lindex,true);

        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }
    forceRedraw();

    return true;
}

void  MapEditor::loadFromfPrototypeMaker()
{
    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found");
        box.exec();
        return;
     }

    if (!proto->hasContent())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Prototype has no content");
        box.exec();
        return;
    }

    MapPtr map = proto->getProtoMap();

    eMapedLayer lindex = db->obtainEmptyLayer();
    if (lindex != COMPOSITE)
    {
        MapEditorLayer & layer = db->getLayer(lindex);
        layer.setLayer(map,proto);
        db->setEditSelect(lindex);
        db->setViewSelect(lindex,true);

        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }
    forceRedraw();
}

bool  MapEditor::loadSelectedMotifs()
{
    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Prototype Maker");
        box.exec();
        return false;
     }

    auto dels = prototypeMaker->getSelectedDELs(MVD_DELEM);
    if (dels.size() == 0)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No design elements found found for prototype");
        box.exec();
        return false;
    }

    TilingPtr tiling = proto->getTiling();

    for (const auto & del : std::as_const(dels))
    {
        db->getDesignElements().push_back(del);

        auto motif  = del->getMotif();
        if (motif)
        {
            MapPtr map           = motif->getMotifMap();
            TilePtr tile         = del->getTile();
            TilingPtr tiling     = del->getTiling();
            Q_ASSERT(tiling);
            QTransform placement = tiling->getFirstPlacement(tile);
            MapPtr tmap          = map->getTransformed(placement);

            eMapedLayer lindex = db->obtainEmptyLayer();
            if (lindex != COMPOSITE)
            {
                MapEditorLayer & layer = db->getLayer(lindex);
                layer.setLayer(map,proto,del,motif);
                db->setEditSelect(lindex);
                db->setViewSelect(lindex,true);

                if (config->mapEditorMode == MAPED_MODE_DCEL)
                {
                    if (!useExistingDCEL(map))
                        createLocalDCEL(map);
                }
            }
        }
    }
    forceRedraw();
    return true;
}

void MapEditor::loadTilingUnit()
{
    TilingPtr tp = tilingMaker->getSelected();

    if (!tp || tp->isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return;
    }

    MapPtr map = tp->createMapSingle();

    eMapedLayer lindex = db->obtainEmptyLayer();
    if (lindex != COMPOSITE)
    {
        MapEditorLayer & layer = db->getLayer(lindex);
        layer.setLayer(map,tp,MAPED_LOADED_FROM_TILING_UNIT);
        db->setEditSelect(lindex);
        db->setViewSelect(lindex,true);

        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }

    forceRedraw();
}

void  MapEditor::loadTilingRepeated()
{
    TilingPtr tp = tilingMaker->getSelected();
    if (!tp || tp->isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return;
    }

    MapPtr map = tp->createMapFullSimple();

    eMapedLayer lindex = db->obtainEmptyLayer();
    if (lindex != COMPOSITE)
    {
        MapEditorLayer & layer = db->getLayer(lindex);
        layer.setLayer(map,tp,MAPED_LOADED_FROM_TILING_REPEATED);
        db->setEditSelect(lindex);
        db->setViewSelect(lindex,true);

        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }

    forceRedraw();
}

void  MapEditor::loadFromMap(MapPtr map, eMapEditorMapType mtype)
{
    eMapedLayer lindex = db->obtainEmptyLayer();
    if (lindex != COMPOSITE)
    {
        MapEditorLayer & layer = db->getLayer(lindex);
        layer.setLayer(map,mtype);
        db->setEditSelect(lindex);
        db->setViewSelect(lindex,true);

        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }

    emit Sys::mapEditorView->sig_reconstructView(); // because it can load a background
}

bool MapEditor::useExistingDCEL(MapPtr map)
{
    if (!map)
        return false;

    auto dcel = map->getDerivedDCEL();
    if (dcel)
    {
        db->setLocalDCEL(dcel);
        return true;
    }

    return false;
}

bool MapEditor::createLocalDCEL(MapPtr map)
{
    if (!map)
        return false;

    DCELPtr dcel = make_shared<DCEL>(map.get());
    if (dcel->build())
    {
        db->setLocalDCEL(dcel);
        map->setDerivedDCEL(dcel);
        return true;
    }
    else
    {
        return false;
    }
}

void MapEditor::updateMosiacWithPrototype(ProtoPtr proto, eMOSM_Event event)
{
    MosaicEvent mevent;
    mevent.prototype = proto;
    mevent.event     = event;
    mosaicMaker->sm_takeUp(mevent);

    emit Sys::mapEditorView->sig_reconstructView(); // triggers createSyleRepresentation

    // does not switch view to mosaic
    qDebug().noquote() << "MapEditor::pushToMosaic - end";
}

////////////////////////////////////////////////////////////////
//
// Pushes
//
////////////////////////////////////////////////////////////////

bool MapEditor::pushToMosaic(MapEditorLayer & layer)
{
    qDebug().noquote() << "MapEditor::pushToMosaic" << Transform::info(Sys::mapEditorView->getLayerTransform());

    if (layer.getLayerMapType() == MAPED_LOADED_FROM_PROTOTMAKER)
    {
        eMOSM_Event event;
        if (config->mapedLoadCopies)
            event = MOSM_RELOAD_PROTO_SINGLE;   // single uses the shared ptr
        else
            event = MOSM_RELOAD_PROTO_MULTI;    // multi sets the prototype ptr too

        ProtoPtr proto = layer.getProto();
        updateMosiacWithPrototype(proto,event);

        return true;
    }

    // is there an existing mosaic?
    MosaicPtr mosaic  = mosaicMaker->getMosaic();
    MapPtr mosaic_map = mosaic->getFirstExistingPrototypeMap();
    if (!mosaic_map)
    {
        // this proably means there was no prototype, so make a prototype from the map
        MapPtr map = layer.getMapedLayerMap();
        qDebug() << map->info();

        ProtoPtr proto = make_shared<Prototype>(map);
        updateMosiacWithPrototype(proto,MOSM_LOAD_PROTO_SINGLE);

        return true;
    }

    // at this point should be a new map or style map
    switch (layer.getLayerMapType())
    {
    case MAPED_LOADED_FROM_STYLE:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_COMPOSITE:
        break;  // continue

    default:
        return false;
    }

    // put map into existing style prototype
    MapPtr map   = layer.getMapedLayerMap();

    //where does the style come from,
    //style->setStyleMap(map);


    auto olddcel = mosaic_map->getDerivedDCEL();

    if (mosaic_map != map)
    {
        //no it does not @!!@!!
        mosaic_map = map;    // this sets map into prototype
    }

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(mosaic_map))
        {
            createLocalDCEL(mosaic_map);
        }
    }
    else
    {
        if (olddcel)
        {
            auto dcel = std::make_shared<DCEL>(mosaic_map.get());
            if (dcel->build())
            {
                mosaic_map->setDerivedDCEL(dcel);
            }
            else
            {
                DCELPtr nulldcel;
                mosaic_map->setDerivedDCEL(nulldcel);
            }
        }
    }

    //Sys::render(RENDER_RESET_STYLES);
    emit sig_styleMapUpdated(map);

    return true;
}

bool MapEditor::pushToMotif(MapEditorLayer & layer)
{
    // pushing motif makes a new irregular explicit map motif which is either added or replaces
    // motif in design element or adds a new design element

    qDebug().noquote() << "MapEditor::convertToMotif" << Transform::info(Sys::mapEditorView->getLayerTransform());

    MapPtr map   = layer.getMapedLayerMap();

    eMapEditorMapType mtype = db->getMapType(map);
    if (!db->isMotif(mtype) && mtype != MAPED_TYPE_CREATED)
        return false;

    auto del   = prototypeMaker->getSelectedDEL();
    if (!del)
        return false;

    TilingPtr tiling = del->getTiling();
    Q_ASSERT(tiling);
    auto transform = tiling->getFirstPlacement(del->getTile()).inverted();
    auto map2      = map->getTransformed(transform);

    auto motif = make_shared<ExplicitMapMotif>(map2);
    motif->setTile(del->getTile());
    del->setMotif(motif);

    prototypeMaker->selectDesignElement(del);

    auto proto =  prototypeMaker->getSelectedPrototype();

    ProtoEvent pevent;
    pevent.event = PROM_MOTIF_CHANGED;
    pevent.tiling = proto->getTiling();
    prototypeMaker->sm_takeUp(pevent);

    qDebug().noquote() << "MapEditor::convertToMotif - end" << Transform::info(Sys::mapEditorView->getLayerTransform());
    return true;
}

// Replaces the complete set of tiles in the tiling unit
bool MapEditor::pushToTiling(MapEditorLayer & layer, bool outer)
{
    qDebug().noquote() << "MapEditor::pushToTiling" << Transform::info(Sys::mapEditorView->getLayerTransform());

    MapPtr map   = layer.getMapedLayerMap();
    qDebug() << map->summary();

    // converts the map to DCELs so that tiles can be made
    createLocalDCEL(map);
    DCELPtr dcel = db->getLocaldDCEL();
    qDebug() << dcel->info();

    const FaceSet & faces = dcel->getFaceSet();
    qDebug() << "num new faces =" << faces.size();
    if (faces.size() == 0)
    {
        qWarning() << "No faces: cannot create a tiling";
        return false;
    }

    TilingPtr tp = tilingMaker->getSelected();
    TilingUnit tilingUnit(tp.get());
    for (auto & face : std::as_const(faces))
    {
        if (  (!outer && !face->outer)
            ||( outer &&  face->outer))
        {
            EdgePoly ep     = *face;
            TilePtr fp      = make_shared<Tile>(ep);
            auto placedTile = make_shared<PlacedTile>(fp,QTransform());
            tilingUnit.addPlacedTile(placedTile);
        }
    }

    tilingMaker->replaceTilingUnit(tilingUnit);

    // aligns the tiling maker to the map editor too
    Sys::viewController->setSelectedPrimaryLayer(tp);

    const Xform & xf = Sys::mapEditorView->getModelXform();
    Sys::tilingMakerView->setModelXform(xf,true,Sys::nextSigid());

    qDebug().noquote() << "MapEditor::pushToTiling" << "- end" << Transform::info(Sys::mapEditorView->getLayerTransform());
    return true;
}

void MapEditor::setMapedMouseMode(eMapEditorMouseMode mode)
{
    db->setMouseMode(mode);

    db->resetMouseInteraction();      // only needed by border edit

    switch (mode)
    {
    default:
    case MAPED_MOUSE_NONE:
    case MAPED_MOUSE_DRAW_LINE:
    case MAPED_MOUSE_DELETE:
    case MAPED_MOUSE_SPLIT_LINE:
        Sys::controlPanel->restorePageStatus();
        break;

    case MAPED_MOUSE_EXTEND_LINE_P1:
    case MAPED_MOUSE_EXTEND_LINE_P2:
        Sys::controlPanel->overridePagelStatus("Click on edge and drag to extend. Press F to flip direction of extension");
        break;

    case MAPED_MOUSE_CREATE_LINE:
        Sys::controlPanel->overridePagelStatus("Click on map edge to  draw two lines at center");
        break;

    case MAPED_MOUSE_CONSTRUCTION_LINES:
        db->showConstructionLines = true;
        Sys::controlPanel->restorePageStatus();
        break;

    case MAPED_MOUSE_CONSTRUCTION_CIRCLES:
        Sys::controlPanel->overridePagelStatus("Right-click: create circle | Left-click-inside: drag to move circle  | Left-click-edge: resize circle");
        db->showConstructionLines = true;
        break;
    }
}

eMapEditorMouseMode MapEditor::getMapedMouseMode()
{
    return db->getMouseMode();
}

QString MapEditor::getStatus()
{
    QString s("Map Editor:: ");
    s += sMapEditorMouseMode[db->getMouseMode()];
    MapMouseActionPtr mma  = db->getMouseInteraction();
    if (mma)
    {
        s += "  ";
        s += mma->desc;
    }
    return s;
}

bool MapEditor::loadCurrentStash()
{
    bool rv = _stash.destash();
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

bool MapEditor::loadNextStash()
{
    _stash.getNext();
    bool rv = loadCurrentStash();
    return rv;
}

bool MapEditor::loadPrevStash()
{
    _stash.getPrev();
    bool rv = loadCurrentStash();
    return rv;
}

bool MapEditor::loadTemplate(VersionedFile xfile, bool animate)
{
    bool rv;
    if (animate)
    {
        rv = _stash.animateReadStash(xfile);
        if (!timer)
        {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [this] { _stash.nextAnimationStep(db,timer); emit sig_updateView(); } );
        }
        timer->start(500);
    }
    else
    {
        rv = _stash.readStash(xfile,db);
    }
    if (!rv) return false;

    forceRedraw();
    _stash.stash(db);

    return true;
}

bool MapEditor::saveTemplate(VersionedName & vname)
{
    return _stash.saveTemplate(vname);

}
bool MapEditor::keepStash(VersionedName vname)
{
    if (db->hasConstructionLines())
        return _stash.saveTemplate(vname);
    else
        return false;
}

bool MapEditor::initStashFrom(VersionedName mosaicname)
{
    // load any stash associated with the newly loaded mosaic
    bool rv = _stash.initStash(mosaicname,db);
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

void MapEditor::forceRedraw()
{
    if (Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
    {
        emit sig_updateView();
    }
}


/////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (!Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
    {
        return false;
    }

    int key = k->key();

    switch (key)
    {
    case Qt::Key_F1: Shortcuts::popup(VIEW_MAP_EDITOR); break;

    // modes
    case Qt::Key_Escape: setMapedMouseMode(MAPED_MOUSE_NONE);  return false; // propagate
    case Qt::Key_F3:     setMapedMouseMode(MAPED_MOUSE_DRAW_LINE); break;
    case Qt::Key_F4:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_LINES); break;
    case Qt::Key_F5:     setMapedMouseMode(MAPED_MOUSE_DELETE); break;
    case Qt::Key_F6:     setMapedMouseMode(MAPED_MOUSE_SPLIT_LINE); break;
    case Qt::Key_F7:     setMapedMouseMode(MAPED_MOUSE_EXTEND_LINE_P1); break;
    case Qt::Key_F9:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_CIRCLES); break;

    default: return false;
    }

    return true;
}
