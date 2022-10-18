#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "enums/emapeditor.h"
#include "tile/tiling.h"
#include "makers/map_editor/map_editor_mouseactions.h"

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class TilingMaker>   TilingMakerPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;
typedef std::shared_ptr<class MapEditorView> MapedViewPtr;

class MapEditorSelection;
class MapEditorDb;
class MapEditorLayer;

class MapEditor
{
    friend class MoveVertex;
    friend class MoveEdge;
    friend class DrawLine;
    friend class ConstructionLine;
    friend class ExtendLine;
    friend class EditConstructionCircle;
    friend class CreateCrop;
    friend class CreateMosaicCrop;
    friend class EditCrop;

public:
    static MapEditor * getInstance();

    bool    loadMosaicPrototype();
    void    loadMotifPrototype();
    bool    loadSelectedMotifs();
    void    loadTilingUnit();
    void    loadTilingRepeated();
    void    loadFromMap(MapPtr map, eMapEditorMapType type);

    void    unload();

    bool    useExistingDCEL(MapPtr map);
    bool    createLocalDCEL(MapPtr map);

    bool    pushToMosaic(MapEditorLayer & layer);
    bool    convertToMotif(MapPtr map);
    bool    convertToTiling(MapPtr map, bool outer);

    QString  getStatus();

    bool    procKeyEvent(QKeyEvent *k);

    void    cleanupMapPoints();

    bool    loadCurrentStash();
    bool    loadNamedStash(QString file, bool animate);
    bool    loadPrevStash();
    bool    loadNextStash();
    bool    keepStash(QString name);
    bool    initStashFrom(QString name);

    void    flipLineExtension();

    void setMapedMouseMode(eMapEditorMouseMode mapType);
    eMapEditorMouseMode getMouseMode();

    MapEditorDb        * getDb()        { return db; }
    MapEditorSelection * getSelector()  { return selector; }
    MapedViewPtr         getMapedView() { return meView; }

    void     forceRedraw();

protected:
    MapEditor();    // dont call this

private:
    static MapEditor *     mpThis;

    MapEditorDb           * db;
    MapEditorSelection    * selector;
    MapedViewPtr            meView;

    class Configuration   * config;
    class ViewControl     * view;
    class MosaicMaker     * mosaicMaker;
    class MotifMaker      * motifMaker;
    TilingMakerPtr          tilingMaker;
    class ControlPanel    * cpanel;
};

#endif
