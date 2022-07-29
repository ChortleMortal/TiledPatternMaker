////////////////////////////////////////////////////////////////////////////
//
// FeatureView.java
//
// It's unlikely this file will get used in the applet.  A FeatureView
// is a special kind of GeoView that assumes a subclass will maintain
// a collection of Features.  It knows how to draw Features quickly,
// and provides a bunch of services to subclasses for mouse-based
// interaction with features.

#ifndef TILING_MAKER_VIEW_H
#define TILING_MAKER_VIEW_H

#include "misc/layer_controller.h"
#include "enums/etilingmakermousemode.h"
#include "geometry/edgepoly.h"
#include "misc/unique_qvector.h"
#include "makers/tiling_maker/tiling_mouseactions.h"

class GeoGraphics;


typedef std::shared_ptr<class TilingSelector>  TilingSelectorPtr;
typedef std::shared_ptr<class PlacedFeature>   PlacedFeaturePtr;

class TilingMakerView : public LayerController
{
public:
    TilingMakerView(class TilingMaker * maker);
    ~TilingMakerView() override;

    virtual void    paint(QPainter * painter) override;

    void drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol );
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);

    void hideTiling(bool state);

    TilingSelectorPtr findSelection(QPointF spt);
    TilingSelectorPtr findFeature(QPointF spt);
    TilingSelectorPtr findEdge(QPointF spt);
    TilingSelectorPtr findPoint(QPointF spt);
    TilingSelectorPtr findVertex(QPointF spt);
    TilingSelectorPtr findMidPoint(QPointF spt);
    TilingSelectorPtr findArcPoint(QPointF spt);

    TilingSelectorPtr findFeature(QPointF spt, TilingSelectorPtr ignore);
    TilingSelectorPtr findEdge(QPointF spt, TilingSelectorPtr ignore );
    TilingSelectorPtr findPoint(QPointF spt, TilingSelectorPtr ignore);
    TilingSelectorPtr findVertex(QPointF spt, TilingSelectorPtr ignore);
    TilingSelectorPtr findMidPoint(QPointF spt, TilingSelectorPtr ignore);

    TilingSelectorPtr findCenter(PlacedFeaturePtr feature, QPointF spt);

    QPointF           findSelectionPointOrPoint(QPointF spt);

    TilingSelectorPtr findNearGridPoint(QPointF spt);

    QVector<PlacedFeaturePtr> & getAllFeatures()   { return allPlacedFeatures; }
    QVector<PlacedFeaturePtr> & getInTiling()      { return in_tiling; } // DAC was hash
    EdgePoly                  & getAccumW()        { return wAccum; }
    QVector<Measurement*>     & getMeasurementsS() { return wMeasurements; }

    QPointF getMousePos() { return sMousePos; }
    QVector<QLineF> & getConstructionLines() { return constructionLines; }

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

    eTilingMakerMouseMode       tilingMakerMouseMode;     // set by tiling designer menu
    QVector<PlacedFeaturePtr>   allPlacedFeatures;
    QVector<PlacedFeaturePtr>   in_tiling;

    EdgePoly                    wAccum;       // world points
    QVector<Measurement*>       wMeasurements;

    bool                        _hideTiling;

    UniqueQVector<PlacedFeaturePtr>   overlapping;  // calculated DAC was hash
    UniqueQVector<PlacedFeaturePtr>   touching;     // calculated

    TilingSelectorPtr           featureSelector;        // Current mouse selection.
    PlacedFeaturePtr            currentPlacedFeature;   // current menu row selection too
    PlacedFeaturePtr            editPlacedFeature;      // Feature in DlgFeatureEdit

    QLineF                      visibleT1;                  // Translation vector so that the tiling tiles the plane.
    QLineF                      visibleT2;

    QPointF                     sMousePos;                  // screen points DAC added
    QPointF                     featureEditPoint;
    QVector<QLineF>             constructionLines;

private:
    class TilingMaker *         tilingMaker;
};

#endif
