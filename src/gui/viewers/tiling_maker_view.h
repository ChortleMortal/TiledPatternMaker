#pragma once
#ifndef TILING_MAKER_VIEW_H
#define TILING_MAKER_VIEW_H

////////////////////////////////////////////////////////////////////////////
//
// FeatureView.java
//
// It's unlikely this file will get used in the applet.  A TilingMakerView
// is a special kind of GeoView that assumes a subclass will maintain
// a collection of Tiles.  It knows how to draw Tiles quickly,
// and provides a bunch of services to subclasses for mouse-based
// interaction with features.

#include "gui/viewers/layer_controller.h"
#include "sys/geometry/edgepoly.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"

class GeoGraphics;

typedef std::shared_ptr<class Tiling>       TilingPtr;
typedef std::weak_ptr<class Tiling>        wTilingPtr;
typedef std::shared_ptr<class PlacedTileSelector> PlacedTileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;

class TilingMaker;

class TilingMakerView : public LayerController
{
    Q_OBJECT

public:
    TilingMakerView();
    ~TilingMakerView() override;

    void setMaker(TilingMaker * maker) { tilingMaker = maker;}

    void setTiling(TilingPtr tiling);
    void setFill(bool enb);

    void clearViewData();
    void clearConstructionLines() { constructionLines.clear(); }

    void paint(QPainter * painter) override;

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

    void drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool draw_c, QColor icol );
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);
    void drawTranslationVectors(GeoGraphics * g2d, TilingPtr tiling);

    void hideTiling(bool state);
    void hideVectors(bool state);

    PlacedTileSelectorPtr getTileSelector()                   { return tileSelector; }
    void                  resetTileSelector()                 { tileSelector.reset(); }
    void                  setTileSelector(PlacedTileSelectorPtr tsp){ tileSelector = tsp; }

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

    EdgePoly               & getAccumW()        { return wAccum; }
    QVector<Measurement*>  & getMeasurementsS() { return wMeasurements; }
    QVector<QLineF>    & getConstructionLines() { return constructionLines; }
    QPointF                  getMousePos()      { return sMousePos; }

    bool accumHasPoint(QPointF wpt);

    // Mouse tracking.
    PlacedTileSelectorPtr findTileUnderMouse();
    void setMousePos(QPointF spt);

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);
    void setMouseInteraction(MouseActionPtr mouseAct) { mouse_interaction = mouseAct; }
    void resetMouseInteraction()                      { mouse_interaction.reset();}
    MouseActionPtr getMouseInteraction()              { return mouse_interaction; }

    // Mouse mode handling.
    void updateUnderMouse(QPointF  spt);

    // Mouse interaction underway..
    void drawMouseInteraction(GeoGraphics * g2d);

    void setEditPlacedTile(PlacedTilePtr ptp) { editPlacedTile = ptp; }
    void resetEditPlacedTile()                { editPlacedTile.reset(); }

public slots:
    eViewType iamaLayer() override { return VIEW_TILING_MAKER; }
    void iamaLayerController() override {}

    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseTranslate(QPointF pt)      override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

    void slot_setCenter(QPointF spt)    override;
    void slot_wheel_scale(qreal delta)  override;
    void slot_wheel_rotate(qreal delta) override;

    void slot_scale(int amount)    override;
    void slot_rotate(int amount)   override;
    void slot_moveX(qreal amount)  override;
    void slot_moveY(qreal amount)  override;

    void  slot_setTileEditPoint(QPointF pt);

protected:
    void draw(GeoGraphics * g2d);
    void drawTiling(GeoGraphics * g2d, TilingPtr tiling);
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void determineOverlapsAndTouching(TilingPtr tiling);

    static constexpr QColor normal_color        = QColor(217,217,255,128);  // pale lilac
    static constexpr QColor in_tiling_color     = QColor(255,217,217,128);  // pink
    static constexpr QColor overlapping_color   = QColor(205,102, 25,128);  // ochre
    static constexpr QColor touching_color      = QColor( 25,102,205,128);  // blue
    static constexpr QColor under_mouse_color   = QColor(127,255,127,128);  // green
    static constexpr QColor selected_color      = QColor(  0,255,  0,128);
    static constexpr QColor construction_color  = QColor(  0,128,  0,128);
    static constexpr QColor drag_color          = QColor(206,179,102,128);
    static constexpr QColor circle_color        = QColor(202,200,  0,128);

private:
    wTilingPtr              wTiling;

    EdgePoly                wAccum;             // world points
    QVector<Measurement*>   wMeasurements;

    bool                    _hideTiling;
    bool                    _hideVectors;

    PlacedTileSelectorPtr   tileSelector;       // Current mouse selection.
    PlacedTilePtr           editPlacedTile;     // Tile in DlgTileEdit

    QColor                  lineColor;
    QPointF                 sMousePos;          // screen points DAC added
    QPointF                 tileEditPoint;
    QVector<QLineF>         constructionLines;

    MouseActionPtr          mouse_interaction;

    TilingMaker           * tilingMaker;

    bool                    debugMouse;
};

#endif
