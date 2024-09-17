#pragma once
#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "sys/enums/emapeditor.h"
#include "model/tilings/tiling.h"
#include "gui/map_editor/map_editor_mouseactions.h"
#include "gui/map_editor/map_editor_stash.h"

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class MapEditorSelection;
class MapEditorDb;
class MapEditorLayer;

class MapEditor : public  QObject
{
    friend class MoveVertex;
    friend class MoveEdge;
    friend class DrawLine;
    friend class ConstructionLine;
    friend class ExtendLine;
    friend class EditConstructionCircle;

    Q_OBJECT

public:
    MapEditor();
    ~MapEditor();
    void    init();

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
    bool    pushToMotif(MapPtr map);
    bool    pushToTiling(MapPtr map, bool outer);

    QString  getStatus();

    bool    procKeyEvent(QKeyEvent *k);

    void    initStash() { _stash.init(); }
    void    stash()     { _stash.stash(db); }
    bool    loadTemplate(VersionedFile xfile, bool animate);
    bool    saveTemplate(VersionedName & vname);
    bool    loadCurrentStash();
    bool    loadPrevStash();
    bool    loadNextStash();
    bool    keepStash(VersionedName name);
    bool    initStashFrom(VersionedName mosaicname);

    void    flipLineExtension();

    void setMapedMouseMode(eMapEditorMouseMode mapType);
    eMapEditorMouseMode getMouseMode();

    MapEditorDb        * getDb()        { return db; }
    MapEditorSelection * getSelector()  { return selector; }

    void     forceRedraw();

signals:
    void    sig_close();
    void    sig_updateView();
    void    sig_raiseMenu();

protected:

private:
    MapEditorStash         _stash;                // stash of construction lines
    QTimer                * timer;

    MapEditorDb           * db;
    MapEditorSelection    * selector;
    MapEditorView         * meView;

    class Configuration   * config;
    class MosaicMaker     * mosaicMaker;
    class PrototypeMaker  * prototypeMaker;
    class TilingMaker     * tilingMaker;
};

#endif
