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

#include "TilingDesignerView.h"
#include "base/canvas.h"
#include "base/view.h"

TilingDesignerView::TilingDesignerView() : Layer("TilingDesignerView")
{
    normal_color        = QColor(217,217,255,230);
    in_tiling_color     = QColor(255,217,217    );
    overlapping_color   = QColor(205,102, 25,127);
    touching_color      = QColor( 25,102,205,127);
    under_mouse_color   = QColor(127,255,127,154);
    construction_color  = Qt::darkGreen;
}


void TilingDesignerView::drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol )
{
    Transform T = pf->getTransform();
    //qDebug().noquote() << T.toString();
    g2d->pushAndCompose(T);

    FeaturePtr f = pf->getFeature();
    QPolygonF pts = f->getPoints();

    g2d->setColor( icol );
    g2d->drawPolygon( pts, true );
    g2d->setColor( Qt::black);
    g2d->drawPolygon( pts, false );

    if( draw_c )
    {
        g2d->setColor(Qt::red );
        qreal radius = g2d->getTransform().distFromInvertedZero( 6.0 );
        g2d->drawCircle( Point::center( pts ), radius );
        //QPointF center = pts.boundingRect().center();
        //g2d->drawCircle(center, radius );
    }

    g2d->pop();
}


// Feature, edge and vertex finding.

void TilingDesignerView::applyTransform( QPolygonF & poly, Transform T )
{
    for( int idx = 0; idx < poly.size(); idx++ )
    {
        poly[ idx ] = T.apply( poly[ idx ] );
    }
}

TilingSelectionPtr TilingDesignerView::findFeature(QPointF spt)
{
    TilingSelectionPtr sel;

    QPointF wpt = screenToWorld(spt);

    // Of course, find receivers in reverse order from draw order.
    // Much more intuitive this way.
    for(auto it = placed_features.rbegin(); it != placed_features.rend(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        Transform T         = pf->getTransform();
        FeaturePtr f        = pf->getFeature();
        QPolygonF pgon      = f->getPolygon();

        applyTransform(pgon,T);

        if (pgon.containsPoint(wpt,Qt::OddEvenFill))
        {
            return make_shared<TilingSelection>(INTERIOR,pf);
        }
    }

    return sel;
}

TilingSelectionPtr TilingDesignerView::findVertex(QPointF spt)
{
    // Of course, find receivers in reverse order from draw order.
    // Much more intuitive this way.
    for(auto it = placed_features.rbegin(); it != placed_features.rend(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        Transform T         = pf->getTransform();
        Feature f           = pf->getFeature();
        QPolygonF & pgon = f.getPolygon();

        for( int v = 0; v < pgon.size(); ++v )
        {
            QPointF a  = T.apply(pgon[v]);
            QPointF sa = worldToScreen(a);

            if (Point::dist2(spt,sa) < 49.0 )
            {
                return make_shared<TilingSelection>(VERTEX,pf,a);
            }
        }
    }

    TilingSelectionPtr sel;
    return sel;
}

TilingSelectionPtr TilingDesignerView::findMidPoint(QPointF spt)
{
    TilingSelectionPtr sel;

    // Of course, find receivers in reverse order from draw order.
    // Much more intuitive this way.
    for(auto it = placed_features.rbegin(); it != placed_features.rend(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        Transform T         = pf->getTransform();
        FeaturePtr f        = pf->getFeature();
        QPolygonF pgon      = f->getPolygon();

        int vertCount = pgon.size();
        for( int v = 0; v < vertCount; ++v )
        {
            QPointF a    = T.apply(pgon[v]);
            QPointF b    = T.apply(pgon[(v+1) % vertCount]);
            QPointF mid  = Point::convexSum(a, b, 0.5);
            QPointF smid = worldToScreen(mid);
            if (Point::dist2(spt,smid) < 49.0)
            {
                // Avoid selecting middle point if end-points are too close together.
                QPointF aa = worldToScreen(a);
                QPointF bb = worldToScreen(b);
                qreal screenDist = Point::dist2(aa,bb);
                if ( screenDist < (7 * 7 * 7.0 * 7.0) )
                    return sel;

                return make_shared<TilingSelection>(MID_POINT, pf, mid);
            }
        }
    }

    return sel;
}

TilingSelectionPtr TilingDesignerView::findEdge(QPointF spt)
{
    TilingSelectionPtr sel;
    return findEdge(spt, sel);
}

TilingSelectionPtr TilingDesignerView::findEdge(QPointF spt, TilingSelectionPtr ignore )
{
    // Of course, find receivers in reverse order from draw order.
    // Much more intuitive this way.
    for(auto it = placed_features.rbegin(); it != placed_features.rend(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getFeature() == pf))
            continue;

        Transform T    = pf->getTransform();
        Feature f      = pf->getFeature();
        QPolygonF pgon = f.getPolygon();

        for( int v = 0; v < pgon.size(); ++v)
        {
            QPointF a = T.apply( pgon[v]);
            QPointF b = T.apply( pgon[(v+1) % pgon.size()]);
            QPointF sa = worldToScreen( a );
            QPointF sb = worldToScreen( b );

            if (Point::distToLine(spt, sa, sb) < 7.0)
            {
                return make_shared<TilingSelection>(EDGE,pf,QLineF(a,b));
            }
        }
    }

    TilingSelectionPtr sel;
    return sel;
}

TilingSelectionPtr TilingDesignerView::findSelection(QPointF spt)
{
    TilingSelectionPtr sel;

    if (      (sel = findVertex(spt)) )
        return sel;
    else if ( (sel = findMidPoint(spt)) )
        return sel;
    else if ( (sel = findEdge(spt)) )
        return sel;
    else if ( (sel = findFeature(spt)) )
        return sel;

    return sel;
}

QRectF TilingDesignerView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void TilingDesignerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    //qDebug() << "TilingDesignerView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(QPen(Qt::black,3));

    Transform tr = *getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
}
