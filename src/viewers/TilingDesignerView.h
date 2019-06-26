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

#ifndef FEATURE_VIEW_H
#define FEATURE_VIEW_H

#include "viewers/GeoGraphics.h"
#include "makers/TilingSelection.h"
#include "tile/PlacedFeature.h"
#include "base/layer.h"



class TilingDesignerView : public Layer
{
public:
    TilingDesignerView();

    virtual void    draw( GeoGraphics * gg) = 0;
    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    virtual QRectF  boundingRect() const Q_DECL_OVERRIDE;

    void drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol );

    void applyTransform(QPolygonF & poly, Transform T );

    TilingSelectionPtr findFeature(QPointF spt );
    TilingSelectionPtr findVertex(QPointF spt );
    TilingSelectionPtr findMidPoint(QPointF spt );
    TilingSelectionPtr findEdge(QPointF spt );

    TilingSelectionPtr findEdge(QPointF spt, TilingSelectionPtr ignore );
    TilingSelectionPtr findSelection(QPointF spt );

protected:
    QVector<PlacedFeaturePtr> placed_features;

    QColor       in_tiling_color;
    QColor       overlapping_color;
    QColor       touching_color;
    QColor       under_mouse_color;
    QColor       construction_color;
    QColor       normal_color;

private:
    class Canvas * canvas;
};

#endif
