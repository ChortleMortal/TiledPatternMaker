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

#include "viewers/tilingmakerview.h"
#include "base/canvas.h"
#include "base/view.h"
#include "geometry/Transform.h"
#include "geometry/Point.h"

TilingMakerView::TilingMakerView() : Layer("TilingMakerView")
{
    config              = Configuration::getInstance();
#if 0
    normal_color        = QColor(217,217,255,230);  // pale lilac
    in_tiling_color     = QColor(255,217,217,230);  // pink
    overlapping_color   = QColor(205,102, 25,127);  // ochre
    touching_color      = QColor( 25,102,205,127);  // blue
    under_mouse_color   = QColor(127,255,127,127);  // green
    drag_color          = QColor(206,179,102,230);
#else
    normal_color        = QColor(217,217,255,127);  // pale lilac
    in_tiling_color     = QColor(255,217,217,127);  // pink
    overlapping_color   = QColor(205,102, 25,127);  // ochre
    touching_color      = QColor( 25,102,205,127);  // blue
    under_mouse_color   = QColor(127,255,127,127);  // green
    drag_color          = QColor(206,179,102,127);
#endif
    construction_color  = Qt::darkGreen;

    _hide               = false;
    _showOverlaps       = false;
}

TilingMakerView::~TilingMakerView()
{
    allPlacedFeatures.clear();
    in_tiling.clear();
}

void TilingMakerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    //qDebug() << "TilingMakerView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    layerPen = QPen(Qt::black,3);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    if (config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}



void TilingMakerView::drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end)
{
    qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
    qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);

    layerPen.setColor(construction_color);

    if (t1_start != t1_end)
    {
        g2d->drawLine(t1_start, t1_end, layerPen);
        g2d->drawArrow(t1_start, t1_end, arrow_length, arrow_width, layerPen, QBrush(construction_color));
        g2d->drawText(worldToScreen(t1_end) + QPointF(10,0),"T1");
    }

    if (t2_start != t2_end)
    {
        g2d->drawLine(t2_start, t2_end, layerPen);
        g2d->drawArrow(t2_start, t2_end, arrow_length, arrow_width, layerPen, QBrush(construction_color));
        g2d->drawText(worldToScreen(t2_end) + QPointF(10,0),"T2");
    }
}

void TilingMakerView::drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol )
{
    EdgePoly ep   = pf->getPlacedEdgePoly();
    QPolygonF pts = pf->getPlacedPolygon();

    // fill the polygon
    layerPen.setColor(icol);
    g2d->drawPolygon(pts,layerPen,QBrush(icol));

    // draw outline
    //g2d->drawPolygon( pts, false );
    layerPen.setColor(Qt::black);
    for (auto it = ep.begin(); it != ep.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGE_LINE)
        {
            g2d->drawLine(edge->getLine(),layerPen);
        }
        else if (edge->getType() == EDGE_CURVE)
        {
            g2d->drawChord(edge->getV1()->getPosition(),edge->getV2()->getPosition(),edge->getArcCenter(),layerPen,QBrush(icol),edge->isConvex());

            if (mouse_mode == CURVE_EDGE_MODE)
            {
                QPen apen = layerPen;
                apen.setColor(Qt::blue);
                g2d->drawCircle(edge->getArcCenter(),5,apen,QBrush());
            }
        }
    }

    // draw center circle
    if( draw_c )
    {
        QPointF center = Point::center(pts);
        layerPen.setColor(Qt::red);
        g2d->drawCircle(center,6,layerPen, QBrush());
    }
}

void TilingMakerView::drawAccum(GeoGraphics * g2d)
{
    if ( wAccum.size() == 0)
    {
        return;
    }

    layerPen.setColor(construction_color);
    QBrush brush(construction_color);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGE_LINE)
        {
            qDebug() << "draw accum edge";
            QPointF p1 = edge->getV1()->getPosition();
            QPointF p2 = edge->getV2()->getPosition();
            g2d->drawCircle(p1,6,layerPen,brush);
            g2d->drawCircle(p2,6,layerPen,brush);
            g2d->drawLine(p1, p2,layerPen);
        }
        else if (edge->getType() == EDGE_POINT)
        {
            qDebug() << "draw accum point";
            QPointF p = edge->getV1()->getPosition();
            g2d->drawCircle(p,6,layerPen,brush);
        }
    }
}

void TilingMakerView::drawMeasurements(GeoGraphics *g2d)
{
    layerPen.setColor(construction_color);
    for (auto it = wMeasurements.begin(); it != wMeasurements.end(); it++)
    {
        Measurement & mm = *it;
        g2d->drawLineDirect(mm.startS(), mm.endS(),layerPen);
        QString msg = QString("%1 (%2)").arg(QString::number(mm.lenS(),'f',2)).arg(QString::number(mm.lenW(),'f',8));
        g2d->drawText(mm.endS() + QPointF(10,0),msg);
    }
}

//
// Feature, edge and vertex finding.
//

TilingSelectionPtr TilingMakerView::findFeature(QPointF spt)
{
    TilingSelectionPtr sel;

    QPointF wpt = screenToWorld(spt);

    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        QPolygonF      pgon = pf->getPlacedPolygon();
        if (pgon.containsPoint(wpt,Qt::OddEvenFill))
        {
            return make_shared<TilingSelection>(INTERIOR,pf);
        }
    }
    return sel;
}

TilingSelectionPtr TilingMakerView::findVertex(QPointF spt)
{
    TilingSelectionPtr sel;
    return findVertex(spt,sel);
}

TilingSelectionPtr TilingMakerView::findVertex(QPointF spt,TilingSelectionPtr ignore)
{
    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        QPolygonF pgon = pf->getFeaturePolygon();
        QTransform   T = pf->getTransform();
        for( int v = 0; v < pgon.size(); ++v )
        {
            QPointF a = pgon[v];
            QPointF b = T.map(a);
            QPointF c = worldToScreen(b);
            if (Point::dist2(spt,c) < 49.0 )
            {
                return make_shared<TilingSelection>(VERTEX,pf,a);
            }
        }
    }

    TilingSelectionPtr sel;
    return sel;
}

TilingSelectionPtr TilingMakerView::findMidPoint(QPointF spt)
{
    TilingSelectionPtr sel;
    return findMidPoint(spt,sel);
}

TilingSelectionPtr TilingMakerView::findMidPoint(QPointF spt, TilingSelectionPtr ignore)
{
    TilingSelectionPtr sel;

    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        QTransform T = pf->getTransform();
        EdgePoly ep  = pf->getFeatureEdgePoly();
        for (auto edge : ep)
        {
            QPointF a    = edge->getV1()->getPosition();
            QPointF b    = edge->getV2()->getPosition();
            QPointF mid  = edge->getMidPoint();
            QPointF aa   = T.map(a);
            QPointF bb   = T.map(b);
            QPointF midd = T.map(mid);
            QPointF smid = worldToScreen(midd);
            if (Point::dist2(spt,smid) < 49.0)
            {
                // Avoid selecting middle point if end-points are too close together.
                QPointF a2 = worldToScreen(aa);
                QPointF b2 = worldToScreen(bb);
                qreal screenDist = Point::dist2(a2,b2);
                if ( screenDist < (7 * 7 * 7.0 * 7.0) )
                    return sel;

                return make_shared<TilingSelection>(MID_POINT, pf, edge, mid);
            }
        }
    }

    return sel;
}

TilingSelectionPtr TilingMakerView::findArcPoint(QPointF spt)
{
    TilingSelectionPtr sel;

    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        QTransform T   = pf->getTransform();
        EdgePoly epoly = pf->getFeatureEdgePoly();

        for(auto it = epoly.begin(); it != epoly.end(); it++)
        {
            EdgePtr ep   = *it;
            if (ep->getType() == EDGE_CURVE)
            {
                QPointF a    = ep->getArcCenter();
                QPointF aa   = T.map(a);
                QPointF aad  = worldToScreen(aa);
                if (Point::dist2(spt,aad) < 49.0)
                {
                    return make_shared<TilingSelection>(ARC_POINT, pf, ep, a);
                }
            }
        }
    }

    return sel;
}

TilingSelectionPtr TilingMakerView::findEdge(QPointF spt)
{
    TilingSelectionPtr sel;
    return findEdge(spt, sel);
}

TilingSelectionPtr TilingMakerView::findEdge(QPointF spt, TilingSelectionPtr ignore )
{
    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        EdgePoly epoly = pf->getFeatureEdgePoly();
        QTransform T   = pf->getTransform();

        for( int v = 0; v < epoly.size(); ++v)
        {
            EdgePtr edge  = epoly[v];
            QLineF  line  = edge->getLine();
            QLineF  lineW = T.map(line);
            QLineF  LineS = worldToScreen(lineW);

            if (Point::distToLine(spt, LineS) < 7.0)
            {
                return make_shared<TilingSelection>(EDGE,pf,edge);
            }
        }
    }

    TilingSelectionPtr sel;
    return sel;
}

TilingSelectionPtr TilingMakerView::findSelection(QPointF spt)
{
    TilingSelectionPtr sel;

    if (      (sel = findVertex(spt)) )
        return sel;
    else if ( (sel = findMidPoint(spt)) )
        return sel;
    else if ( (sel = findEdge(spt)) )
        return sel;
    else
    {
        TilingSelectionPtr sel2 = findFeature(spt);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedFeature(),spt);
            if (sel)
                return sel;
            else
                sel = sel2;
        }
    }
    return sel;
}

TilingSelectionPtr TilingMakerView::findPoint(QPointF spt)
{
    TilingSelectionPtr sel;
    return findPoint(spt,sel);
}

TilingSelectionPtr TilingMakerView::findPoint(QPointF spt, TilingSelectionPtr ignore)
{
    TilingSelectionPtr sel;

    if (      (sel = findVertex(spt,ignore)) )
        return sel;
    else if ( (sel = findMidPoint(spt,ignore)) )
        return sel;
    return sel;
}

QPointF TilingMakerView::findSelectionPointOrPoint(QPointF spt)
{
    TilingSelectionPtr sel = findPoint(spt);
    if (!sel)
    {
        return screenToWorld(spt);
    }

    return sel->getPlacedPoint();       // TODO - is this right or Model Point
}


TilingSelectionPtr TilingMakerView::findCenter(PlacedFeaturePtr pf, QPointF spt)
{
    QTransform T    = pf->getTransform();
    EdgePoly  epoly = pf->getFeatureEdgePoly();

    QPointF apt = Point::center(epoly);
    QPointF mpt = T.map(apt);
    QPointF wpt = worldToScreen(mpt);

    if (Point::dist2(spt,wpt) < 49.0)
    {
        return make_shared<TilingSelection>(FEAT_CENTER, pf, apt);
    }

    TilingSelectionPtr sel;
    return sel;
}

QRectF TilingMakerView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

////////////////////////////////////////////////////////////////////////////
//
// Validating that features don't overlap when tiled.
//
// Note: we're not very stringent to avoid flagging polygons that would
//       only slightly overlap. This is more about including completely
//       unnecessary polygons that cover area already covered by another
//       copy, or other gross errors that are hard to see (especially once
//       the plane if fully tiled!).
//
// casper - original is replaced with a more robust implementation which
// also distinguishes between touching and overlapping.

void TilingMakerView::determineOverlapsAndTouching()
{
    overlapping.clear();
    touching.clear();

    if (!_showOverlaps)
    {
        return;
    }

    for (auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        QPolygonF poly      = pf->getPlacedPolygon();

        for (auto it2 = allPlacedFeatures.begin(); it2 != allPlacedFeatures.end(); it2++)
        {
            PlacedFeaturePtr pf2 = *it2;
            if (pf2 == pf)
            {
                continue;
            }
            QPolygonF poly2 = pf2->getPlacedPolygon();
            if (poly2.intersects(poly))
            {
                QPolygonF p3 = poly2.intersected(poly);
                if (!p3.isEmpty())
                {
                    //qDebug() << "overlapping";
                    if (!overlapping.contains(pf))
                        overlapping.push_back(pf);
                    if (!overlapping.contains(pf2))
                        overlapping.push_back(pf2);
                }
                else
                {
                    //qDebug() << "touching";
                    if (!touching.contains(pf))
                        touching.push_back(pf);
                    if (!touching.contains(pf2))
                        touching.push_back(pf2);
                }
            }
        }
    }
}
