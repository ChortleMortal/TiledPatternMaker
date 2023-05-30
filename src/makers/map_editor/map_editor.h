#pragma once
#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "enums/emapeditor.h"
#include "tile/tiling.h"
#include "makers/map_editor/map_editor_mouseactions.h"

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

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

public:
    static MapEditor * getInstance();
    static void        releaseInstance();

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

    void     forceRedraw();

protected:
    MapEditor();    // dont call this

private:
    static MapEditor *     mpThis;

    MapEditorDb           * db;
    MapEditorSelection    * selector;
    MapEditorView         * meView;

    class Configuration   * config;
    class ViewControl     * view;
    class MosaicMaker     * mosaicMaker;
    class PrototypeMaker  * prototypeMaker;
    class TilingMaker     * tilingMaker;
    class ControlPanel    * cpanel;
};

#endif
