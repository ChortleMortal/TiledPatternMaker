#include <QDebug>
#include "makers/map_editor/map_editor_db.h"
#include "geometry/map.h"
#include "panels/panel.h"
#include "settings/configuration.h"

MapEditorDb::MapEditorDb()
{
    showMap               = true;
    showBoundaries        = false;
    showConstructionLines = false;
    showPoints            = true;
    showMidPoints         = false;
    showDirnPoints        = false;
    showArcCentre         = false;

    stash = new MapEditorStash(this);
}

void MapEditorDb::reset()
{
    createdTilingMap.reset();
    currentTilingMap.reset();
    localDcel.reset();
    activeDcel.reset();

    tiling.reset();
    motifPrototype.reset();
    clearLayers();

    delps.clear();
    constructionLines.clear();
    constructionCircles.clear();

    setViewSelect(NO_MAP,true);
    setEditSelect(NO_MAP);
}

bool MapEditorDb::insertLayer(MapEditorLayer mapLayer)
{
    bool rv;
    if (_layer1.type == MAPED_TYPE_UNKNOWN)
    {
        _layer1 = mapLayer;
        setEditSelect(LAYER_1);
        setViewSelect(LAYER_1,true);
        rv = true;
    }
    else if (_layer2.type == MAPED_TYPE_UNKNOWN)
    {
        _layer2 = mapLayer;
        setEditSelect(LAYER_2);
        setViewSelect(LAYER_2,true);
        rv = true;
    }
    else if (_layer3.type == MAPED_TYPE_UNKNOWN)
    {
        _layer3 = mapLayer;
        setEditSelect(LAYER_3);
        setViewSelect(LAYER_3,true);
        rv = true;
    }
    else
    {
        setEditSelect(COMPOSITE);
        setViewSelect(COMPOSITE,true);
        rv = false;
    }
    return rv;
}

void MapEditorDb::clearLayers()
{
    _compositeLayer.reset();
    _layer1.reset();
    _layer2.reset();
    _layer3.reset();
}

void MapEditorDb::replaceLayer(eLayer layer,MapEditorLayer mapLayer)
{
    switch (layer)
    {
    case COMPOSITE:
        _compositeLayer = mapLayer;
        break;
    case LAYER_1:
        _layer1 = mapLayer;
        break;
    case LAYER_2:
        _layer2 = mapLayer;
        break;
    case LAYER_3:
        _layer3 = mapLayer;
        break;

    case NO_MAP:
        break;
    }
}

void MapEditorDb::createComposite()
{
    if (!isViewSelected(COMPOSITE))
        return;

    ControlPanel::getInstance()->splashTiling("Creating composite");

    eMapEditorMapType mtype = MAPED_TYPE_UNKNOWN;
    WeakDELPtr wdelp;

    qreal tolerance = Configuration::getInstance()->mapedMergeSensitivity;

    MapPtr compositeMap = std::make_shared<Map>("Composite map");

    for (auto & layer : getComposableLayers())
    {
        if (isMotif(layer.type))
        {
            mtype = MAPED_TYPE_COMPOSITE_MOTIF;
            wdelp = layer.wdel;
        }
        else
        {
            mtype = MAPED_TYPE_COMPOSITE;
        }
        MapPtr map = layer.getMap();
        qDebug() << map->namedSummary();
        compositeMap->mergeMap(map,tolerance);
        qDebug() << compositeMap->namedSummary();
    }

    compositeMap->deDuplicateVertices(tolerance);

    //compositeMap->buildNeighbours();

    replaceLayer(COMPOSITE, MapEditorLayer(compositeMap,mtype,wdelp));

    ControlPanel::getInstance()->removeSplashTiling();
}

void MapEditorDb::setViewSelect(eLayer layer, bool on)
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

MapPtr MapEditorDb::getEditMap()
{
    MapPtr m;
    switch (getEditSelect())
    {
    case COMPOSITE:
        m = _compositeLayer.getMap();
        break;
    case LAYER_1:
        if (_layer1.type != MAPED_TYPE_UNKNOWN)
            m = _layer1.getMap();
        break;
    case LAYER_2:
        if (_layer2.type != MAPED_TYPE_UNKNOWN)
            m = _layer2.getMap();
        break;
    case LAYER_3:
        if (_layer3.type != MAPED_TYPE_UNKNOWN)
            m = _layer3.getMap();
        break;

    case NO_MAP:
        break;
     }
    return m;
}

MapEditorLayer MapEditorDb::getEditLayer()
{
    MapEditorLayer m;
    switch (getEditSelect())
    {
    case COMPOSITE:
        m = _compositeLayer;
        break;
    case LAYER_1:
        m = _layer1;
        break;
    case LAYER_2:
        m = _layer2;
        break;
    case LAYER_3:
        m = _layer3;
        break;
    case NO_MAP:
        break;
    }
    return m;
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
        if (_compositeLayer.type != MAPED_TYPE_UNKNOWN)
        {
            maps.push_back(_compositeLayer.getMap());
        }
    }
    else
    {
        if (isViewSelected(LAYER_1))
        {
            if (_layer1.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer1.getMap());
            }
        }
        if (isViewSelected(LAYER_2))
        {
            if (_layer2.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer2.getMap());
            }
        }
        if (isViewSelected(LAYER_3))
        {
            if (_layer3.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer3.getMap());
            }
        }
    }
    return maps;
}

QVector<MapPtr> MapEditorDb::getMapLayerMaps()
{
    QVector<MapPtr> maps;

    if (_layer1.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer1.getMap());
    }
    if (_layer2.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer2.getMap());
    }
    if (_layer3.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer3.getMap());
    }

    return maps;
}

QVector<MapEditorLayer> MapEditorDb::getDrawLayers()
{
    QVector<MapEditorLayer> maps;
    if (isViewSelected(COMPOSITE))
    {
        if (_compositeLayer.type != MAPED_TYPE_UNKNOWN)
        {
            maps.push_back(_compositeLayer);
        }
    }
    else
    {
        if (isViewSelected(LAYER_1))
        {
            if (_layer1.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer1);
            }
        }
        if (isViewSelected(LAYER_2))
        {
            if (_layer2.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer2);
            }
        }
        if (isViewSelected(LAYER_3))
        {
            if (_layer3.type != MAPED_TYPE_UNKNOWN)
            {
                maps.push_back(_layer3);
            }
        }
    }
    return maps;
}

QVector<MapEditorLayer> MapEditorDb::getComposableLayers()
{
    QVector<MapEditorLayer> maps;
    if (_layer1.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer1);
    }
    if (_layer2.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer2);
    }
    if (_layer3.type != MAPED_TYPE_UNKNOWN)
    {
        maps.push_back(_layer3);
    }
    return maps;
}

eMapEditorMapType MapEditorDb::getMapType(MapPtr map)
{
    if (map == _compositeLayer.getMap())
        return _compositeLayer.type;
    if (map == _layer1.getMap())
        return _layer1.type;
    if (map == _layer2.getMap())
        return _layer2.type;
    if (map == _layer3.getMap())
        return _layer3.type;
    return MAPED_TYPE_UNKNOWN;
}

MapPtr  MapEditorDb::getMap(eLayer mode)
{
    MapPtr m;
    switch (mode)
    {
    case COMPOSITE:
        if (_compositeLayer.type != MAPED_TYPE_UNKNOWN)
            m = _compositeLayer.getMap();
        break;
    case LAYER_1:
        if (_layer1.type != MAPED_TYPE_UNKNOWN)
            m = _layer1.getMap();
        break;
    case LAYER_2:
        if (_layer2.type != MAPED_TYPE_UNKNOWN)
            m = _layer2.getMap();
        break;
    case LAYER_3:
        if (_layer3.type != MAPED_TYPE_UNKNOWN)
            m = _layer3.getMap();
        break;
    case NO_MAP:
        break;
    }
    return m;
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
