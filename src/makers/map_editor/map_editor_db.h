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
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;
typedef std::shared_ptr<class Circle>           CirclePtr;

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

    void set(MapPtr map, eMapEditorMapType type);
    void set(MapPtr map, eMapEditorMapType type, WeakDELPtr wdel);
    void reset();

    void setDel(DesignElementPtr del)   { wdel = del; }
    DesignElementPtr  getDel() const    { return wdel.lock(); }

    MapPtr getMapedLayerMap() const     { return map; }
    eMapEditorMapType getLayerMapType() const { return mtype; }


private:
    WeakDELPtr        wdel;
    eMapEditorMapType mtype;
    MapPtr            map;
};

class MapEditorDb  : public MosaicCropMaker
{
public:
    MapEditorDb();
    virtual ~MapEditorDb() {}

    void                reset();

    MapEditorLayer &    getLayer(eMapedLayer layer);
    MapEditorLayer &    getEditLayer();

    MapPtr              getMap(eMapedLayer mode);
    eMapEditorMapType   getMapType(MapPtr map);
    MapPtr              getEditMap();

    eMapedLayer         insertLayer(MapPtr map, eMapEditorMapType mtype);
    void                createComposite();

    void                setViewSelect(eMapedLayer layer, bool on);
    void                setEditSelect(eMapedLayer layer) { editSelect = layer; }
    bool                isViewSelected(eMapedLayer layer){ return viewSelect & layer; }
    eMapedLayer         getEditSelect()             { return editSelect; }

    void                setTiling(TilingPtr tp)     { tiling = tp; }
    TilingPtr           getTiling()                 { return tiling.lock(); }

    void                setMotfiPrototype(ProtoPtr pp) { motifPrototype = pp; }
    ProtoPtr            getMotifPrototype()         { return motifPrototype.lock(); }

    QVector<WeakDELPtr> & getDesignElements()       { return delps; }

    void                setActiveDCEL(DCELPtr dcel) { activeDcel = dcel; localDcel.reset();}
    DCELPtr             getActiveDCEL()             { return activeDcel.lock(); }
    void                setLocalDCEL(DCELPtr dcel)  { activeDcel = dcel; localDcel = dcel; }
    DCELPtr             getLocaldDCEL()             { return localDcel; }

    BkgdImagePtr        getBackgroundImage()        { return bimage; }
    void                setBackgroundImage(BkgdImagePtr bip) { bimage = bip; }

    MapPtr              getFirstDrawMap();
    QVector<MapPtr>     getDrawMaps();
    QVector<const MapEditorLayer *> getDrawLayers();
    QVector<const MapEditorLayer *> getComposableLayers();

    QVector<MapPtr>     getMapLayerMaps();    // all layers

    static bool         isMotif(eMapEditorMapType type);

    MapEditorStash *    getStash() { return stash; }

    void                setMouseMode(eMapEditorMouseMode emm) { mouseMode = emm; }
    eMapEditorMouseMode getMouseMode()              { return mouseMode;}

    void                setMouseInteraction(MapMouseActionPtr action) { mouse_interaction = action; }
    void                resetMouseInteraction()     { mouse_interaction.reset(); }
    MapMouseActionPtr   getMouseInteraction()       { return mouse_interaction; }

    UniqueQVector<QLineF>    constructionLines;
    UniqueQVector<CirclePtr> constructionCircles;

    MapPtr              importedTilingMap;
    MapPtr              currentTilingMap;

    void                setCrop(CropPtr acrop) override;
    CropPtr             getCrop()              override { return _crop; }
    void                removeCrop()           override { _crop.reset(); }
    bool                embedCrop(MapPtr map);
    bool                cropMap(MapPtr map);

    bool                showMap;
    bool                showBoundaries;
    bool                showConstructionLines;
    bool                showPoints;
    bool                showMidPoints;
    bool                showDirnPoints;
    bool                showArcCentre;

protected:
    eMapEditorMouseMode  mouseMode; // Mouse mode, triggered by the toolbar.

    QVector<WeakDELPtr> delps;
    WeakTilingPtr       tiling;
    WeakProtoPtr        motifPrototype;

    WeakDCELPtr         activeDcel;
    DCELPtr             localDcel;

    BkgdImagePtr        bimage;
    MapMouseActionPtr   mouse_interaction;    // used by menu

private:
    MapEditorStash* stash;                // stash of construction lines

    int             viewSelect;         // OE'd bits
    eMapedLayer     editSelect;
    MapEditorLayer  __compositeLayer;
    MapEditorLayer  __layer1;
    MapEditorLayer  __layer2;
    MapEditorLayer  __layer3;
    CropPtr         _crop;

};

#endif // MAPEDITORDB_H
