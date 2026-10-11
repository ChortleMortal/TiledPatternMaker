#pragma once

////////////////////////////////////////////////////////////////////////////
//
// FeatureView.java
//
// It's unlikely this file will get used in the applet.  A TilingMakerView
// is a special kind of GeoView that assumes a subclass will maintain
// a collection of Tiles.  It knows how to draw Tiles quickly,
// and provides a bunch of services to subclasses for mouse-based
// interaction with features.

#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/viewers/layer_controller.h"
#include "model/tilings/placed_tile.h"
#include "sys/geometry/edge_poly.h"

class GeoGraphics;
class DlgEdgePolyEdit;

typedef std::shared_ptr<class Tiling>               TilingPtr;
typedef std::shared_ptr<class PlacedTileSelector>   PlacedTileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>           PlacedTilePtr;
typedef std::shared_ptr<class TileMotif>            TileMotifPtr;
typedef std::weak_ptr<class Tiling>                 WeakTilingPtr;

class TilingMaker;

class TilingMakerView : public LayerController
{
    Q_OBJECT

public:
    enum class ViewMode
    {
        showTiles,
        showAttachment,
        showExcluded,
        showUnique,
    };

    static constexpr QColor tile_color          = QColor(217,217,255);
    static constexpr QColor overlapping_color   = QColor(205,102, 25);  // ochre
    static constexpr QColor touching_color      = QColor( 25,102,205);  // blue
    static constexpr QColor excluded_color      = QColor(255,217,217);  // pink
    static constexpr QColor included_color      = QColor(217,255,255);
    static constexpr QColor unique_color        = QColor(255,255,217);
    static constexpr QColor selected_color      = QColor(127,255,127);  // green
    static constexpr QColor under_mouse_color   = QColor(255,105,180);  // hot pink
    static constexpr QColor circle_color        = QColor(202,200,  0,128);
    static constexpr QColor construction_color  = QColor(  0,128,  0,128);
    static constexpr QColor construction_mid_color = QColor(  0,  0,255,200);
    static constexpr QColor construction_crossing_color = QColor(  0,255,255,220);

    TilingMakerView();
    ~TilingMakerView() override;

    void setMaker(TilingMaker * maker) { tilingMaker = maker;}

    void setTiling(TilingPtr tiling);
    void setFill(bool enb);

    void unloadLayerContent() override { wTiling.reset(); clearViewData();}

    void clearViewData();
    void clearConstructionLines() { constructionLines.clear(); selectedConstructionLine = -1; forceRedraw(); }

    enum class ConstructionEndpointState { None, Near, Match };

    // construction lines and translation vectors live in canvas space, not model space -
    // they must never be warped by canvas distortion, unlike the tiles they're drawn alongside.
    QPointF undistortedScreenToModel(QPointF spt);
    QPointF undistortedModelToScreen(QPointF wpt);
    QLineF  undistortedModelToScreen(QLineF line);

    int     findConstructionLineNear(QPointF spt);
    int     findConstructionEndpointNear(int lineIndex, QPointF spt);
    bool    findConstructionSnapCandidate(QPointF spt, QPointF & outPt, PlacedTileSelectorPtr ignore = PlacedTileSelectorPtr(), int excludeLineIndex = -1);
    QPointF findConstructionSnapPoint(QPointF spt);
    ConstructionEndpointState classifyConstructionEndpoint(QPointF pt, int excludeLineIndex);
    int  getSelectedConstructionLine()             { return selectedConstructionLine; }
    void setSelectedConstructionLine(int index)    { selectedConstructionLine = index; }

    void paint(QPainter * painter) override;

    void drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool drawCenter, QColor color);
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);
    void drawAnchors(GeoGraphics * g2d);
    void drawTranslationVectors(GeoGraphics * g2d, TilingPtr tiling, const PlacedTileSet &set);
    void drawTranslationAnchors(GeoGraphics * g2d);

    void hideVectors(bool state);

    ViewMode getViewMode()                          { return _viewMode; }
    void     setViewMode(ViewMode mode)             { _viewMode = mode; }

    const PlacedTileSet & getViewablePlacedTiles();

    void setViewPlacedTileSelector(PlacedTileSelectorPtr tsp, bool toggle = false);
    inline PlacedTileSelectorPtr viewPlacedTileSelector() { return _viewPlacedTileSelector; }

    PlacedTileSelectorPtr findSelection(QPointF spt);
    PlacedTileSelectorPtr findTile(QPointF spt);
    PlacedTileSelectorPtr findEdge(QPointF spt);
    PlacedTileSelectorPtr findPoint(QPointF spt);
    PlacedTileSelectorPtr findVertex(QPointF spt);
    PlacedTileSelectorPtr findMidPoint(QPointF spt);
    PlacedTileSelectorPtr findArcPoint(QPointF spt);

    PlacedTileSelectorPtr findTile(QPointF spt, PlacedTileSelectorPtr ignore);
    PlacedTileSelectorPtr findEdge(QPointF spt, PlacedTileSelectorPtr ignore );
    PlacedTileSelectorPtr findPoint(QPointF spt, PlacedTileSelectorPtr ignore);
    PlacedTileSelectorPtr findVertex(QPointF spt, PlacedTileSelectorPtr ignore);
    PlacedTileSelectorPtr findMidPoint(QPointF spt, PlacedTileSelectorPtr ignore);

    PlacedTileSelectorPtr findCenter(PlacedTilePtr feature, QPointF spt);
    QPointF               findSelectionPointOrPoint(QPointF spt);
    PlacedTileSelectorPtr findNearGridPoint(QPointF spt);

    EdgeSet                & getAccumW()        { return wAccum; }
    QVector<Measurement*>  & getMeasurementsS() { return wMeasurements; }
    QVector<QLineF>    & getConstructionLines() { return constructionLines; }
    QPointF                  getMousePos()      { return sMousePos; }

    bool accumHasPoint(QPointF wpt);

    // Mouse tracking.
    PlacedTileSelectorPtr findTileUnderMouse();
    void setMousePos(QPointF spt);

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, Qt::MouseButton mouseButton);
    void setMouseInteraction(MouseActionPtr mouseAct) { mouse_interaction = mouseAct; }
    void resetMouseInteraction()                      { mouse_interaction.reset();}
    MouseActionPtr getMouseInteraction()              { return mouse_interaction; }

    // Mouse mode handling.
    void updateUnderMouse(QPointF  spt);

    // Mouse interaction underway..
    void drawMouseInteraction(GeoGraphics * g2d);

    // editing edge polys
    inline PlacedTilePtr editPlacedTile()     { return _editPlacedTile; }
    void setEditPlacedTile(PlacedTilePtr ptp) { _editPlacedTile = ptp; }
    void resetEditPlacedTile()                { _editPlacedTile.reset(); }

public slots:
    void iamaLayerController() override {}

    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

    void slot_mouseTranslate(uint sigid, QPointF pt)      override;
    void slot_wheel_scale(uint sigid, qreal delta)  override;
    void slot_wheel_rotate(uint sigid, qreal delta) override;

    void slot_scale(uint sigid, int amount)    override;
    void slot_rotate(uint sigid, int amount)   override;
    void slot_moveX(uint sigid, qreal amount)  override;
    void slot_moveY(uint sigid, qreal amount)  override;

    void slot_setTileEditPoint(QPointF pt);
    void slot_subMenu1();
    void slot_TL();
    void slot_TR();
    void slot_BL();
    void slot_BR();

protected:
    void draw(GeoGraphics * g2d);
    void drawTiling(GeoGraphics * g2d, const PlacedTileSet &set);
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void determineOverlapsAndTouching(const PlacedTileSet & placedTiles);
    PlacedTileSet _createViewablePlacedTiles(TilingPtr tiling);

    inline bool tileIsSelected(const PlacedTilePtr &tile);
    inline bool tileUnderMouse(const PlacedTilePtr & tile);

private:
    WeakTilingPtr           wTiling;

    ViewMode                _viewMode;

    EdgeSet                 wAccum;             // world points
    QVector<Measurement*>   wMeasurements;

    bool                    _hideVectors;

    PlacedTileSelectorPtr   _viewPlacedTileSelector;      // Current mouse selection.
    PlacedTilePtr           _editPlacedTile;    // Tile in DlgTileEdit

    QColor                  lineColor;
    QPointF                 sMousePos;          // screen points DAC added
    QPointF                 tileEditPoint;
    QVector<QLineF>         constructionLines;
    int                     selectedConstructionLine = -1;

    MouseActionPtr          mouse_interaction;

    TilingMaker           * tilingMaker;

    bool                    debugMouse;
};
