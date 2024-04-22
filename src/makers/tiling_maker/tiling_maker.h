#pragma once
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
#include "viewers/tiling_maker_view.h"
#include "enums/etilingmakermousemode.h"
#include "enums/estatemachineevent.h"
#include "misc/unique_qvector.h"

class QKeyEvent;
class TilingMonitor;
class TilingData;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class TilingMouseAction>MouseActionPtr;
typedef std::shared_ptr<class Tile>              TilePtr;

typedef QVector<PlacedTilePtr> PlacedTiles;

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

class TilingMaker : public QObject
{
    Q_OBJECT

public:
    TilingMaker();
    ~TilingMaker();
    void        init();

    TilingPtr   loadTiling(QString name, eTILM_Event event);
    void        saveTiling(QString name);

    void        sm_takeUp(TilingPtr tiling, eTILM_Event event);
    void        sm_takeDown(QVector<TilingPtr> &proto_tilings, eTILM_Event event);

    QString     getStatus();
    void        setPropagate(bool enb) { propagate = enb; }
    bool        getPropagate()         { return propagate; }

    // Tiling related
    void        select(TilingPtr tiling, bool force = false);
    void        select(ProtoPtr prototype);
    TilingPtr   getSelected() { return selectedTiling; }
    int         numTilings() { return tilings.size(); }
    void        removeTiling(TilingPtr tp);
    void        setupMaker(TilingPtr tp);   // caution = generally use select()

    void        eraseTilings();
    TilingPtr   findTilingByName(QString name);
    bool        isLoaded(QString name);
    bool        isValidTiling(TilingPtr tiling);

    void        unload();

    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        pushTilingToPrototypeMaker(ePROM_Event event);

    void        clearMakerData();
    void        updateVectors();
    void        updateReps();
    void        resetOverlaps();

    // Tile related
    const PlacedTiles & getInTiling() const;

    void        pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile);

    void        addInTiling(PlacedTilePtr pf);
    void        replaceInTilings(PlacedTiles & placedTiles);
    void        removeFromInTiling(PlacedTilePtr pf);

    int         numExcluded();
    void        flipTileRegularity(TilePtr tile);

    QVector<TilePtr> getUniqueTiles();
    TileSelectorPtr  getCurrentSelection() { return tmView->getTileSelector(); }

    bool        verifyTiling();
    void        deleteTile(PlacedTilePtr pf);
    void        unifyTile(PlacedTilePtr pf);

    eTilingMakerMouseMode getTilingMakerMouseMode();
    int        getPolygonSides() { return poly_side_count; }

    TilingMonitor * getTilingMonitor() { return tilingMonitor; }

    // Tile management.
    void        addNewPlacedTile(PlacedTilePtr placedTile);
    void        replacePlacedTiles(PlacedTiles &placedTiles);
    void        deleteTile(TileSelectorPtr sel);
    TileSelectorPtr addTileSelectionPointer(TileSelectorPtr sel );
    void        addToTranslate(QLineF mLine, QPointF origin);

    void        setCurrentPlacedTile(PlacedTilePtr pfp);
    void        resetCurrentPlacedTile() { currentPlacedTile.reset();  emit sig_current_tile(currentPlacedTile); }
    PlacedTilePtr getCurrentPlacedTile() { return currentPlacedTile; }

    void        toggleInclusion(TileSelectorPtr sel);
    bool        isIncluded(PlacedTilePtr pfp);
    bool        procKeyEvent(QKeyEvent * k);

    void        clearConstructionLines() { tmView->clearConstructionLines(); }

    void        setClickedSelector(TileSelectorPtr tsp) { clickedSelector = tsp; }
    void        setClickedPoint(QPointF pt)             { clickedSpt      = pt; }

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
    void copyPolygon(TileSelectorPtr sel );
    void mirrorPolygonX(TileSelectorPtr sel);
    void mirrorPolygonY(TileSelectorPtr sel);
    bool reflectPolygon(TileSelectorPtr sel);

signals:
    void sig_tilingLoaded(QString name);
    void sig_tilingWritten(QString name);

    void sig_buildMenu();
    void sig_refreshMenu();
    void sig_current_tile(PlacedTilePtr pfp);

    void sig_monitor(bool reset);

public slots:
    void updatePolygonSides(int number);
    void updatePolygonRot(qreal angle);
    void setTilingMakerMouseMode(eTilingMakerMouseMode mode);
    void addRegularPolygon();
    void slot_fillUsingTranslations();
    void removeExcluded();
    void clearTranslationVectors();

    void slot_deleteTile();
    void slot_hideTile();
    void slot_showTile();
    void slot_includeTile();
    void slot_excludeTile();
    void slot_editTile();
    void slot_copyMoveTile();
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
    void fillUsingTranslations();

    // state machine
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_resetCurrentAndAdd(TilingPtr tiling);
    void     sm_add(TilingPtr tiling);
    eTMState sm_getState();
    bool     sm_askAdd();

    void forceRedraw();

private:
    TilingMakerView *           tmView;
    TilingMonitor   *           tilingMonitor;

    bool                        propagate;

    UniqueQVector<TilingPtr>    tilings;
    TilingPtr                   selectedTiling;

    PlacedTilePtr               currentPlacedTile;  // current menu row selection too
    PlacedTilePtr               unifyBase;

    eTilingMakerMouseMode       tilingMakerMouseMode;     // set by tiling designer menu
    eVectorState                vectorState;

    TileSelectorPtr             clickedSelector;
    QPointF                     clickedSpt;

    int                         poly_side_count;            // number of selected vertices when drawing polygons.
    qreal                       poly_rotation;              // regular polygon tile rotation

    QStack<TilingData>          undoStack;
    QStack<TilingData>          redoStack;

    class View                * view;
    class ViewController      * viewControl;
    class PrototypeMaker      * prototypeMaker;
    class MapEditor           * maped;
    class ControlPanel        * controlPanel;
    class Configuration       * config;
};

#endif
