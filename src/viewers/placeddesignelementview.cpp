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

#include "viewers/placeddesignelementview.h"
#include "viewers/GeoGraphics.h"
#include "base/shared.h"
#include "base/workspace.h"
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
    Q_UNUSED(option)
    Q_UNUSED(widget)
    qDebug() << "PlacedDesignElementView::paint:" << pde->toString();

    if(!pde->getFeature())
    {
        qWarning() << "Can't draw empty design element";
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);

    // We could keep the figure's map cached until the figure changes.
    // On the other hand, it's really fast enough already.
    drawPlacedDesignElement(&gg, pde, QPen(Qt::green,3), QBrush(feature_interior), QPen(feature_border,3));
}

void PlacedDesignElementView::drawPlacedDesignElement(GeoGraphics * gg, PlacedDesignElementPtr pde, QPen linePen, QBrush interiorBrush, QPen borderPen)
{
    Configuration * config = Configuration::getInstance();

    QTransform T = pde->getTransform();
    gg->pushAndCompose(T);

    // Draw the feature.
    FeaturePtr fp = pde->getFeature();
    QPolygonF pts = fp->getPoints();
    gg->drawPolygon(pts,linePen, interiorBrush);     // fills the feature

    QPen pen;
    if (fp.get() == config->selectedDesignElementFeature)
        pen = QPen(Qt::red,3);
    else
        pen = borderPen;
    EdgePoly ep = fp->getEdgePoly();
    ep.draw(gg,pen);        // outlines the feature

    // Draw the figure
    pen = linePen;
    FigurePtr fig = pde->getFigure();
    if (!fig)
    {
        qWarning() << "DesignElementView::drawDesignElement - figure is NULL";
        gg->pop();
        return;
    }

    MapPtr map = fig->getFigureMap();
    for(auto edge :  map->getEdges())
    {
        if (edge->getType() == EDGE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGE_CURVE)
        {
            gg->drawChord(edge->getV1()->getPosition(),edge->getV2()->getPosition(),edge->getArcCenter(),pen,QBrush(),edge->isConvex());
        }
    }
    gg->pop();
}
