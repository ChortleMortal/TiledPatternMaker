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

#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"
#include "misc/unique_qvector.h"
#include "makers/tiling_maker/tiling_mouseactions.h"

class GeoGraphics;

typedef std::shared_ptr<class Tiling>       TilingPtr;
typedef std::shared_ptr<class TileSelector> TileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;

typedef QVector<PlacedTilePtr>              PlacedTiles;
typedef UniqueQVector<PlacedTilePtr>        UPlacedTiles;

Q_DECLARE_METATYPE(PlacedTilePtr)

class TilingMaker;

class TilingMakerView : public LayerController
{
    Q_OBJECT

public:
    static TilingMakerView * getInstance();
    static void             releaseInstance();

    void setMaker(TilingMaker * maker) { tilingMaker = maker;}
    void setupView(TilingPtr tiling);
    void clearViewData();
    void clearConstructionLines() { constructionLines.clear(); }

    virtual void paint(QPainter * painter) override;

    void drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool draw_c, QColor icol );
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);

    void hideTiling(bool state);

    TileSelectorPtr getTileSelector()   { return tileSelector; }
    void            resetTileSelector() { tileSelector.reset(); }

    TileSelectorPtr findSelection(QPointF spt);
    TileSelectorPtr findTile(QPointF spt);
    TileSelectorPtr findEdge(QPointF spt);
    TileSelectorPtr findPoint(QPointF spt);
    TileSelectorPtr findVertex(QPointF spt);
    TileSelectorPtr findMidPoint(QPointF spt);
    TileSelectorPtr findArcPoint(QPointF spt);

    TileSelectorPtr findTile(QPointF spt, TileSelectorPtr ignore);
    TileSelectorPtr findEdge(QPointF spt, TileSelectorPtr ignore );
    TileSelectorPtr findPoint(QPointF spt, TileSelectorPtr ignore);
    TileSelectorPtr findVertex(QPointF spt, TileSelectorPtr ignore);
    TileSelectorPtr findMidPoint(QPointF spt, TileSelectorPtr ignore);

    TileSelectorPtr findCenter(PlacedTilePtr feature, QPointF spt);
    QPointF         findSelectionPointOrPoint(QPointF spt);
    TileSelectorPtr findNearGridPoint(QPointF spt);

    PlacedTiles            & getAllTiles()      { return allPlacedTiles; }
    PlacedTiles            & getInTiling()      { return in_tiling; } // DAC was hash
    EdgePoly               & getAccumW()        { return wAccum; }
    QVector<Measurement*>  & getMeasurementsS() { return wMeasurements; }
    QVector<QLineF>    & getConstructionLines() { return constructionLines; }
    QPointF                  getMousePos()      { return sMousePos; }

    QLineF  getVisT1()         { return visibleT1; }
    QLineF  getVisT2()         { return visibleT2; }
    void    setVisT1(QLineF l) { visibleT1 = l; }
    void    setVisT2(QLineF l) { visibleT2 = l; }

    bool accumHasPoint(QPointF wpt);

    // Mouse tracking.
    TileSelectorPtr findTileUnderMouse();
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

    QString    getStatus();

public slots:
    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

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

    void  slot_setTileEditPoint(QPointF pt);

protected:
    void draw(GeoGraphics * g2d);
    void drawTiling(GeoGraphics * g2d);

    void determineOverlapsAndTouching();

    static constexpr QColor normal_color        = QColor(217,217,255,128);  // pale lilac
    static constexpr QColor in_tiling_color     = QColor(255,217,217,128);  // pink
    static constexpr QColor overlapping_color   = QColor(205,102, 25,128);  // ochre
    static constexpr QColor touching_color      = QColor( 25,102,205,128);  // blue
    static constexpr QColor under_mouse_color   = QColor(127,255,127,128);  // green
    static constexpr QColor selected_color      = QColor(  0,255,  0,128);
    static constexpr QColor construction_color  = QColor(  0,128,  0,128);
    static constexpr QColor drag_color          = QColor(206,179,102,128);
    static constexpr QColor circle_color        = QColor(202,200,  0,128);

    PlacedTiles             allPlacedTiles;
    PlacedTiles             in_tiling;

    EdgePoly                wAccum;             // world points
    QVector<Measurement*>   wMeasurements;

    bool                    _hideTiling;

    UPlacedTiles            overlapping;        // calculated DAC was hash
    UPlacedTiles            touching;           // calculated

    TileSelectorPtr         tileSelector;       // Current mouse selection.
    PlacedTilePtr           editPlacedTile;     // Tile in DlgTileEdit

    QLineF                  visibleT1;          // Translation vector so that the tiling tiles the plane.
    QLineF                  visibleT2;

    QPointF                 sMousePos;          // screen points DAC added
    QPointF                 tileEditPoint;
    QVector<QLineF>         constructionLines;

    MouseActionPtr          mouse_interaction;

private:
    TilingMakerView();
    ~TilingMakerView() override;

    static TilingMakerView  * mpThis;

    TilingMaker * tilingMaker;

    bool debugMouse;
};

#endif
