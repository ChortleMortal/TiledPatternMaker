#pragma once
#include "model/tilings/tiling_data.h"
#ifndef TILING_MAKER_H
#define TILING_MAKER_H

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
#include "sys/enums/etilingmakermousemode.h"
#include "sys/enums/estatemachineevent.h"
#include "sys/qt/unique_qvector.h"

class QKeyEvent;
class TilingData;

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

enum eVectorState
{
    VEC_READY,
    VEC_SETTING_T1,
    VEC_SETTING_T2
};

static QString tm_states[]
{
    E2STR(TM_EMPTY),
    E2STR(TM_SINGLE),
    E2STR(TM_MULTI)
};

class TMStack
{
public:
    TMStack() { stackIndex  = -1; }

    void         clear()               { stack.clear(); stackIndex = -1; }
    bool         add(TilingData & td);
    bool         pop(TilingData & td);
    bool         redo(TilingData & td);

    QString     getStackStatus()      { return QString("size=%1 index=%2").arg(stack.size()).arg(stackIndex); }

private:
    int                   stackIndex;
    QVector<TilingData>   stack;
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
    void        saveTiling(TilingPtr tiling);

    void        sm_takeUp(TilingPtr tiling, eTILM_Event event);
    void        sm_takeDown(QVector<TilingPtr> &proto_tilings, eTILM_Event event);

    QString     getStatus();
    bool        tilingChanged();

    void        setPropagate(bool enb) { propagate = enb; }
    bool        getPropagate()         { return propagate; }

    // Tiling related
    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        select(TilingPtr tiling, bool force = false);
    TilingPtr   getSelected() { return selectedTiling; }
    int         numTilings() { return tilings.size(); }
    void        removeTiling(TilingPtr tp);
    void        setupMaker(TilingPtr tp);   // caution = generally use select()

    void        eraseTilings();
    TilingPtr   findTilingByName(QString name);
    bool        isLoaded(QString name);
    bool        isValidTiling(TilingPtr tiling);
    bool        verifyTiling();

    void        pushTilingToPrototypeMaker(ePROM_Event event);
    void        pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile);

    void        clearMakerData();
    void        removeExcludeds();
    void        updateVectors();
    void        updateReps();
    void        resetOverlaps();

    void        addRegularPolygon();

    // PlacedTile related
    TilingPlacements getTilingUnitPlacements() const;

    void        addPlacedTile(PlacedTilePtr pf);
    void        addNewPlacedTile(PlacedTilePtr placedTile);
    void        removePlacedTile(PlacedTilePtr pf);
    void        replaceTilingUnit(TilingUnit & tilingUnit);
    void        deletePlacedTile(PlacedTilePtr pf);

    void        unifyTile(PlacedTilePtr pf);

    // Tile related
    PlacedTileSelectorPtr  getCurrentSelection() { return tmView->getTileSelector(); }

    void        flipTileRegularity(TilePtr tile);

    eTilingMakerMouseMode getTilingMakerMouseMode();
    int        getPolygonSides() { return poly_side_count; }

    // Tile management.

    PlacedTileSelectorPtr addTileSelectionPointer(PlacedTileSelectorPtr sel );
    void        deleteTile(PlacedTileSelectorPtr sel);
    void        addToTranslate(QLineF mLine, QPointF origin);
    void        duplicateSelectedTiling();

    void          setCurrentPlacedTile(PlacedTilePtr pfp);
    void          resetCurrentPlacedTile() { currentPlacedTile.reset();  emit sig_current_tile(currentPlacedTile); }
    PlacedTilePtr getCurrentPlacedTile() { return currentPlacedTile; }

    void        toggleInclusion(PlacedTileSelectorPtr sel);
    bool        procKeyEvent(QKeyEvent * k);

    void        clearConstructionLines() { tmView->clearConstructionLines(); }

    void        setClickedSelector(PlacedTileSelectorPtr tsp) { clickedSelector = tsp; }
    void        setClickedPoint(QPointF pt)             { clickedSpt      = pt; }

    TMStack &   getStack() { return tmStack; }

    // global modifications to features
    void tilingDeltaX(qreal delta);
    void tilingDeltaY(qreal delta);
    void tilingDeltaScale(int delta);
    void tilingDeltaRotate(int delta);

    void placedTileDeltaX(qreal delta);
    void placedTileDeltaY(qreal delta);
    void placedTileSetTranslate(qreal x, qreal y);

    void placedTileDeltaScale(int delta);
    void placedTileDeltaScale(qreal scale);
    void placedTileSetScale(qreal scale);

    void placedTileDeltaRotate(int delta);
    void placedTileDeltaRotate(qreal rotate);
    void placedTileSetRotate(qreal rotate);

    void uniqueTileDeltaScale(int delta);
    void uniqueTileDeltaScale(qreal scale);
    void uniqueTileDeltaRotate(int delta);
    void uniqueTileDeltaRotate(qreal rotate);

    // Possible user actions.
    void copyPolygon(PlacedTileSelectorPtr sel );
    void mirrorPolygonX(PlacedTileSelectorPtr sel);
    void mirrorPolygonY(PlacedTileSelectorPtr sel);
    bool reflectPolygon(PlacedTileSelectorPtr sel);

signals:
    void sig_close();
    void sig_tilingLoaded(VersionedFile name);
    void sig_tilingWritten();

    void sig_buildMenu();
    void sig_refreshMenu();
    void sig_raiseMenu();
    void sig_current_tile(PlacedTilePtr pfp);

    void sig_updateView();
    void sig_reconstructView();

public slots:
    void updatePolygonSides(int number);
    void updatePolygonRot(qreal angle);
    void setTilingMakerMouseMode(eTilingMakerMouseMode mode);
    void clearTranslationVectors();

    void slot_deleteTile();
    void slot_hideTile();
    void slot_showTile();
    void slot_includeTile();
    void slot_excludeTile();
    void slot_editTile();
    void slot_copyMoveTile();
    void slot_copyJoinPoint();
    void slot_copyJoinMidPoint();
    void slot_copyJoinEdge();
    void slot_uniquifyTile();
    void slot_convertTile();
    void slot_createCurve();
    void slot_flatenCurve();
    void slot_makeConvex();
    void slot_makeConcave();
    void slot_moveArcCenter();
    void slot_editMagnitude();

    void slot_stack_save();
    void slot_stack_undo();
    void slot_stack_redo();

    void slot_debugCompose();
    void slot_debugDecompose();

protected:
    void     useLoadedTiling(TilingPtr tiling, VersionedFile &vfile, eTILM_Event event);

    // state machine
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_resetCurrentAndAdd(TilingPtr tiling);
    void     sm_add(TilingPtr tiling);
    eTMState sm_getState();
    bool     sm_askAdd();

    void     forceRedraw();

private:
    TilingMakerView *           tmView;

    bool                        propagate;

    UniqueQVector<TilingPtr>    tilings;
    TilingPtr                   selectedTiling;

    PlacedTilePtr               currentPlacedTile;  // current menu row selection too
    PlacedTilePtr               unifyBase;

    eTilingMakerMouseMode       tilingMakerMouseMode;     // set by tiling designer menu
    eVectorState                vectorState;

    PlacedTileSelectorPtr       clickedSelector;
    QPointF                     clickedSpt;

    int                         poly_side_count;            // number of selected vertices when drawing polygons.
    qreal                       poly_rotation;              // regular polygon tile rotation

    TMStack                     tmStack;

    TilingData                  _lastSelectedData;  // use only to detect changes
    bool                        _hasChanged;

    class ViewController      * viewControl;
    class PrototypeMaker      * prototypeMaker;
    class MapEditor           * maped;
    class ControlPanel        * controlPanel;
    class Configuration       * config;
};

#endif
