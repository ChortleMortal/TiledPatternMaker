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

#include "base/configuration.h"
#include "base/canvas.h"
#include "viewers/ProtoFeatureView.h"
#include "viewers/placeddesignelementview.h"
#include "geometry/Point.h"

ProtoFeatureView::ProtoFeatureView(PrototypePtr proto) : Layer("ProtoFeatureView")
{
    Q_ASSERT(proto);
    qDebug() << "ProtoFeatureView::constructor";

    feature_interior = QColor(255, 217, 217, 127);
    feature_border   = QColor(140, 140, 140);

    pp = proto;
    //proto->walk();

    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);
    //tiling->dump();

    t1 = tiling->getTrans1();
    t2 = tiling->getTrans2();

    for(auto placedFeaturePtr : tiling->getPlacedFeatures())
    {
        FeaturePtr feature  = placedFeaturePtr->getFeature();
        QTransform T        = placedFeaturePtr->getTransform();
        FigurePtr fig       = proto->getFigure(feature );

        PlacedDesignElement rpf(feature,fig,T);
        rpfs.push_back(rpf);
    }

    forceRedraw();
}

QRectF ProtoFeatureView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void ProtoFeatureView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    qDebug() << "ProtoFeatureView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    if (Layer::config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}

void ProtoFeatureView::draw( GeoGraphics * gg )
{
    //gg->setColor(QColor(20,150,210));
    fill(gg, pp->getTiling()->getFillData());
}

void ProtoFeatureView::receive(GeoGraphics *gg, int h, int v )
{
    for (auto placedDesignElement : rpfs)
    {
        QPointF pt    = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
        QTransform T0 = placedDesignElement.getTransform();
        QTransform T1 = QTransform::fromTranslate(pt.x(),pt.y());
        QTransform T2 = T0 * T1;
        //qDebug() << "T" << Transform::toInfoString(T);

        PlacedDesignElementPtr pdep = make_shared<PlacedDesignElement>(placedDesignElement.getFeature(),
                                                                       placedDesignElement.getFigure(),
                                                                       T2);
        PlacedDesignElementView::drawPlacedDesignElement(gg,
                                                         pdep,
                                                         QPen(Qt::green,3),
                                                         QBrush(feature_interior),
                                                         QPen(feature_border,3));
    }
}

