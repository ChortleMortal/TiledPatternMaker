#pragma once

#include "gui/map_editor/map_editor_layer.h"
#include "gui/map_editor/map_editor_mouseactions.h"
#include "gui/map_editor/map_editor_stash.h"
#include "model/makers/mosaic_maker.h"

extern const QString sMapEditorMapType[];
extern const QString sMapEditorMode[];
extern const QString sMapEditorMouseMode[];

class MapEditor : public  QObject
{
    friend class MoveVertex;
    friend class MoveEdge;
    friend class DrawLine;
    friend class ConstructionLine;
    friend class ExtendLine;
    friend class EditConstructionCircle;

    using LayerId = MapEditorLayer::LayerId;
    using MapType = MapEditorMap::MapType;

    Q_OBJECT

public:

    enum class EditorMode
    {
        Map = 0,
        DCEL,
    };

    enum class MouseMode
    {
        None,
        Delete,
        DrawLine,
        SplitLine,
        ExtendP1,
        ExtendP2,
        CreateLine,
        ConstructionLines,
        ConstructionCircles,
    };

    MapEditor();
    ~MapEditor();

    void    init();

    bool    loadFromMosaicStyle(StylePtr style);
    bool    loadFromfPrototypeMaker();
    bool    loadMotifUnit();
    bool    loadSelectedMotifs();
    bool    loadTilingUnit();
    bool    loadTilingRepeated();
    bool    loadFromMap(MapPtr map, MapEditorMap::MapType type);

    void    unload();

    bool    useExistingDCEL(MapPtr map);
    bool    createLocalDCEL(MapPtr map);

    bool    embedCrop(MapPtr map);
    bool    cropMap(MapPtr map);

    bool    pushToMosaic(MapEditorLayer & layer, bool convertToMap = false);
    bool    pushToMotif(MapEditorLayer & layer);
    bool    pushToTiling(MapEditorLayer &layer, bool outer);

    QString  getStatus();

    bool    procKeyEvent(QKeyEvent *k);

    void    setViewSelect(LayerId layer, bool on);
    bool    isViewSelected(LayerId layer)   { return viewSelect & (int)layer; }
    void    setEditSelect(LayerId layer)    { editSelect = layer; }
    LayerId getEditSelect()                 { return editSelect; }

    void    stash()     { MapEditorStash::stash(db); }
    bool    loadTemplate(VersionedFile xfile);
    bool    saveTemplate(VersionedName & vname);

    void      setMapedMouseMode(MouseMode mapType);
    MouseMode getMapedMouseMode();

    MapEditorLayer &    getLayer(LayerId layer);
    MapEditorLayer &    getLayer(MapPtr map);
    MapEditorLayer &    getEditLayer();

    LayerId             obtainEmptyLayer();
    void                createComposite();

    QVector<const MapEditorLayer *> gerUsedComposableLayers();
    QVector<const MapEditorLayer *> getDrawLayers();
    QVector<TimoMap *>              getDrawTimoMaps();
    TileMotifSet                    getEditTileMotifs();

    QVector<MapPtr>     getMapLayerMaps();    // all layers
    MapPtr              getMap(LayerId mode);
    MapType             getMapType(MapPtr map);
    MapPtr              getEditMap();
    MapPtr              getFirstDrawMap();
    QVector<MapPtr>     getDrawMaps();

    MapEditorDb        * getDb()        { return db; }
    MapEditorSelection * getSelector()  { return selector; }

    void     forceRedraw();

signals:
    void    sig_close();
    void    sig_updateView();
    void    sig_raiseMenu();
    void    sig_styleMapUpdated(MapPtr map);

protected:
    void     updateMosiacWithPrototype(ProtoPtr proto, MosaicMaker::Event event, ProtoPtr oldProto = ProtoPtr());

private:
    MapEditorDb           * db;
    MapEditorSelection    * selector;

    int                     viewSelect;     // OR'd bits
    MapEditorLayer::LayerId editSelect;

    MapEditorLayer  __compositeLayer;
    MapEditorLayer  __layer1;
    MapEditorLayer  __layer2;
    MapEditorLayer  __layer3;

    class Configuration   * config;
    class MosaicMaker     * mosaicMaker;
    class MotifMaker      * motifMaker;
    class TilingMaker     * tilingMaker;
};
