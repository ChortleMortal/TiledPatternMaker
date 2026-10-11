#include "gui/map_editor/map_editor.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/viewers/map_editor_view.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/adaptive_motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/transform.h"

using std::make_shared;

using LayerId = MapEditorLayer::LayerId;

MapEditor::MapEditor()
{
    editSelect            = LayerId::L1;
    viewSelect            = (int)LayerId::None;

    __compositeLayer.init(LayerId::Composite,Qt::green);
    __layer1.init(LayerId::L1,QColorConstants::Svg::forestgreen);
    __layer2.init(LayerId::L2,QColorConstants::Svg::orange);
    __layer3.init(LayerId::L3,QColorConstants::Svg::dodgerblue);
}

void MapEditor::init()
{
    db              = new MapEditorDb(this);
    selector        = new MapEditorSelection(this);

    Sys::mapEditorView->init(this,selector);

    config          = Sys::config;
    mosaicMaker     = Sys::mosaicMaker;
    motifMaker      = Sys::motifMaker;
    tilingMaker     = Sys::tilingMaker;

    unload();

    MapEditorStash::restore(db);

    connect(this, &MapEditor::sig_updateView, Sys::systemViewController, &ViewController::slot_updateView);
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

    getLayer(LayerId::L1).reset();
    getLayer(LayerId::L2).reset();
    getLayer(LayerId::L3).reset();
    getLayer(LayerId::Composite).reset();

    setViewSelect(LayerId::None,true);
    setEditSelect(LayerId::L1);
    setMapedMouseMode(MouseMode::None);
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

    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }

    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(map,style);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);
    }

    forceRedraw();

    return true;
}

bool  MapEditor::loadFromfPrototypeMaker()
{
    auto protos = motifMaker->protoData.getPrototypes();
    if (protos.isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found");
        box.exec();
        return false;
     }

    bool rv = false;

    for (auto & proto : protos)
    {
        if (!proto->hasContent())
        {
            QMessageBox box(Sys::controlPanel);
            box.setIcon(QMessageBox::Warning);
            box.setText("Prototype has no content");
            box.exec();
            continue;
        }

        MapPtr map = proto->getProtoMap();

        LayerId lindex = obtainEmptyLayer();
        if (lindex == LayerId::None)
        {
            QMessageBox box(Sys::controlPanel);
            box.setIcon(QMessageBox::Warning);
            box.setText("No empty layer for map");
            box.exec();
            continue;
        }

        MapEditorLayer & layer = getLayer(lindex);
        layer.setLayer(map,proto);
        setEditSelect(lindex);
        setViewSelect(lindex,true);

        if (config->mapEditorMode == EditorMode::DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
        rv = true;
    }
    forceRedraw();

    return rv;
}

bool  MapEditor::loadSelectedMotifs()
{
    ProtoPtr proto = motifMaker->motifData.getActivePrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Prototype Maker");
        box.exec();
        return false;
    }

    TileMotifSet timos = motifMaker->motifData.getVisibleTileMotifs();
    if (timos.size() == 0)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tile motifs found for prototype");
        box.exec();
        return false;
    }

    TilingPtr tiling = proto->getTiling();

    MapPtr layerMap = make_shared<Map>("Motifs");

    for (const auto & timo : std::as_const(timos))
    {
        auto motif = timo->getMotif();
        if (motif)
        {
            MapPtr map = motif->getExistingMotifMap();
            if (map)
            {
                TilePtr tile         = timo->getTile();
                QTransform placement = tiling->getFirstPlacement(tile);
                MapPtr tmap          = map->getTransformed(placement);
                layerMap->mergeMap(tmap);
            }
        }
    }

    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }

    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(layerMap,proto,timos,MapType::Motif);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(layerMap))
        {
            createLocalDCEL(layerMap);
        }
    }

    forceRedraw();
    return true;
}

bool  MapEditor::loadMotifUnit()
{
    ProtoPtr proto = motifMaker->motifData.getActivePrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Prototype Maker");
        box.exec();
        return false;
    }

    TileMotifSet timos = motifMaker->motifData.getTileMotifs();
    if (timos.size() == 0)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tile motifs found for prototype");
        box.exec();
        return false;
    }

    TilingPtr tiling = proto->getTiling();

    MapPtr unitMap = make_shared<Map>("Motif Unit Map");

    for (const auto & timo : std::as_const(timos))
    {
        auto motif = timo->getMotif();
        if (motif)
        {
            MapPtr map = motif->getExistingMotifMap();
            if (map)
            {
                TilePtr tile    = timo->getTile();
                auto placements = tiling->unit().getPlacements(tile);
                for (QTransform & placement : placements)
                {
                    MapPtr tmap = map->getTransformed(placement);
                    unitMap->mergeMap(tmap);
                }
            }
        }
    }

    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }
    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(unitMap,proto,timos,MapType::MotifUnit);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(unitMap))
        {
            createLocalDCEL(unitMap);
        }
    }

    forceRedraw();
    return true;
}

bool MapEditor::loadTilingUnit()
{
    TilingPtr tp = tilingMaker->getSelectedTiling();

    if (!tp || tp->isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return false;
    }

    MapPtr map = tp->createMapSingle();

    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }

    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(map,tp,MapType::TilingUnit);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);
    }

    forceRedraw();

    return true;
}

bool MapEditor::loadTilingRepeated()
{
    TilingPtr tp = tilingMaker->getSelectedTiling();
    if (!tp || tp->isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return false;
    }

    MapPtr map = tp->createMapFullSimple();

    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }

    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(map,tp,MapType::TilingFull);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);
    }

    forceRedraw();

    return true;
}

bool MapEditor::loadFromMap(MapPtr map, MapEditorMap::MapType mtype)
{
    LayerId lindex = obtainEmptyLayer();
    if (lindex == LayerId::None)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No empty layer for map");
        box.exec();
        return false;
    }

    MapEditorLayer & layer = getLayer(lindex);
    layer.setLayer(map,mtype);
    setEditSelect(lindex);
    setViewSelect(lindex,true);

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);
    }
    emit Sys::mapEditorView->sig_reconstructView(); // because it can load a background

    return true;
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

void MapEditor::updateMosiacWithPrototype(ProtoPtr proto, MosaicMaker::Event event, ProtoPtr oldProto)
{
    MosaicEvent mevent;
    mevent.prototype = proto;
    mevent.event     = event;
    if (oldProto)
        mevent.oldPrototypes = { oldProto };
    mosaicMaker->sm_takeUp(mevent);

    // MotifMaker only holds weak refs to prototypes and normally stays in sync because
    // it's the one driving prototype changes (it updates itself, then calls down to
    // MosaicMaker) - this path goes straight to MosaicMaker instead, so resync explicitly
    // or MotifMaker's active prototype is left as a dangling weak pointer.
    QVector<ProtoPtr> protos = mosaicMaker->getMosaic()->getPrototypes();
    if (!protos.isEmpty())
        motifMaker->sm_takeDown(protos);

    emit Sys::mapEditorView->sig_reconstructView(); // triggers createSyleRepresentation

    // does not switch view to mosaic
    qDebug().noquote() << "MapEditor::pushToMosaic - end";
}

////////////////////////////////////////////////////////////////
//
// Pushes
//
////////////////////////////////////////////////////////////////

bool MapEditor::pushToMosaic(MapEditorLayer & layer, bool convertToMap)
{
    qDebug().noquote() << "MapEditor::pushToMosaic" << Transform::info(Sys::mapEditorView->getLayerTransform());

    if (layer.getLayerMapType() == MapType::Prototype)
    {
        MapedMapPtr mmp     = layer.getWorking();
        ProtoMap * protoMap = static_cast<ProtoMap*>(mmp.get());

        ProtoPtr oldProto = protoMap->getPrototype();

        if (convertToMap)
        {
            // irreversible: discards the tiling and tile motifs, the map becomes the whole prototype
            ProtoPtr newProto = make_shared<Prototype>(protoMap->getMap());
            updateMosiacWithPrototype(newProto,MosaicMaker::Event::ReloadProtoMulti,oldProto);
            return true;
        }

        MosaicMaker::Event event;
        if (config->mapedLoadCopies)
            event = MosaicMaker::Event::ReloadProtoSingle;   // single uses the shared ptr
        else
            event = MosaicMaker::Event::ReloadProtoMulti;    // multi sets the prototype ptr too

        oldProto->setProtoMap(protoMap->getMap());   // attach the (possibly edited) working map
        updateMosiacWithPrototype(oldProto,event,oldProto);

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
        updateMosiacWithPrototype(proto,MosaicMaker::Event::LoadProtoSingle);

        return true;
    }

    // at this point should be a new map or style map
    switch (layer.getLayerMapType())
    {
    case MapType::Style:
    case MapType::Created:
    case MapType::Composite:
        break;  // continue

    default:
        return false;
    }

    // put map into existing style prototype
    MapPtr map   = layer.getMapedLayerMap();

    if (convertToMap)
    {
        // irreversible: discards the tiling and tile motifs, the map becomes the whole prototype
        ProtoPtr oldProto = mosaic->getFirstExistingPrototype();
        ProtoPtr newProto = make_shared<Prototype>(map);
        updateMosiacWithPrototype(newProto,MosaicMaker::Event::ReloadProtoMulti,oldProto);
        return true;
    }

    ProtoPtr proto = mosaic->getFirstExistingPrototype();   // mirrors getFirstExistingPrototypeMap() above
    Q_ASSERT(proto);
    proto->setProtoMap(map);

    auto olddcel = mosaic_map->getDerivedDCEL();   // whether a DCEL existed before, on the OLD map

    if (config->mapEditorMode == EditorMode::DCEL)
    {
        if (!useExistingDCEL(map))
        {
            createLocalDCEL(map);
        }
    }
    else
    {
        if (olddcel)
        {
            auto dcel = std::make_shared<DCEL>(map.get());
            if (dcel->build())
            {
                map->setDerivedDCEL(dcel);
            }
            else
            {
                DCELPtr nulldcel;
                map->setDerivedDCEL(nulldcel);
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
    // motif in tile motif or adds a new tile motif

    qDebug().noquote() << "MapEditor::convertToMotif" << Transform::info(Sys::mapEditorView->getLayerTransform());

    MapPtr map   = layer.getMapedLayerMap();

    MapType mtype = getMapType(map);
    if (!db->isMotif(mtype) && mtype != MapType::Created)
        return false;

    TileMotifPtr timo;
    MapType layerType = layer.getLayerMapType();
    if (layerType == MapType::Motif || layerType == MapType::MotifUnit || layerType == MapType::CompositeMotif)
    {
        MapedMapPtr mmp   = layer.getWorking();
        TimoMap * timoMap = static_cast<TimoMap*>(mmp.get());
        auto origins      = timoMap->getTileMotifs();
        if (!origins.isEmpty())
            timo = origins.first();
    }
    if (!timo)
        timo = motifMaker->motifData.getDelegatedTileMotif();
    if (!timo)
        return false;

    TilingPtr tiling = timo->getTiling();
    Q_ASSERT(tiling);
    auto transform = tiling->getFirstPlacement(timo->getTile()).inverted();
    auto map2      = map->getTransformed(transform);

    auto motif = make_shared<ExplicitMapMotif>(map2);
    motif->setTile(timo->getTile());
    motif->build();     // populates motifMap - see TileMotif::createMotifFromTile for the same pattern
    timo->setMotif(motif);

    // notify motif maker - matches MotifEditor::slot_motifModified
    motifMaker->motifData.show(timo);

    MotifEvent pevent;
    pevent.event  = MotifMaker::Event::MotifChanged;
    pevent.tiling = tiling;    // the tiling timo actually belongs to, not whatever's "active" in the Motif Maker page
    motifMaker->sm_takeUp(pevent);

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

    TilingPtr tp;
    MapType layerType = layer.getLayerMapType();
    if (layerType == MapType::TilingUnit || layerType == MapType::TilingFull)
    {
        MapedMapPtr mmp     = layer.getWorking();
        TilingMap * tilingMap = static_cast<TilingMap*>(mmp.get());
        tp = tilingMap->getTiling();
    }
    if (!tp)
        tp = tilingMaker->getSelectedTiling();   // fallback for File/Created-origin maps
    TilingUnitController tilingUnit(tp.get());
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
    Sys::systemViewController->setSelectedPrimaryLayer(tp);

    const Xform & xf = Sys::mapEditorView->getModelXform();
    Sys::tilingMakerView->setModelXform(xf,true,Sys::nextSigid());

    qDebug().noquote() << "MapEditor::pushToTiling" << "- end" << Transform::info(Sys::mapEditorView->getLayerTransform());
    return true;
}

void MapEditor::setMapedMouseMode(MouseMode mode)
{
    db->setMouseMode(mode);

    db->resetMouseInteraction();      // only needed by border edit

    switch (mode)
    {
    default:
    case MouseMode::None:
    case MouseMode::DrawLine:
    case MouseMode::Delete:
    case MouseMode::SplitLine:
        Sys::controlPanel->restorePageStatus();
        break;

    case MouseMode::ExtendP1:
    case MouseMode::ExtendP2:
        Sys::controlPanel->overridePagelStatus("Click on edge and drag to extend. Press F to flip direction of extension");
        break;

    case MouseMode::CreateLine:
        Sys::controlPanel->overridePagelStatus("Click on map edge to  draw two lines at center");
        break;

    case MouseMode::ConstructionLines:
        db->showConstructionLines = true;
        Sys::controlPanel->restorePageStatus();
        break;

    case MouseMode::ConstructionCircles:
        Sys::controlPanel->overridePagelStatus("Right-click: create circle | Left-click-inside: drag to move circle  | Left-click-edge: resize circle");
        db->showConstructionLines = true;
        break;
    }

    forceRedraw();
}

MapEditorLayer &  MapEditor::getLayer(LayerId layer)
{
    switch (layer)
    {
    case LayerId::L1:
        return __layer1;

    case LayerId::L2:
        return __layer2;

    case LayerId::L3:
        return __layer3;

    case LayerId::Composite:
        return __compositeLayer;

    default:
        break;
    }

    Q_ASSERT(false);            // this would be a dumb call
    return __compositeLayer;    // makes dumber compiler happy
}

MapEditorLayer & MapEditor::getEditLayer()
{
    // there is always a single edit layer
    return getLayer(getEditSelect());
}

MapEditorLayer &  MapEditor::getLayer(MapPtr map)
{
    MapEditorLayer & layer = getLayer(LayerId::L1);
    if (layer.getMapedLayerMap() == map)
    {
        return __layer1;
    }

    MapEditorLayer & layer2 = getLayer(LayerId::L2);
    if (layer2.getMapedLayerMap() == map)
    {
        return __layer2;
    }

    MapEditorLayer & layer3 = getLayer(LayerId::L3);
    if (layer3.getMapedLayerMap() == map)
    {
        return __layer3;
    }
    else
    {
        return __compositeLayer;
    }
}


MapPtr MapEditor::getEditMap()
{
    return getEditLayer().getMapedLayerMap();
}

LayerId MapEditor::obtainEmptyLayer()
{
    MapEditorLayer & melayer = getLayer(LayerId::L1);
    if (melayer.getLayerMapType() == MapType::None)
    {
        return LayerId::L1;
    }

    MapEditorLayer & melayer2 = getLayer(LayerId::L2);
    if (melayer2.getLayerMapType() == MapType::None)
    {
        return LayerId::L2;
    }

    MapEditorLayer & melayer3 = getLayer(LayerId::L3);
    if (melayer3.getLayerMapType() == MapType::None)
    {
        return LayerId::L3;
    }

    return LayerId::None;
}


void MapEditor::createComposite()
{
    if (!isViewSelected(LayerId::Composite))
        return;

    Sys::systemViewController->splash_display("Creating composite");

    QVector<TileMotifPtr> timos;
    ProtoPtr              proto;

    qreal tolerance = Sys::config->mapedMergeSensitivity;

    MapPtr compositeMap = std::make_shared<Map>("Composite");

    for (const MapEditorLayer * layer : gerUsedComposableLayers())
    {
        MapType mapType = layer->getLayerMapType();
        switch (mapType)
        {
        case MapType::Motif:
        case MapType::MotifUnit:
        {
            MapedMapPtr mmp = layer->getWorking();
            TimoMap * timoMap = static_cast<TimoMap*>(mmp.get());
            timos  += timoMap->getTileMotifs();
            proto   = timoMap->getPrototype();
        }   break;

        default:
            break;
        }

        MapPtr map = layer->getMapedLayerMap();
        qDebug() << map->summary();
        compositeMap->mergeMap(map,tolerance);
        qDebug() << compositeMap->summary();
    }

    MapCleanser mc(compositeMap);
    mc.deDuplicateVertices(tolerance);

    MapEditorLayer & layer = getLayer(LayerId::Composite);
    layer.reset();
    if (timos.size() > 0)
    {
        layer.setLayer(compositeMap,proto,timos,MapType::CompositeMotif);
    }
    else
    {
        layer.setLayer(compositeMap,MapType::Composite);
    }

    Sys::systemViewController->splash_remove();
}

QVector<const MapEditorLayer* > MapEditor::gerUsedComposableLayers()
{
    QVector<const MapEditorLayer *> maps;

    if (getLayer(LayerId::L1).getLayerMapType() != MapType::None)
    {
        maps.push_back(&getLayer(LayerId::L1));
    }

    if (getLayer(LayerId::L2).getLayerMapType() != MapType::None)
    {
        maps.push_back(&getLayer(LayerId::L2));
    }

    if (getLayer(LayerId::L3).getLayerMapType() != MapType::None)
    {
        maps.push_back(&getLayer(LayerId::L3));
    }

    return maps;
}

MapPtr MapEditor::getFirstDrawMap()
{
    MapPtr mp;
    QVector<MapPtr> maps = getDrawMaps();
    if (maps.size())
    {
        mp = maps.first();
    }
    return mp;
}

QVector<MapPtr> MapEditor::getDrawMaps()
{
    QVector<MapPtr> maps;
    if (isViewSelected(LayerId::Composite))
    {
        MapEditorLayer & layerC = getLayer(LayerId::Composite);
        if (layerC.getLayerMapType() != MapType::None)
        {
            maps.push_back(layerC.getMapedLayerMap());
        }
    }
    else
    {
        if (isViewSelected(LayerId::L1))
        {
            MapEditorLayer & layer1 = getLayer(LayerId::L1);
            if (layer1.getLayerMapType() != MapType::None)
            {
                maps.push_back(layer1.getMapedLayerMap());
            }
        }
        if (isViewSelected(LayerId::L2))
        {
            MapEditorLayer & layer2 = getLayer(LayerId::L2);
            if (layer2.getLayerMapType() != MapType::None)
            {
                maps.push_back(layer2.getMapedLayerMap());
            }
        }
        if (isViewSelected(LayerId::L3))
        {
            MapEditorLayer & layer3 = getLayer(LayerId::L3);
            if (layer3.getLayerMapType() != MapType::None)
            {
                maps.push_back(layer3.getMapedLayerMap());
            }
        }
    }
    return maps;
}

QVector<MapPtr> MapEditor::getMapLayerMaps()
{
    QVector<MapPtr> maps;

    if (getLayer(LayerId::L1).getLayerMapType() != MapType::None)
    {
        maps.push_back(getLayer(LayerId::L1).getMapedLayerMap());
    }
    if (getLayer(LayerId::L2).getLayerMapType() != MapType::None)
    {
        maps.push_back(getLayer(LayerId::L2).getMapedLayerMap());
    }
    if (getLayer(LayerId::L3).getLayerMapType() != MapType::None)
    {
        maps.push_back(getLayer(LayerId::L3).getMapedLayerMap());
    }

    return maps;
}

QVector<const MapEditorLayer *> MapEditor::getDrawLayers()
{
    QVector<const MapEditorLayer * > layers;
    if (isViewSelected(LayerId::Composite))
    {
        MapEditorLayer & layerC = getLayer(LayerId::Composite);
        if (layerC.getLayerMapType() != MapType::None)
        {
            layers.push_back(&layerC);
        }
    }
    else
    {
        if (isViewSelected(LayerId::L1))
        {
            MapEditorLayer & layer1 = getLayer(LayerId::L1);
            if (layer1.getLayerMapType() != MapType::None)
            {
                layers.push_back(&layer1);
            }
        }
        if (isViewSelected(LayerId::L2))
        {
            MapEditorLayer & layer2 = getLayer(LayerId::L2);
            if (layer2.getLayerMapType() != MapType::None)
            {
                layers.push_back(&layer2);
            }
        }
        if (isViewSelected(LayerId::L3))
        {
            MapEditorLayer & layer3 = getLayer(LayerId::L3);
            if (layer3.getLayerMapType() != MapType::None)
            {
                layers.push_back(&layer3);
            }
        }
    }
    return layers;
}


QVector<TimoMap *> MapEditor::getDrawTimoMaps()
{
    QVector<TimoMap * > tmaps;
    if (isViewSelected(LayerId::Composite))
    {
        MapEditorLayer & layerC = getLayer(LayerId::Composite);
        if (layerC.getLayerMapType() != MapType::CompositeMotif)
        {
            MapedMapPtr map = layerC.getWorking();
            TimoMap * tmap  = static_cast<TimoMap*>(map.get());
            tmaps.push_back(tmap);
        }
    }
    else
    {
        if (isViewSelected(LayerId::L1))
        {
            MapEditorLayer & layer = getLayer(LayerId::L1);
            MapType type           = layer.getLayerMapType();
            if (type == MapType::Motif || type == MapType::MotifUnit || type == MapType::FileMotif)
            {
                MapedMapPtr map = layer.getWorking();
                TimoMap * tmap  = static_cast<TimoMap*>(map.get());
                tmaps.push_back(tmap);
            }
        }
        if (isViewSelected(LayerId::L2))
        {
            MapEditorLayer & layer = getLayer(LayerId::L2);
            MapType type           = layer.getLayerMapType();
            if (type == MapType::Motif || type == MapType::MotifUnit || type == MapType::FileMotif)
            {
                MapedMapPtr map = layer.getWorking();
                TimoMap * tmap  = static_cast<TimoMap*>(map.get());
                tmaps.push_back(tmap);
            }
        }
        if (isViewSelected(LayerId::L3))
        {
            MapEditorLayer & layer = getLayer(LayerId::L3);
            MapType type           = layer.getLayerMapType();
            if (type == MapType::Motif || type == MapType::MotifUnit || type == MapType::FileMotif)
            {
                MapedMapPtr map = layer.getWorking();
                TimoMap * tmap  = static_cast<TimoMap*>(map.get());
                tmaps.push_back(tmap);
            }
        }
    }
    return tmaps;
}

TileMotifSet MapEditor::getEditTileMotifs()
{
    MapEditorLayer & layer = getEditLayer();
    MapType type           = layer.getLayerMapType();
    if (type == MapType::Motif || type == MapType::MotifUnit || type == MapType::FileMotif || type == MapEditor::MapType::CompositeMotif)
    {
        MapedMapPtr map = layer.getWorking();
        TimoMap * tmap  = static_cast<TimoMap*>(map.get());
        return tmap->getTileMotifs();
    }
    else
    {
        TileMotifSet timos;    //empty
        return timos;
    }
}

MapEditor::MapType MapEditor::getMapType(MapPtr map)
{
    if (map ==  getLayer(LayerId::Composite).getMapedLayerMap())
        return getLayer(LayerId::Composite).getLayerMapType();

    else if (map ==  getLayer(LayerId::L1).getMapedLayerMap())
        return getLayer(LayerId::L1).getLayerMapType();

    else if (map ==  getLayer(LayerId::L2).getMapedLayerMap())
        return getLayer(LayerId::L2).getLayerMapType();

    else if (map ==  getLayer(LayerId::L3).getMapedLayerMap())
        return getLayer(LayerId::L3).getLayerMapType();

    else
        return MapType::None;
}

MapPtr  MapEditor::getMap(LayerId mode)
{
    MapEditorLayer & layer = getLayer(mode);
    return layer.getMapedLayerMap();
}

void MapEditor::setViewSelect(LayerId layer, bool on)
{
    if (on)
    {
        if (layer == LayerId::None)
        {
            viewSelect = (int)LayerId::None;
        }
        else if (layer == LayerId::Composite)
        {
            viewSelect = (int)LayerId::Composite;
            createComposite();
            setEditSelect(LayerId::Composite);
        }
        else
        {
            viewSelect &= ~(int)LayerId::Composite;
            viewSelect |= (int)layer;
        }
    }
    else
    {
        if (layer == LayerId::Composite)
        {
            viewSelect = (int)LayerId::None;
        }
        else
        {
            viewSelect &= ~(int)layer;
        }
    }
}

MapEditor::MouseMode MapEditor::getMapedMouseMode()
{
    return db->getMouseMode();
}

QString MapEditor::getStatus()
{
    QString s("Map Editor:: ");
    s += sMapEditorMouseMode[(int)db->getMouseMode()];
    MapMouseActionPtr mma  = db->getMouseInteraction();
    if (mma)
    {
        s += "  ";
        s += mma->desc;
    }
    return s;
}

bool MapEditor::loadTemplate(VersionedFile xfile)
{
    bool rv = MapEditorStash::loadTemplate(xfile,db);
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

bool MapEditor::saveTemplate(VersionedName & vname)
{
    return MapEditorStash::saveTemplate(vname,db);
}

void MapEditor::forceRedraw()
{
    if (Sys::systemViewController->isEnabled(Layer::Type::MapEditor))
    {
        emit sig_updateView();
    }
}

///////////////////////////////////////////////////////////////////////////
///
/// Crop
///
///////////////////////////////////////////////////////////////////////////

// merges the crop into the map - called by map editor
bool MapEditor::embedCrop(MapPtr map)
{
    if (!map)
        return false;

    auto crop = db->getCrop();
    if (!crop)
        return false;

    qDebug() << "embed crop in map";

    switch (crop->getCropType())
    {
    case CROP_RECTANGLE:
        map->embedCrop(crop->getRect());
        break;

    case CROP_CIRCLE:
        map->embedCrop(crop->getCircle());
        break;

    case CROP_POLYGON:
        map->embedCrop(crop->getAPolygon().get());
        break;

    case CROP_UNDEFINED:
        qWarning() << "not implemented";
        return false;
    }

    return true;
}

// deletes everything outside of the crop rectangle - called by map editgor
bool MapEditor::cropMap(MapPtr map)
{
    if (!map)
        return false;

    CropPtr crop = db->getCrop();
    if (!crop)
        return false;

    qDebug() << "apply crop to map";

    switch (crop->getCropType())
    {
    case CROP_RECTANGLE:
        map->embedCrop(crop->getRect());
        map->cropOutside(crop->getRect());
        break;
    case CROP_CIRCLE:
        map->embedCrop(crop->getCircle());
        map->cropOutside(crop->getCircle());
        break;
    case CROP_POLYGON:
        map->embedCrop(crop->getAPolygon().get());
        map->cropOutside(crop->getAPolygon().get());
        break;
    case CROP_UNDEFINED:
        qWarning() << "Crop Not defined";
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (!Sys::systemViewController->isEnabled(Layer::Type::MapEditor))
    {
        return false;
    }

    int key = k->key();

    switch (key)
    {
    case Qt::Key_F1: Shortcuts::popup(Layer::Type::MapEditor); break;

    // modes
    case Qt::Key_Escape: setMapedMouseMode(MouseMode::None);                  return false; // propagate
    case Qt::Key_F3:     setMapedMouseMode(MouseMode::DrawLine);              break;
    case Qt::Key_F4:     setMapedMouseMode(MouseMode::ConstructionLines);     break;
    case Qt::Key_F5:     setMapedMouseMode(MouseMode::ConstructionCircles);   break;
    case Qt::Key_F6:     setMapedMouseMode(MouseMode::SplitLine);             break;
    case Qt::Key_F7:     setMapedMouseMode(MouseMode::ExtendP1);              break;
    case Qt::Key_F8:     setMapedMouseMode(MouseMode::ExtendP2);              break;
    case Qt::Key_F9:     setMapedMouseMode(MouseMode::Delete);                break;

    default:                                                                  return false;
    }

    return true;
}

const QString sMapEditorMapType[] =
    {
    E2STR(None),
    E2STR(Style),
    E2STR(Prototype),
    E2STR(Motif),
    E2STR(Motif Unit),
    E2STR(Tiling Unit),
    E2STR(Tiling Full),
    E2STR(File),
    E2STR(File Motif),
    E2STR(Created),
    E2STR(Layer::Composite),
    E2STR(Layer::Composite Motif),
    E2STR(Crop)
};

const QString sMapEditorMode[] =
    {
        E2STR(MAPED_MODE_MAP),
        E2STR(MAPED_MODE_DCEL)
};

const QString sMapEditorMouseMode[] =
    {
        "None",
        "Delete",
        "DrawLine",
        "SplitLine",
        "ExtendP1",
        "ExtendP2",
        "CreateLine",
        "ConstructionLines",
        "ConstructionCircles"
};
