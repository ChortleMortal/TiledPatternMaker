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

// Casper: this is derived from the tapratty FeatureButton

// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

#include "viewers/designelementview.h"
#include "base/shared.h"
#include "base/workspace.h"
#include "viewers/GeoGraphics.h"
#include "base/configuration.h"
#include "base/canvas.h"


PlacedDesignElementView::PlacedDesignElementView(PlacedDesignElementPtr pde) : Layer("PlacedDesignElementView")
{
    qDebug() << "DesignElementView::constructor";
    Q_ASSERT(pde);

    feature_interior = QColor(240, 240, 255);
    feature_border   = QColor(140, 140, 160);

    this->pde = pde;
}

QRectF PlacedDesignElementView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void PlacedDesignElementView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qDebug() << "PlacedDesignElementView::paint:" << pde->toString();

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(QPen(Qt::black,3));

    Transform tr = *getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
}

void PlacedDesignElementView::draw(GeoGraphics * gg )
{
    // We could keep the figure's map cached until the figure changes.
    // On the other hand, it's really fast enough already.

    if( !pde->getFeature() || !pde->getFigure() )
    {
        qWarning() << "Can't draw empty design element";
        return;
    }

    drawPlacedDesignElement(gg, pde, QColor(Qt::green), feature_interior, feature_border);
}

void PlacedDesignElementView::drawPlacedDesignElement(GeoGraphics * gg, PlacedDesignElementPtr pde, QColor line, QColor interior, QColor border)
{
    Configuration * config = Configuration::getInstance();

    Transform T = pde->getTransform();
    gg->pushAndCompose(T);

    // Draw the feature.
    FeaturePtr fp = pde->getFeature();
    QPolygonF pts = fp->getPoints();
    gg->setColor(interior);
    gg->drawPolygon( pts, true );

    if (fp.get() == config->selectedDesignElementFeature)
        gg->setColor(Qt::red);
    else
        gg->setColor(border);
    gg->drawPolygon( pts, false );

    // Draw the figure
    gg->setColor(line);

    FigurePtr fig = pde->getFigure();
    if (!fig)
    {
        qWarning() << "DesignElementView::drawDesignElement - figure is NULL";
        gg->pop();
        return;
    }

    MapPtr map = fig->getFigureMap();
    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        QPointF p1 = edge->getV1()->getPosition();
        QPointF p2 = edge->getV2()->getPosition();

        gg->drawLine( p1, p2 );
    }
    gg->pop();
}
