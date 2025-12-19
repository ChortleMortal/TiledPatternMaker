#pragma once
#ifndef TILING_MAKER_H
#define TILING_MAKER_H

#include "sys/enums/edgetype.h"
#include "model/tilings/tiling_header.h"

////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly featureful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#include <QString>
#include "gui/viewers/tiling_maker_view.h"
#include "model/tilings/tiling.h"
#include "sys/enums/estatemachineevent.h"
#include "sys/enums/etilingmaker.h"
#include "sys/qt/unique_qvector.h"

class QKeyEvent;
class TilingHeader;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class TilingMouseAction>MouseActionPtr;
typedef std::shared_ptr<class Tile>              TilePtr;

#define E2STR(x) #x

enum eTMState
{
    TM_EMPTY,
    TM_SINGLE,
    TM_MULTI
};

class LoadUnit;

class TMStack
{
public:
    TMStack() { stackIndex  = -1; }

    void    clear()               { stack.clear(); stackIndex = -1; }
    bool    add(TilingUnit & td);
    bool    pop(TilingUnit & td);
    bool    redo(TilingUnit & td);

    QString getStackStatus()      { return QString("Stack=%1 index=%2").arg(stack.size()).arg(stackIndex); }

private:
    int                   stackIndex;
    QVector<TilingUnit>   stack;
};

class TilingMaker : public QObject
{
    Q_OBJECT

public:
    TilingMaker();
    ~TilingMaker();

    void        init();
    void        unload();

    TilingPtr   loadTiling(VersionedFile vfile, eTILM_Event event);
    void        saveTiling(TilingPtr tiling, bool forceOverwrite);

    void        sm_takeUp(const TilingEvent & tilingEvent);
    void        sm_takeDown(QVector<TilingPtr> &proto_tilings, eTILM_Event event);
    bool        getPropagate()         { return _tm_propagate; }
    void        reload();

    QString     getStatus();
    LoadUnit  * getLoadUnit() { return loadUnit; }

    bool        procKeyEvent(QKeyEvent * k);

    // Tiling related
    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        select(TilingPtr tiling);
    TilingPtr   getSelected() { return selectedTiling; }
    int         numTilings() { return tilings.size(); }
    void        removeTiling(TilingPtr tp);

    TilingPtr   findTilingByName(QString name);
    bool        isLoaded(QString name);
    bool        verifyTiling();

    void        removeExcludeds();
    void        updateVectors();
    void        updateReps();
    void        resetOverlaps();

    void        addRegularPolygon();

    // PlacedTile related
    void        addPlacedTile(PlacedTilePtr placedTile);
    void        deletePlacedTile(PlacedTilePtr ptp);

    void        replaceTilingUnit(TilingUnit & tilingUnit);

    void        unifyTile(PlacedTilePtr pf);
    void        uniquifyTile(PlacedTilePtr pf);
    void        decomposeTile(PlacedTilePtr pf);
    void        editTile(PlacedTilePtr pf);

    void        flipTileRegularity(TilePtr tile);

    eTilingMakerMouseMode getTilingMakerMouseMode() { return _tilingMakerMouseMode; }
    int         getPolygonSides() { return poly_side_count; }

    // Tile management.
    PlacedTileSelectorPtr addTileSelectionPointer(PlacedTileSelectorPtr sel );
    void        deleteTile(PlacedTileSelectorPtr sel);
    void        addToTranslate(QLineF mLine, QPointF origin);
    void        duplicateSelectedTiling();
    void        mergeTilings();

    void        toggleInclusion(PlacedTileSelectorPtr sel);
    void        clearConstructionLines() { Sys::tilingMakerView->clearConstructionLines(); }

    // selected tile
    inline PlacedTilePtr selectedTile()       { return _selectedTile; }
    void        selectTile(PlacedTilePtr ptp) { _selectedTile = ptp;  emit sig_tileSelected(ptp); }
    void        deselectTile()                { _selectedTile.reset(); }

    // tile selctor
    inline PlacedTileSelectorPtr clickedSelector()              { return _clickedSelector; }
    void        setClickedSelector(PlacedTileSelectorPtr tsp)   { _clickedSelector = tsp; }
    void        resetClickedSelector()                          { _clickedSelector.reset(); }

    void        setClickedPoint(QPointF pt)                     { clickedSpt      = pt; }

    TMStack &   getStack() { return tmStack; }

    // global modifications to features    // editing edge polys
    void              setEdgePolyEditor(DlgEdgePolyEdit* dlg)  { _epolyEdit = dlg; }
    DlgEdgePolyEdit * getEdgePolyEditor()                      { return  _epolyEdit; }

    void tilingDeltaX(qreal delta);
    void tilingDeltaY(qreal delta);
    void tilingDeltaScale(int delta);
    void tilingDeltaRotate(int delta);

    void placedTileDeltaX(qreal delta);
    void placedTileDeltaY(qreal delta);
    void placedTileDeltaScale(int delta);
    void placedTileDeltaRotate(int delta);

    void placedTileDeltaScale(qreal scale);
    void placedTileDeltaRotate(qreal rotate);

    void placedTileSetTranslate(qreal x, qreal y);
    void placedTileSetScale(qreal scale);
    void placedTileSetRotate(qreal rotate);

    // tiles dont't have x,y position - only scale and rotate;
    void uniqueTileDeltaScale(int delta);
    void uniqueTileDeltaRotate(int delta);

    void uniqueTileDeltaScale(qreal scale);
    void uniqueTileDeltaRotate(qreal rotate);

    // Possible user actions.
    void copyPolygon(PlacedTileSelectorPtr sel );
    void mirrorPolygonX(PlacedTileSelectorPtr sel);
    void mirrorPolygonY(PlacedTileSelectorPtr sel);
    bool reflectPolygon(PlacedTileSelectorPtr sel);

    void moveTileTo(QPointF pt);

    void forceRedraw();

signals:
    void sig_close();
    void sig_close_editor();
    void sig_tilingLoaded(VersionedFile name);
    void sig_tilingWritten();

    void sig_menuRefresh(eTileMenuRefresh scope);
    void sig_raiseMenu();

    void sig_updateView();
    void sig_reconstructView();

    void sig_tileSelected(PlacedTilePtr ptp);

public slots:
    void updatePolygonSides(int number);
    void updatePolygonRot(qreal angle);
    void setTilingMakerMouseMode(eTilingMakerMouseMode mode);
    void clearTranslationVectors();

    void slot_propagate_changed(bool val);
    void slot_deleteTile();
    void slot_hideTile();
    void slot_showTile();
    void slot_includeTile();
    void slot_excludeTile();
    void slot_view_menu_editTile();
    void slot_copyMoveTile();
    void slot_copyJoinPoint();
    void slot_copyJoinMidPoint();
    void slot_copyJoinEdge();
    void slot_uniquifyTile();
    void slot_convertTile();
    void slot_createConvex();
    void slot_createConcave();
    void slot_flatenCurve();
    void slot_makeConvex();
    void slot_makeConcave();
    void slot_moveArcCenter();
    void slot_editMagnitude();

    void slot_stack_save();
    void slot_stack_undo();
    void slot_stack_redo();

protected:
    // state machine
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_replaceCurrent(TilingPtr tiling);
    void     sm_replace(TilingPtr tiling, TilingPtr old);
    void     sm_add(TilingPtr tiling);
    eTMState sm_getState();
    bool     sm_askAdd();

    void     createCurveFromEdge(eCurveType ctype);

private:
    void setPropagate(bool enb) { _tm_propagate = enb; }

    LoadUnit *                  loadUnit;

    UniqueQVector<TilingPtr>    tilings;

    TilingPtr                   selectedTiling;

    PlacedTilePtr               _selectedTile;      // current menu column selection too
    PlacedTilePtr               unifyBase;

    TilingPtr                   decomposedTiling;   // contents decompsed from another tiling

    eVectorState                vectorState;

    PlacedTileSelectorPtr       _clickedSelector;
    QPointF                     clickedSpt;

    int                         poly_side_count;    // number of selected vertices when drawing polygons.
    qreal                       poly_rotation;      // regular polygon tile rotation

    TMStack                     tmStack;

    eTilingMakerMouseMode       _tilingMakerMouseMode;
    bool                        _tm_propagate;

    DlgEdgePolyEdit *           _epolyEdit;

    class PrototypeMaker      * prototypeMaker;
};

#endif
