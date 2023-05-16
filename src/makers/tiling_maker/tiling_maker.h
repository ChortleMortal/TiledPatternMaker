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

class QKeyEvent;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
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

static QString tm_states[]
{
    E2STR(TM_EMPTY),
    E2STR(TM_SINGLE),
    E2STR(TM_MULTI)
};

class TilingMaker : public TilingMakerView
{
    Q_OBJECT

public:
    static TilingMakerPtr  getSharedInstance();
    TilingMaker();  // don't call this

    void        init();

    void        sm_takeUp(TilingPtr tiling, eTILM_Event event);
    void        sm_takeDown(QVector<TilingPtr> &proto_tilings, eTILM_Event event);

    void        select(TilingPtr tiling, bool force = false);
    void        select(ProtoPtr prototype);
    TilingPtr   getSelected() { return selectedTiling; }
    int         numTilings() { return tilings.size(); }
    int         numExcluded() { return  allPlacedTiles.count() - in_tiling.count(); }
    void        setupMaker(TilingPtr tp);   // caution = generally use select()

    void        eraseTilings();
    void        removeTiling(TilingPtr tp);
    TilingPtr   findTilingByName(QString name);
    bool        isLoaded(QString name);
    bool        isValidTiling(TilingPtr tiling);

    void        unload();

    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        clearMakerData();
    void        updateTilingplacedTiles();
    void        updateVectors();
    void        updateReps();

    void        addInTiling(PlacedTilePtr pf);
    void        addInTilings(PlacedTiles & placedTiles);
    void        removeFromInTiling(PlacedTilePtr pf);

    void        flipTileRegularity(TilePtr tile);

    void        pushTilingToPrototypeMaker(ePROM_Event event);
    void        pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile);

    QVector<TilePtr>   getUniqueTiles();
    TileSelectorPtr  getCurrentSelection() { return tileSelector; }

    bool        verifyTiling();
    void        deleteTile(PlacedTilePtr pf);
    bool        accumHasPoint(QPointF wpt);

    eTilingMakerMouseMode getTilingMakerMouseMode();
    QString    getStatus();
    int        getPolygonSides() { return poly_side_count; }

    // Mouse interaction underway..
    void       drawMouseInteraction(GeoGraphics * g2d);

    // Tile management.
    void        addNewPlacedTile(PlacedTilePtr placedTile);
    void        addNewPlacedTiles(PlacedTiles &placedTiles);
    void        deleteTile(TileSelectorPtr sel);
    TileSelectorPtr addTileSelectionPointer(TileSelectorPtr sel );
    void        addToTranslate(QLineF mLine);
    void        setCurrentPlacedTile(PlacedTilePtr pfp) { currentPlacedTile = pfp;  emit sig_current_tile(pfp); }
    void        resetCurrentPlacedTile() { currentPlacedTile.reset();  emit sig_current_tile(currentPlacedTile); }
    void        toggleInclusion(TileSelectorPtr sel);
    bool        isIncluded(PlacedTilePtr pfp)  { return in_tiling.contains(pfp); }
    bool        procKeyEvent(QKeyEvent * k);

    void        clearConstructionLines() { constructionLines.clear(); }

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

signals:
    void        sig_tilingLoaded(QString name);
    void        sig_tilingWritten(QString name);

    void        sig_buildMenu();
    void        sig_refreshMenu();
    void        sig_current_tile(PlacedTilePtr pfp);
    void        sig_cycler_ready();

public slots:
    void        slot_loadTiling(QString name, eTILM_Event event);
    void        slot_cyclerLoadTiling(QString name);
    void        slot_saveTiling(QString name);

    void        updatePolygonSides(int number);
    void        updatePolygonRot(qreal angle);
    void        setTilingMakerMouseMode(eTilingMakerMouseMode mode);
    void        addRegularPolygon();
    void        fillUsingTranslations();
    void        removeExcluded();
    void        excludeAll();
    void        clearTranslationVectors();
    void        setTileEditPoint(QPointF pt);

    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_setCenter(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

protected slots:
    void slot_deleteTile();
    void slot_includeTile();
    void slot_excludeTile();
    void slot_editTile();
    void slot_copyMoveTile();
    void slot_uniquifyTile();
    void slot_convertTile();
    void slot_flatenCurve();
    void slot_makeConvex();
    void slot_makeConcave();
    void slot_moveArcCenter();
    void slot_editMagnitude();

protected:

    void updateVisibleVectors();
    void createFillCopies();
    void refillUsingTranslations();

    // state machine
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_resetCurrentAndAdd(TilingPtr tiling);
    void     sm_add(TilingPtr tiling);
    eTMState sm_getState();
    bool     sm_askAdd();

    // Mouse mode handling.
    void setMousePos(QPointF spt);
    void updateUnderMouse(QPointF  spt);

    // Possible user actions.
    void copyPolygon(TileSelectorPtr sel );
    void mirrorPolygonX(TileSelectorPtr sel);
    void mirrorPolygonY(TileSelectorPtr sel);
    bool reflectPolygon(TileSelectorPtr sel);

    // Mouse tracking.
    TileSelectorPtr findTileUnderMouse();

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

    // global modifications to features
    void tilingDeltaX(int delta);
    void tilingDeltaY(int delta);
    void tilingDeltaScale(int delta);
    void tilingDeltaRotate(int delta);

    void placedTileDeltaX(int delta);
    void placedTileDeltaY(int delta);
    void placedTileDeltaScale(int delta);
    void placedTileDeltaScale(qreal scale);
    void placedTileDeltaRotate(int delta);
    void placedTileDeltaRotate(qreal rotate);

    void uniqueTileDeltaScale(int delta);
    void uniqueTileDeltaScale(qreal scale);
    void uniqueTileDeltaRotate(int delta);
    void uniqueTileDeltaRotate(qreal rotate);

private:
    static TilingMakerPtr       spThis;

    UniqueQVector<TilingPtr>    tilings;
    TilingPtr                   selectedTiling;

    MouseActionPtr              mouse_interaction;
    TileSelectorPtr           clickedSelector;
    QPointF                     clickedSpt;

    bool                        filled;                     // state - currently filled or not
    int                         poly_side_count;            // number of selected vertices when drawing polygons.
    qreal                       poly_rotation;              // regular polygon tile rotation
    bool                        debugMouse;

    class ViewControl         * view;
    class PrototypeMaker      * prototypeMaker;
    class MapEditor           * maped;
    class ControlPanel        * controlPanel;
};

#endif
