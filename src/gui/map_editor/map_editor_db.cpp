#include <QDebug>
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/splash_screen.h"
#include "model/settings/configuration.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map.h"
#include "sys/tiledpatternmaker.h"

MapEditorDb::MapEditorDb()
{
    showMap               = true;
    showBoundaries        = false;
    showConstructionLines = false;
    showPoints            = true;
    showMidPoints         = false;
    showDirnPoints        = false;
    showArcCentre         = false;

    editSelect            = LAYER_1;
    viewSelect            = NO_MAP;
}

void MapEditorDb::reset()
{
    importedTilingMap.reset();
    currentTilingMap.reset();
    localDcel.reset();
    activeDcel.reset();

    tiling.reset();
    motifPrototype.reset();

    getLayer(LAYER_1).reset();
    getLayer(LAYER_2).reset();
    getLayer(LAYER_3).reset();
    getLayer(COMPOSITE).reset();

    delps.clear();
    constructionLines.clear();
    constructionCircles.clear();

    setViewSelect(NO_MAP,true);
    setEditSelect(LAYER_1);
}

MapEditorLayer &  MapEditorDb::getLayer(eMapedLayer layer)
{
    switch (layer)
    {
    case LAYER_1:
        return __layer1;

    case LAYER_2:
        return __layer2;

    case LAYER_3:
        return __layer3;

    case COMPOSITE:
        return __compositeLayer;

    default:
        break;
    }

    Q_ASSERT(false);            // this would be a dumb call
    return __compositeLayer;    // makes dumber compiler happy
}

MapEditorLayer & MapEditorDb::getEditLayer()
{
    // there is always a single edit layer
    return getLayer(getEditSelect());
}

MapPtr MapEditorDb::getEditMap()
{
    return getEditLayer().getMapedLayerMap();
}

eMapedLayer MapEditorDb::insertLayer(MapPtr map, eMapEditorMapType mtype)
{
    qDebug().noquote() << "inserting map" << map->summary() << sMapEditorMapType[mtype];

    MapEditorLayer & layer = getLayer(LAYER_1);
    if (layer.getLayerMapType() == MAPED_TYPE_UNKNOWN)
    {
        layer.set(map,mtype);
        setEditSelect(LAYER_1);
        setViewSelect(LAYER_1,true);
        return LAYER_1;
    }

    MapEditorLayer & layer2 = getLayer(LAYER_2);
    if (layer2.getLayerMapType() == MAPED_TYPE_UNKNOWN)
    {
        layer2.set(map,mtype);
        setEditSelect(LAYER_2);
        setViewSelect(LAYER_2,true);
        return LAYER_2;
    }

    MapEditorLayer & layer3 = getLayer(LAYER_3);
    if (layer3.getLayerMapType() == MAPED_TYPE_UNKNOWN)
    {
        layer3.set(map,mtype);
        setEditSelect(LAYER_3);
        setViewSelect(LAYER_3,true);
        return LAYER_3;
    }
    else
    {
        // default nothing inserted
        setEditSelect(COMPOSITE);
        setViewSelect(COMPOSITE,true);
        return COMPOSITE;
    }
}

void MapEditorDb::createComposite()
{
    if (!isViewSelected(COMPOSITE))
        return;

    Sys::splash->display("Creating composite");

    DesignElementPtr  del;

    qreal tolerance = Sys::config->mapedMergeSensitivity;

    MapPtr compositeMap = std::make_shared<Map>("Composite");

    for (const MapEditorLayer * layer : getComposableLayers())
    {
        eMapEditorMapType maptype = layer->getLayerMapType();
        if (isMotif(maptype))
        {
            del = layer->getDel();
        }
        MapPtr map = layer->getMapedLayerMap();
        qDebug() << map->summary();
        compositeMap->mergeMap(map,tolerance);
        qDebug() << compositeMap->summary();
    }

    compositeMap->deDuplicateVertices(tolerance);

    if (del)
        getLayer(COMPOSITE).set(compositeMap,MAPED_TYPE_COMPOSITE_MOTIF,del);
    else
        getLayer(COMPOSITE).set(compositeMap,MAPED_TYPE_COMPOSITE);

    Sys::splash->remove();
}

void MapEditorDb::setViewSelect(eMapedLayer layer, bool on)
{
    if (on)
    {
        if (layer == NO_MAP)
        {
            viewSelect = NO_MAP;
        }
        else if (layer == COMPOSITE)
        {
            viewSelect = COMPOSITE;
            createComposite();
            setEditSelect(COMPOSITE);
        }
        else
        {
            viewSelect &= ~COMPOSITE;
            viewSelect |= layer;
        }
    }
    else
    {
        if (layer == COMPOSITE)
        {
            viewSelect = NO_MAP;
        }
        else
        {
            viewSelect &= ~layer;
        }
    }
}

MapPtr MapEditorDb::getFirstDrawMap()
{
    MapPtr mp;
    QVector<MapPtr> maps = getDrawMaps();
    if (maps.size())
    {
        mp = maps.first();
    }
    return mp;
}

QVector<MapPtr>  MapEditorDb::getDrawMaps()
{
    QVector<MapPtr> maps;
    if (isViewSelected(COMPOSITE))
    {
        MapEditorLayer & layerC = getLayer(COMPOSITE);
        if (layerC.getLayerMapType() != MAPED_TYPE_UNKNOWN)
        {
            maps.push_back(layerC.getMapedLayerMap());
        }
    }
    else
    {
        if (isViewSelected(LAYER_1))
        {
            MapEditorLayer & layer1 = getLayer(LAYER_1);
            if (layer1.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(layer1.getMapedLayerMap());
            }
        }
        if (isViewSelected(LAYER_2))
        {
            MapEditorLayer & layer2 = getLayer(LAYER_2);
            if (layer2.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(layer2.getMapedLayerMap());
            }
        }
        if (isViewSelected(LAYER_3))
        {
            MapEditorLayer & layer3 = getLayer(LAYER_3);
            if (layer3.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(layer3.getMapedLayerMap());
            }
        }
    }
    return maps;
}

QVector<MapPtr> MapEditorDb::getMapLayerMaps()
{
    QVector<MapPtr> maps;

    if (getLayer(LAYER_1).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(getLayer(LAYER_1).getMapedLayerMap());
    }
    if (getLayer(LAYER_2).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(getLayer(LAYER_2).getMapedLayerMap());
    }
    if (getLayer(LAYER_3).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(getLayer(LAYER_3).getMapedLayerMap());
    }

    return maps;
}

QVector<const MapEditorLayer *> MapEditorDb::getDrawLayers()
{
    QVector<const MapEditorLayer * > layers;
    if (isViewSelected(COMPOSITE))
    {
        MapEditorLayer & layerC = getLayer(COMPOSITE);
        if (layerC.getLayerMapType() != MAPED_TYPE_UNKNOWN)
        {
            layers.push_back(&layerC);
        }
    }
    else
    {
        if (isViewSelected(LAYER_1))
        {
            MapEditorLayer & layer1 = getLayer(LAYER_1);
            if (layer1.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                layers.push_back(&layer1);
            }
        }
        if (isViewSelected(LAYER_2))
        {
            MapEditorLayer & layer2 = getLayer(LAYER_2);
            if (layer2.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                layers.push_back(&layer2);
            }
        }
        if (isViewSelected(LAYER_3))
        {
            MapEditorLayer & layer3 = getLayer(LAYER_3);
            if (layer3.getLayerMapType() != MAPED_TYPE_UNKNOWN)
            {
                layers.push_back(&layer3);
            }
        }
    }
    return layers;
}

QVector<const MapEditorLayer* > MapEditorDb::getComposableLayers()
{
    QVector<const MapEditorLayer *> maps;

    if (getLayer(LAYER_1).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(&getLayer(LAYER_1));
    }

    if (getLayer(LAYER_2).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(&getLayer(LAYER_2));
    }

    if (getLayer(LAYER_3).getLayerMapType() != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(&getLayer(LAYER_3));
    }

    return maps;
}

eMapEditorMapType MapEditorDb::getMapType(MapPtr map)
{
    if (map ==  getLayer(COMPOSITE).getMapedLayerMap())
        return getLayer(COMPOSITE).getLayerMapType();

    else if (map ==  getLayer(LAYER_1).getMapedLayerMap())
        return getLayer(LAYER_1).getLayerMapType();

    else if (map ==  getLayer(LAYER_2).getMapedLayerMap())
        return getLayer(LAYER_2).getLayerMapType();

    else if (map ==  getLayer(LAYER_3).getMapedLayerMap())
        return getLayer(LAYER_3).getLayerMapType();

    else
        return MAPED_TYPE_UNKNOWN;
}

MapPtr  MapEditorDb::getMap(eMapedLayer mode)
{
    MapEditorLayer & layer = getLayer(mode);
    return layer.getMapedLayerMap();
}

bool MapEditorDb::isMotif(eMapEditorMapType type)
{
    switch (type)
    {
    case MAPED_LOADED_FROM_MOTIF:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_COMPOSITE_MOTIF:
    case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
        return true;

    default:
        return false;
    }
}

// merges the crop into the map - called by map editor
bool MapEditorDb::embedCrop(MapPtr map)
{
    if (!map)
        return false;

    auto crop = getCrop();
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
bool MapEditorDb::cropMap(MapPtr map)
{
    if (!map)
        return false;

    CropPtr crop = getCrop();
    if (!crop)
        return false;

    qDebug() << "apply crop to map";

    switch (crop->getCropType())
    {
    case CROP_RECTANGLE:
        map->cropOutside(crop->getRect());
        break;
    case CROP_CIRCLE:
        map->cropOutside(crop->getCircle());
        break;
    case CROP_POLYGON:
        map->cropOutside(crop->getAPolygon().get());
        break;
    case CROP_UNDEFINED:
        qWarning() << "Crop Not defined";
        return false;
    }

    return true;
}

void MapEditorDb::setCrop(CropPtr acrop)
{
    // this local crop is unique
    Crop crop = *acrop.get();
    _crop = std::make_shared<Crop>(crop);
}

bool MapEditorDb::hasConstructionLines()
{
    return ( constructionLines.size()|| constructionCircles.size() );
}


//////////////////////////////////////////////////////////////////////
///
///  MapEditor Layer
///
//////////////////////////////////////////////////////////////////////

MapEditorLayer::MapEditorLayer()
{
    reset();
}

void MapEditorLayer::set(MapPtr map, eMapEditorMapType type)
{
    qDebug() << "Inserting map" << map->summary();
    this->map = map;
    mtype = type;
    wdel.reset();
}

void MapEditorLayer::set(MapPtr map, eMapEditorMapType type, WeakDELPtr wdel)
{
    qDebug() << "Inserting map" << map->summary();
    this->map = map;
    mtype = type;
    this->wdel = wdel;
}

void MapEditorLayer::reset()
{
    mtype = MAPED_TYPE_UNKNOWN;
    map.reset();
    wdel.reset();
}
