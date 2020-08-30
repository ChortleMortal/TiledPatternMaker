/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "viewers/geo_graphics.h"
#include "makers/tiling_maker/tiling_selection.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "tile/placed_feature.h"
#include "base/layer.h"
#include "base/configuration.h"

class TilingMakerView : public Layer
{
public:
    TilingMakerView();
    ~TilingMakerView() override;

    virtual void    paint(QPainter * painter) override;
    virtual void    draw( GeoGraphics * gg) = 0;

    void drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol );
    void drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end);
    void drawAccum(GeoGraphics * g2d);
    void drawMeasurements(GeoGraphics * g2d);

    TilingSelectionPtr findSelection(QPointF spt);
    TilingSelectionPtr findFeature(QPointF spt);
    TilingSelectionPtr findEdge(QPointF spt);
    TilingSelectionPtr findPoint(QPointF spt);
    TilingSelectionPtr findVertex(QPointF spt);
    TilingSelectionPtr findMidPoint(QPointF spt);
    TilingSelectionPtr findArcPoint(QPointF spt);

    TilingSelectionPtr findEdge(QPointF spt, TilingSelectionPtr ignore );
    TilingSelectionPtr findPoint(QPointF spt, TilingSelectionPtr ignore);
    TilingSelectionPtr findVertex(QPointF spt, TilingSelectionPtr ignore);
    TilingSelectionPtr findMidPoint(QPointF spt, TilingSelectionPtr ignore);

    TilingSelectionPtr findCenter(PlacedFeaturePtr feature, QPointF spt);

    QPointF            findSelectionPointOrPoint(QPointF spt);

    TilingSelectionPtr findNearGridPoint(QPointF spt);
    bool               nearGridPoint(QPointF spt, QPointF & foundGridPoint );

    QVector<PlacedFeaturePtr> & getAllFeatures()   { return allPlacedFeatures; }
    QVector<PlacedFeaturePtr> & getInTiling()      { return in_tiling; } // DAC was hash
    EdgePoly                  & getAccumW()        { return wAccum; }
    QVector<Measurement>      & getMeasurementsS() { return wMeasurements; }

protected:
    void determineOverlapsAndTouching();

    eTilingMouseMode  mouse_mode;     // set by tiling designer menu

    QVector<PlacedFeaturePtr> allPlacedFeatures;
    QVector<PlacedFeaturePtr> in_tiling;

    EdgePoly                  wAccum;       // world points
    QVector<Measurement>      wMeasurements;

    QColor       in_tiling_color;
    QColor       overlapping_color;
    QColor       touching_color;
    QColor       under_mouse_color;
    QColor       construction_color;
    QColor       normal_color;
    QColor       drag_color;

    bool         _hide;
    bool         _snapToGrid;

    QVector<PlacedFeaturePtr> overlapping;  // calculated DAC was hash
    QVector<PlacedFeaturePtr> touching;     // calculated
};

#endif
