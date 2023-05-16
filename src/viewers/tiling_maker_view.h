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
#include "enums/etilingmakermousemode.h"
#include "geometry/edgepoly.h"
#include "misc/unique_qvector.h"
#include "makers/tiling_maker/tiling_mouseactions.h"

class GeoGraphics;

typedef std::shared_ptr<class TileSelector> TileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;

typedef QVector<PlacedTilePtr>              PlacedTiles;
typedef UniqueQVector<PlacedTilePtr>        UPlacedTiles;

Q_DECLARE_METATYPE(PlacedTilePtr)

class TilingMakerView : public LayerController
{
public:
    TilingMakerView(class TilingMaker * maker);
    ~TilingMakerView() override;

    virtual void    paint(QPainter * painter) override;

    void drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool draw_c, QColor icol );
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);

    void hideTiling(bool state);

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

    QPointF           findSelectionPointOrPoint(QPointF spt);

    TileSelectorPtr findNearGridPoint(QPointF spt);

    PlacedTiles            & getAllTiles()      { return allPlacedTiles; }
    PlacedTiles            & getInTiling()      { return in_tiling; } // DAC was hash
    EdgePoly               & getAccumW()        { return wAccum; }
    QVector<Measurement*>  & getMeasurementsS() { return wMeasurements; }
    QVector<QLineF>    & getConstructionLines() { return constructionLines; }
    QPointF                  getMousePos()      { return sMousePos; }

    QLineF getVisT1() { return visibleT1; }
    QLineF getVisT2() { return visibleT2; }

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

    eTilingMakerMouseMode   tilingMakerMouseMode;     // set by tiling designer menu
    PlacedTiles             allPlacedTiles;
    PlacedTiles             in_tiling;

    EdgePoly                wAccum;             // world points
    QVector<Measurement*>   wMeasurements;

    bool                    _hideTiling;

    UPlacedTiles            overlapping;        // calculated DAC was hash
    UPlacedTiles            touching;           // calculated

    TileSelectorPtr         tileSelector;       // Current mouse selection.
    PlacedTilePtr           currentPlacedTile;  // current menu row selection too
    PlacedTilePtr           editPlacedTile;     // Tile in DlgTileEdit

    QLineF                  visibleT1;          // Translation vector so that the tiling tiles the plane.
    QLineF                  visibleT2;

    QPointF                 sMousePos;          // screen points DAC added
    QPointF                 tileEditPoint;
    QVector<QLineF>         constructionLines;

private:
    class TilingMaker *     tilingMaker;
};

#endif
