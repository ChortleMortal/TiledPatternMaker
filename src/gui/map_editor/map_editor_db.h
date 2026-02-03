#pragma once
#ifndef MAPEDITORDB_H
#define MAPEDITORDB_H

#include "sys/enums/emapeditor.h"
#include "sys/qt/unique_qvector.h"
#include "gui/map_editor/map_editor_mouseactions.h"
#include "gui/map_editor/map_editor_stash.h"
#include "model/makers/crop_maker.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class DesignElement>    DELPtr;
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


// Every Map editor layer has source information and a current map ptr
class MapEditorLayer
{
public:
    MapEditorLayer();

    void init(eMapedLayer id)  { this->id = id; }
    void setType(eMapEditorMapType mtype) { this->mtype = mtype; }

    void setLayer(MapPtr source, StylePtr style);
    void setLayer(MapPtr source, ProtoPtr proto);
    void setLayer(MapPtr source, ProtoPtr proto, DELPtr del, MotifPtr motif);
    void setLayer(MapPtr source, TilingPtr tiling, eMapEditorMapType mtype);
    void setLayer(MapPtr source, eMapEditorMapType mtype);

    void reset();

    MapPtr            getMapedLayerMap() const { return map; }
    eMapEditorMapType getLayerMapType()  const { return mtype; }

    DELPtr            getDel()           const { return del; }
    ProtoPtr          getProto()         const { return proto; }
    MotifPtr          getMotif()         const { return motif; }
    TilingPtr         getTiling()        const { return tiling; }
    eMapedLayer       getId()            const { return id; }

protected:

private:
   // TODO - make all these weak pointers (except map)
    eMapEditorMapType mtype;
    MapPtr            map;
    MapPtr            source;

    StylePtr          style;
    ProtoPtr          proto;
    DELPtr            del;
    MotifPtr          motif;    // maybe not needed (is pasrt of del)
    TilingPtr         tiling;

    eMapedLayer       id;
};

class MapEditorDb  : public MosaicCropMaker
{
public:
    MapEditorDb();
    virtual ~MapEditorDb() {}

    void                reset();

    MapEditorLayer &    getLayer(eMapedLayer layer);
    MapEditorLayer &    getLayer(MapPtr map);
    MapEditorLayer &    getEditLayer();

    MapPtr              getMap(eMapedLayer mode);
    eMapEditorMapType   getMapType(MapPtr map);
    MapPtr              getEditMap();

    eMapedLayer         obtainEmptyLayer();
    void                createComposite();

    void                setViewSelect(eMapedLayer layer, bool on);
    void                setEditSelect(eMapedLayer layer) { editSelect = layer; }
    bool                isViewSelected(eMapedLayer layer){ return viewSelect & layer; }
    eMapedLayer         getEditSelect()             { return editSelect; }

    QVector<WeakDELPtr> & getDesignElements()       { return delps; }

    void                setActiveDCEL(DCELPtr dcel) { activeDcel = dcel; localDcel.reset();}
    DCELPtr             getActiveDCEL()             { return activeDcel.lock(); }
    void                setLocalDCEL(DCELPtr dcel)  { activeDcel = dcel; localDcel = dcel; }
    DCELPtr             getLocaldDCEL()             { return localDcel; }

    BkgdImagePtr        getBackgroundImage()        { return bimage; }
    void                setBackgroundImage(BkgdImagePtr bip) { bimage = bip; }
    void                removeBackgroundImage()     { bimage.reset(); }

    MapPtr              getFirstDrawMap();
    QVector<MapPtr>     getDrawMaps();
    QVector<const MapEditorLayer *> getDrawLayers();
    QVector<const MapEditorLayer *> getComposableLayers();

    QVector<MapPtr>     getMapLayerMaps();    // all layers

    static bool         isMotif(eMapEditorMapType type);

    void                setMouseMode(eMapEditorMouseMode emm) { mouseMode = emm; }
    eMapEditorMouseMode getMouseMode()              { return mouseMode;}

    void                setMouseInteraction(MapMouseActionPtr action) { mouse_interaction = action; }
    void                resetMouseInteraction()     { mouse_interaction.reset(); }
    MapMouseActionPtr   getMouseInteraction()       { return mouse_interaction; }

    bool                hasConstructionLines();

    UniqueQVector<QLineF>    constructionLines;
    UniqueQVector<CirclePtr> constructionCircles;

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


    WeakDCELPtr         activeDcel;
    DCELPtr             localDcel;

    BkgdImagePtr        bimage;
    MapMouseActionPtr   mouse_interaction;    // used by menu

private:

    int             viewSelect;         // OE'd bits
    eMapedLayer     editSelect;
    MapEditorLayer  __compositeLayer;
    MapEditorLayer  __layer1;
    MapEditorLayer  __layer2;
    MapEditorLayer  __layer3;
    CropPtr         _crop;

};

#endif // MAPEDITORDB_H
