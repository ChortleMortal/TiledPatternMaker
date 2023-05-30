#pragma once
#ifndef MAPEDITORDB_H
#define MAPEDITORDB_H

#include "enums/emapeditor.h"
#include "misc/unique_qvector.h"
#include "makers/map_editor/map_editor_mouseactions.h"
#include "makers/map_editor/map_editor_stash.h"
#include "makers/crop_maker/crop_maker.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Style>            StylePtr;

typedef std::weak_ptr<class Mosaic>             WeakMosaicPtr;
typedef std::weak_ptr<class Style>              WeakStylePtr;
typedef std::weak_ptr<class Border>             WeakBorderPtr;
typedef std::weak_ptr<class DCEL>               WeakDCELPtr;
typedef std::weak_ptr<class Crop>               WeakCropPtr;
typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class DesignElement>      WeakDELPtr;
typedef std::weak_ptr<class Tiling>             WeakTilingPtr;
typedef std::weak_ptr<class Map>                WeakMapPtr;

class MapEditorLayer
{
public:
    MapEditorLayer();
    MapEditorLayer(MapPtr map, eMapEditorMapType type);
    MapEditorLayer(MapPtr map, eMapEditorMapType type, WeakDELPtr wdel);

    void reset();

    MapPtr getMapedLayerMap() { return map; }

    eMapEditorMapType type;
    WeakDELPtr        wdel;

private:
    MapPtr            map;
};

class MapEditorDb  : public CropMaker
{
public:
    MapEditorDb();

    void                reset();

    void                setMouseMode(eMapEditorMouseMode emm) { mouseMode = emm; }
    eMapEditorMouseMode getMouseMode() { return mouseMode;}

    void                setMouseInteraction(MapMouseActionPtr action) { mouse_interaction = action; }
    void                resetMouseInteraction() { mouse_interaction.reset(); }
    MapMouseActionPtr   getMouseInteraction() { return mouse_interaction; }

    bool                insertLayer(MapEditorLayer mapLayer);
    void                replaceLayer(eLayer layer,MapEditorLayer mapLayer);
    void                clearLayers();
    void                createComposite();

    void                setViewSelect(eLayer layer, bool on);
    void                setEditSelect(eLayer layer)  { editSelect = layer; }
    bool                isViewSelected(eLayer layer) { return viewSelect & layer; }
    eLayer              getEditSelect()              { return editSelect; }

    void                setTiling(TilingPtr tp) { tiling = tp; }
    TilingPtr           getTiling()         { return tiling.lock(); }

    void                setMotfiPrototype(ProtoPtr pp) { motifPrototype = pp; }
    ProtoPtr            getMotifPrototype() { return motifPrototype.lock(); }

    QVector<WeakDELPtr> & getDesignElements() { return delps; }

    void                setActiveDCEL(DCELPtr dcel) { activeDcel = dcel; localDcel.reset();}
    DCELPtr             getActiveDCEL()             { return activeDcel.lock(); }
    void                setLocalDCEL(DCELPtr dcel)  { activeDcel = dcel; localDcel = dcel; }
    DCELPtr             getLocaldDCEL()             { return localDcel; }

    MapPtr              getEditMap();
    MapEditorLayer      getEditLayer();

    MapPtr              getMap(eLayer mode);

    MapPtr              getFirstDrawMap();
    QVector<MapPtr>     getDrawMaps();
    QVector<MapEditorLayer>   getDrawLayers();
    QVector<MapEditorLayer>   getComposableLayers();

    QVector<MapPtr>     getMapLayerMaps();    // all layers

    eMapEditorMapType   getMapType(MapPtr map);
    static bool         isMotif(eMapEditorMapType type);

    MapEditorStash *    getStash() { return stash; }

    bool                showMap;
    bool                showBoundaries;
    bool                showConstructionLines;
    bool                showPoints;
    bool                showMidPoints;
    bool                showDirnPoints;
    bool                showArcCentre;

    UniqueQVector<QLineF> constructionLines;
    UniqueQVector<Circle> constructionCircles;

    MapPtr              createdTilingMap;
    MapPtr              currentTilingMap;

    void                setCrop(CropPtr acrop) override;
    CropPtr             getCrop()              override { return _crop; }
    void                removeCrop()           override { _crop.reset(); }
    bool                embedCrop(MapPtr map);
    bool                cropMap(MapPtr map);

protected:
    eMapEditorMouseMode  mouseMode; // Mouse mode, triggered by the toolbar.

    QVector<WeakDELPtr> delps;
    WeakTilingPtr       tiling;
    WeakProtoPtr        motifPrototype;

    WeakDCELPtr         activeDcel;
    DCELPtr             localDcel;

    MapMouseActionPtr   mouse_interaction;    // used by menu

private:
    MapEditorStash    *  stash;                // stash of construction lines

    int                 viewSelect;
    eLayer              editSelect;
    MapEditorLayer      _compositeLayer;
    MapEditorLayer      _layer1;
    MapEditorLayer      _layer2;
    MapEditorLayer      _layer3;

    CropPtr              _crop;
};

#endif // MAPEDITORDB_H
