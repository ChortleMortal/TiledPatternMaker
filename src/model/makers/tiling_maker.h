#pragma once

#include <QMessageBox>
#include <QString>

#include "gui/viewers/tiling_maker_view.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_header.h"
#include "sys/enums/edgetype.h"
#include "sys/enums/etilingmaker.h"
#include "sys/qt/unique_qvector.h"
#include "sys/sys.h"

////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly featureful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

class LoadUnit;
class QKeyEvent;
class TilingEvent;
class TilingHeader;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class TilingMouseAction>MouseActionPtr;
typedef std::shared_ptr<class Tile>              TilePtr;
typedef std::shared_ptr<class Edge>              EdgePtr;

// A persistent link between two placed tiles' edge midpoints (runtime-only, not saved to file).
// Edges are identified by index into the tile's edge list rather than by EdgePtr: scaling or
// rotating a Tile's shape (EdgePoly::compose()) discards and replaces all of its Edge objects,
// which would leave a stored EdgePtr pointing at stale, orphaned geometry. The index survives
// because compose() preserves edge count and order.
struct TileAnchor
{
    PlacedTilePtr tileA;
    int           edgeIndexA;
    PlacedTilePtr tileB;
    int           edgeIndexB;
};

// Ties one of a tiling's translation-vector points (origin, or a vector's endpoint) to a placed
// tile, so it tracks that tile's placement. Runtime-only, not saved to file. A null tile means
// unanchored: the point stays fixed in absolute space.
struct TransAnchor
{
    PlacedTilePtr tile;
    QPointF       localPt;   // anchor point in tile's local (pre-placement) frame
};

extern const QString sTMModelMode[];
extern const QString sTILM_Events[];
extern const QString sTilingMakerMouseMode[];

class TMStack
{
public:
    TMStack() { stackIndex  = -1; }

    void    clear()               { stack.clear(); stackIndex = -1; }
    bool    add(TilingUnitController & td);
    bool    pop(TilingUnitController & td);
    bool    redo(TilingUnitController & td);

    QString getStackStatus();

private:
    int stackIndex;
    QVector<TilingUnitController>   stack;
};

class TilingMaker : public QObject
{
    Q_OBJECT

    enum class State
    {
        Empty = 0,
        Single,
        Multi
    };

public:

    enum class eTilingMakerEvent
    {
        LoadEmpty = 0,
        LoadFromMosaic,
        LoadSingle,
        LoadMulti,
        LoadReplace,
        ReplaceAll,
        Reload,
        LoadFromStepper,
        UnloadTiling
    };

    enum class eTilingMakerMouseMode
    {
        None = 0,
        Copy,
        Delete,
        TranslationVectors,
        DrawPolygon,
        Inclusion,
        Position,
        Measure,
        Edit,
        CurveEdge,
        MirrorX,
        MirrorY,
        ConstructionLines,
        ReflectEdge,
        Uniquify,
        Unify,
        Decompose,
    };

    enum eKeyboardMode
    {
        XformAll = 0,
        XformTiling,
        XFormUniqueTile,
        XFormPlacedTile
    };

    using Event     = eTilingMakerEvent;
    using MouseMode = eTilingMakerMouseMode;
    using KeybdMode = eKeyboardMode;


    TilingMaker();
    ~TilingMaker();

    void        init();
    void        unload();

    void        loadTiling(VersionedFile vfile, Event event);
    bool        saveTiling(TilingPtr tiling, VersionedFile vfile);
    TilingPtr   readTiling(VersionedFile & vfile, QString & failMsg);     // no state change
    bool        verifyTilingFiles();

    void        sm_takeUp(const TilingEvent & tilingEvent);
    void        sm_takeDown(QVector<TilingPtr> &proto_tilings, Event event);
    bool        getPropagate()         { return _tm_propagate; }
    void        reload();

    QString     getStatus();
    LoadUnit  * getLoadUnit() { return tilingLoadUnit; }

    bool        procKeyEvent(QKeyEvent * k);

    // Tiling related
    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        selectTiling(TilingPtr tiling);
    TilingPtr   getSelectedTiling() { return selectedTiling; }
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

    void        replaceTilingUnit(TilingUnitController & tilingUnit);

    void        unifyTile(PlacedTilePtr pf);
    void        uniquifyTile(PlacedTilePtr pf);
    void        decomposeTile(PlacedTilePtr pf);
    void        editTile(PlacedTilePtr pf);

    void        flipTileRegularity(TilePtr tile);

    MouseMode   getMouseMode() { return _tilingMakerMouseMode; }

    // Tile management.
    PlacedTileSelectorPtr addTileSelectionPointer(PlacedTileSelectorPtr sel );
    void        deleteTile(PlacedTileSelectorPtr sel);
    void        addToTranslate(QLineF mLine, QPointF origin, PlacedTileSelectorPtr originSel, PlacedTileSelectorPtr endSel);
    void        duplicateSelectedTiling();
    void        mergeTilings();

    void        toggleInclusion(PlacedTileSelectorPtr sel);
    void        clearConstructionLines() { Sys::tilingMakerView->clearConstructionLines(); }
    void        saveConstructionLines();   // pushes the live construction lines into the selected tiling
    void        loadConstructionLines();   // pulls the selected tiling's saved construction lines into the view
    bool        constructionLinesNotSaved(TilingPtr tiling);  // view lines differ from the tiling's

    // selected placed tile
    inline PlacedTilePtr selectedTile()                         { return _selectedPlacedTile; }
    void        selectTile(PlacedTilePtr ptp,bool toggle = false);

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

    // Tile anchors - persistent edge-midpoint links used to keep anchored tiles in contact.
    const QVector<TileAnchor> & getAnchors() const { return anchors; }
    void snapEdgeToEdge(PlacedTilePtr moveTile, EdgePtr moveEdge, PlacedTilePtr fixedTile, EdgePtr fixedEdge);
    void propagateAnchors(PlacedTilePtr root);
    static EdgePtr edgeAt(PlacedTilePtr tile, int edgeIndex);

    // Translation-vector anchors: origin/T1-end/T2-end each independently track a placed tile.
    const TransAnchor & getOriginAnchor() const { return originAnchor; }
    const TransAnchor & getT1Anchor()     const { return t1Anchor; }
    const TransAnchor & getT2Anchor()     const { return t2Anchor; }
    void resolveTranslationAnchors();
    void clearTranslationAnchors();

    // Possible user actions.
    void copyPolygon(PlacedTileSelectorPtr sel );
    void mirrorPolygonX(PlacedTileSelectorPtr sel);
    void mirrorPolygonY(PlacedTileSelectorPtr sel);
    bool reflectPolygon(PlacedTileSelectorPtr sel);

    void moveTileTo(QPointF pt);

    // keyboard view modes
    void      setKeybdMode(KeybdMode mode);
    bool      isKeybdMode(KeybdMode mode);
    KeybdMode getKeybdMode() { return _tilingMakerKeyboardMode; }
    QString   getKeybdModeString();
    void      resetKeybdMode();

    void forceRedraw();

signals:
    void sig_close();
    void sig_close_editor();
    void sig_tilingLoading(VersionedFile name);
    void sig_tilingLoaded( VersionedFile name);   // a new/different tiling is now in memory
    void sig_tilingWritten(VersionedFile name);   // the current in-memory tiling was saved (data unchanged)
    void sig_messageBox(QString msg, QString info, QMessageBox::Icon icon, bool modal);

    void sig_menuRefresh(eTileMenuRefresh scope);
    void sig_raiseMenu();

    void sig_updateView();
    void sig_reconstructView();

    void sig_TMKbdMode(TilingMaker::eKeyboardMode mode);

public slots:
    void slot_propagate_changed(bool val);
    void slot_view_menu_editTile();

    void setMouseMode(TilingMaker::eTilingMakerMouseMode mode);
    void slot_copyMoveTile();
    void slot_copyJoinPoint();
    void slot_copyJoinMidPoint();
    void slot_copyJoinEdge();

    void slot_createAnchor();
    void slot_removeAnchor();

    void slot_hideTile();
    void slot_deselectTile();
    void slot_showTile();
    void slot_deleteTile();
    void slot_includeTile();
    void slot_excludeTile();
    void slot_uniquifyTile();
    void slot_convertTile();
    void slot_stack_save();
    void slot_stack_undo();
    void slot_stack_redo();

    void clearTranslationVectors();

private slots:
    void slot_deleteConstructionLine();

    void slot_createConvex();
    void slot_createConcave();
    void slot_flatenCurve();
    void slot_makeConvex();
    void slot_makeConcave();
    void slot_moveArcCenter();
    void slot_editMagnitude();

private:
    // state machine
    void     setPropagate(bool enb) { _tm_propagate = enb; }
    void     sm_takeUp();
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_replaceCurrent(TilingPtr tiling);
    void     sm_replace(TilingPtr tiling, TilingPtr old);
    void     sm_add(TilingPtr tiling);
    State    sm_getState();

    void     sm_syncConstructionLinesOnLoad();     // loads selectedTiling's lines if the view is enabled, else clears

    void     createCurveFromEdge(eCurveType ctype);

    TransAnchor bindTranslationAnchor(PlacedTileSelectorPtr sel);
    void        updateAnchoredTranslationVectors();

private:
    UniqueQVector<TilingPtr>    tilings;
    TilingPtr                   selectedTiling;
    PlacedTilePtr               _selectedPlacedTile;      // current menu column selection too
    PlacedTilePtr               unifyBase;
    TilingPtr                   decomposedTiling;   // contents decompsed from another tiling
    eVectorState                vectorState;
    QPointF                     clickedSpt;

    TMStack                     tmStack;
    QVector<TileAnchor>         anchors;
    TransAnchor                 originAnchor;
    TransAnchor                 t1Anchor;
    TransAnchor                 t2Anchor;
    MouseMode                   _tilingMakerMouseMode;
    KeybdMode                   _tilingMakerKeyboardMode;
    bool                        _tm_propagate;
    DlgEdgePolyEdit *           _epolyEdit;

    LoadUnit *                  tilingLoadUnit;
    MotifMaker                * motifMaker;
};

class TilingEvent
{
public:
    using Event = TilingMaker::eTilingMakerEvent;

    Event              event;
    TilingPtr          tiling;
    TilingPtr          lastTiling;   // used by replace
};

Q_DECLARE_METATYPE(TilingMaker::KeybdMode);
Q_DECLARE_METATYPE(TilingMaker::MouseMode);
