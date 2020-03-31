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
// TilingViewer.java
//
// TilingViewer gets a Tiling instance and displays the tiling in a
// view window.  It always draws as much of the tiling as necessary to
// fill the whole view area, using the modified polygon-filling algorithm
// in geometry.FillRegion.
//
// TilingViewer also has a test application that accepts the name of
// a built-in tiling on the command line or the specification of a tiling
// on System.in and displays that tiling.  Access it using
// 		java tile.TilingViewer

#include "base/workspace.h"
#include "viewers/TilingView.h"
#include "geometry/Point.h"
#include "base/configuration.h"
#include "base/canvas.h"

TilingView::TilingView(TilingPtr tiling) : Layer("TilingView")
{
    Q_ASSERT(tiling);
    this->tiling = tiling;
    forceRedraw();
}

QRectF TilingView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void TilingView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    qDebug() << "TilingView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    layerPen = QPen(Qt::red,3);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);  // DAC - draw goes to receive which goes to draw placed feature
                // DAC - receive is really 'draw one tile'

    if (Layer::config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}

void TilingView::draw(GeoGraphics *g2d )
{
    fill(g2d, tiling->getFillData());
}

// The FillRegion algorithm relies on a callback to send information
// about which translational units to draw.  Here, we get T1 and T2,
// build a transform and draw the PlacedFeatures in the translational
// unit at that location.
void TilingView::receive(GeoGraphics * gg, int h, int v )
{
    //qDebug() << "fill TilingView::receive:"  << h << v;
    QPointF   pt = (tiling->getTrans1() * static_cast<qreal>(h)) + (tiling->getTrans2() * static_cast<qreal>(v));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

    gg->pushAndCompose(T);

    for (auto it = tiling->getPlacedFeatures().begin(); it != tiling->getPlacedFeatures().end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        drawPlacedFeature(gg, pf);
    }

    gg->pop();
}

void TilingView::drawPlacedFeature(GeoGraphics * g2d, PlacedFeaturePtr pf)
{
    //qDebug().noquote() << "PlacedFeat:" << pf->getFeature().get() <<  "transform:" << Transform::toInfoString(t);

    FeaturePtr f  = pf->getFeature();
    EdgePoly ep   = pf->getPlacedEdgePoly();

    for (auto it = ep.begin(); it != ep.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGE_LINE)
        {
            g2d->drawLine(edge->getLine(),layerPen);
        }
        else if (edge->getType() == EDGE_CURVE)
        {
            g2d->drawChord(edge->getV1()->getPosition(),edge->getV2()->getPosition(),edge->getArcCenter(),layerPen,QBrush(),edge->isConvex());
        }
    }
}
