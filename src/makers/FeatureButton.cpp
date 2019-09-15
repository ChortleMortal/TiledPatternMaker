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

#include "FeatureButton.h"
#include "viewers/GeoGraphics.h"
#include "panels/panel_page.h"
#include "viewers/placeddesignelementview.h"
#include "base/utilities.h"
#include "geometry/Transform.h"

////////////////////////////////////////////////////////////////////////////
//
// FeatureButton.java
//
// A feature button is an (optionally clickable) GeoView that displays
// a DesignElement (perhaps DesignElementButton would have been a better
// name).  It draws the underlying feature with the figure overlaid.
// It includes some optimization for drawing radially symmetric figures
// without building the complete map.
//
// These buttons are meant to function like radio buttons -- they live
// in groups, and one is active at any given time.  If a button is active,
// it gets drawn with a red border.
//
// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

QColor FeatureButton::feature_border   = QColor( 140, 140, 160 );
QColor FeatureButton::feature_interior = QColor( 240, 240, 255 );

FeatureButton::FeatureButton(int index)
{
    construct(nullptr,index);
}

FeatureButton::FeatureButton(DesignElementPtr dep, int index)
{
    construct(dep,index);
}

FeatureButton::~FeatureButton()
{
    //qDebug() << "FeatureButton destructor";
}

void FeatureButton::construct(DesignElementPtr dep, int index)
{

    this->designElement = dep;
    if (dep)
        qDebug() << "FeatureButton dep=" << Utils::addr(dep.get())  << "fig=" << Utils::addr(dep->getFigure().get());
    else
        qDebug() << "FeatureButton dep=0";
    this->index = index;
    selected = false;
    setSize(300,300);
    setStyleSheet("background-color: white;");
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
}

QSizeF FeatureButton::getMinimumSize()
{
    return QSizeF(40, 40);
}

void FeatureButton::setSize(QSize d )
{
    setFixedSize(d);
    transform = resetViewport(designElement, frameRect());
}

void FeatureButton::setSize( int w, int h )
{
    setFixedSize(w,h);
    transform = resetViewport(designElement,frameRect());
}

DesignElementPtr FeatureButton::getDesignElement()
{
    return designElement;
}

void FeatureButton::setDesignElement(DesignElementPtr delp )
{
    if (delp == designElement)
    {
        return;
    }
    designElement = delp;
    if (designElement)
        qDebug() << "FeatureButton dep=" << Utils::addr(designElement.get())  << "fig=" << Utils::addr(designElement->getFigure().get());
    else
        qDebug() << "FeatureButton dep=0";
    designElementChanged();
}

void FeatureButton::designElementChanged()
{
    transform = resetViewport(designElement,frameRect());
    update();
}

QTransform FeatureButton::resetViewport(DesignElementPtr dep, QRect frameRect)
{
    //qDebug() << "reset viewport" << index;

    // Reset the viewport to look at the design element.
    // We can't do this until the Component is mapped and
    // the design element is set.  So we do it really lazily --
    // set a flag to reset the viewport whenever the current
    // DesignElement changes and recompute the viewport from
    // the paint function when the flag is set.

    QTransform t;

    if(!dep)
        return t;

    FigurePtr fig = dep->getFigure();
    if (!fig)
        return t;


    // Get the bounding box of all the figure's vertices and all the
    // feature's vertices.  Then show that region in the viewport.
    // In other words, scale the view to show the DesignElement.

    MapPtr map = fig->getFigureMap();
    QPolygonF pts = dep->getFeature()->getPoints();

    bool have_one = false;

    double xmin = 999.0;
    double xmax = -999.0;
    double ymin = 999.0;
    double ymax = -999.0;

    // This is a cheesy way to string together two streams of points.
    const QVector<VertexPtr> * e = map->getVertices();
    for (auto it = e->begin(); it != e->end(); it++)
    {
        VertexPtr vert = *it;
        QPointF p = vert->getPosition();

        // proc
        qreal x = p.x();
        qreal y = p.y();

        if( have_one )
        {
            xmin = std::min( xmin, x );
            xmax = std::max( xmax, x );

            ymin = std::min( ymin, y );
            ymax = std::max( ymax, y );
        }
        else
        {
            xmin = x;
            xmax = xmin;

            ymin = y;
            ymax = ymin;

            have_one = true;
        }
    }

    map = fig->getDebugMap();
    if (map)
    {
        e = map->getVertices();
        for (auto it = e->begin(); it != e->end(); it++)
        {
            VertexPtr vert = *it;
            QPointF p = vert->getPosition();

            // proc
            qreal x = p.x();
            qreal y = p.y();

            if( have_one )
            {
                xmin = std::min( xmin, x );
                xmax = std::max( xmax, x );

                ymin = std::min( ymin, y );
                ymax = std::max( ymax, y );
            }
        }
    }

    int pidx = 0;
    while ( pidx < pts.size())
    {
        QPointF p = pts[pidx++];

        // proc
        qreal x = p.x();
        qreal y = p.y();

        if( have_one )
        {
            xmin = std::min( xmin, x );
            xmax = std::max( xmax, x );

            ymin = std::min( ymin, y );
            ymax = std::max( ymax, y );
        }
        else
        {
            xmin = x;
            xmax = xmin;

            ymin = y;
            ymax = ymin;

            have_one = true;
        }
    }

    if( have_one )
    {
        //lookAt(QRectF(xmin, ymax, xmax-xmin, ymax-ymin));
        t = lookAt(QRectF(xmin, ymin, xmax-xmin, ymax-ymin),frameRect);
    }
    return t;
}

QTransform FeatureButton::centerInside(QRectF first, QRectF other)
{
/*
 * A useful routine when doing things like printing to
 * postscript.  Given two rectangles, find a good transform
 * for centering the first one inside the second one.
 */
    double xscale = other.width() / first.width();
    double yscale = other.height() / first.height();

    double scale = std::min(xscale, yscale);

    QTransform Ts = QTransform().scale(scale, scale);

    QPointF my_center   = first.center();
    QPointF your_center = other.center();

    // TODO xform
    QTransform your = QTransform::fromTranslate(your_center.x(),your_center.y());
    QTransform my   = QTransform::fromTranslate(-my_center.x(),-my_center.y());
    QTransform res  = my   * Ts * your;
    qDebug() << "res" << Transform::toInfoString(res);
    return my * Ts * your;
}

QTransform FeatureButton::lookAt(QRectF rect, QRect frameRect)
{
    // Leave some breathing room around the rectangle to look at.
    // Having the region of interest bleed right to edge of the
    // window doesn't look too good.

    //qDebug() << "lookAt:" << index << rect;

    QSizeF d         = rect.size();
    d *= 1.25;
    QPointF center   = rect.center();
    QRectF paintRect = QRectF(center.x()-(d.width()/2), center.y()-(d.height()/2), d.width(), d.height());

    QTransform t;
    if ( d.width() > 0 && d.height() > 0 )
    {
        t = centerInside(paintRect,frameRect);
        //forceRedraw();
    }
    return t;
}

void FeatureButton::setSelected( bool selected )
{
    if ( this->selected != selected )
    {
        this->selected = selected;
        //forceRedraw();
    }
}

void FeatureButton::paintEvent(QPaintEvent * event)
{
    if (!designElement)
    {
        QFrame::paintEvent(event);
        return;
    }

    //qDebug() << "FeatureButton::paintEvent";
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing ,true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);

    //qDebug().noquote() << "paint btn:" << index << transform.toString();
    GeoGraphics gg(&painter, transform);
    QTransform t;  // unity
    PlacedDesignElementPtr pdep = make_shared<PlacedDesignElement>(designElement,t);
    PlacedDesignElementView::drawPlacedDesignElement(&gg, pdep, QPen(Qt::black,3), QBrush(feature_interior), QPen(feature_border,3));

#if 0
    QString tempLabel;
    tempLabel = "del=" + Utils::addr(dep.get());
    painter.drawText(5,50,tempLabel);
    tempLabel = "fig=" + Utils::addr(dep->getFigure().get());
    painter.drawText(5,20,tempLabel);
#endif

    QFrame::paintEvent(event);
}


