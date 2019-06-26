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

#include "FeatureViewPaint.h"
#include "tile/PlacedFeature.h"
#include"base/canvas.h"

FeatureViewPaint::FeatureViewPaint(PlacedFeaturePtr pfp) : Layer("FeatureViewPaint")
{
    placedFeature = pfp;
}

void FeatureViewPaint::paintFeature(QPainter * painter, const PlacedFeaturePtr pf, bool draw_circle, QColor icol , qreal radius)
{
    Transform  T = pf->getTransform();
    FeaturePtr f = pf->getFeature();
    QPolygonF  p = f->getPoints();

    transformPolygon(p,T,radius);

    painter->setPen(QPen(icol,3));
    painter->drawPolygon(p);

    painter->setBrush(Qt::blue);
    QPainterPath pp;
    pp.addPolygon(p);
    painter->fillPath(pp,painter->brush());

    if (draw_circle)
    {
        painter->setPen(QPen(Qt::red,1));
        qreal radius2 = T.distFromInvertedZero(6.0);
        QPointF center = p.boundingRect().center();
        painter->drawEllipse(center,radius2,radius2);
        qDebug() << "center="  << center;
        //tileCenter << center;
    }
}

void FeatureViewPaint::transformPolygon(QPolygonF & pgon, Transform &transform, qreal radius)
{
    for (int i = 0; i < pgon.size(); i++)
    {
        QPointF v = pgon[i];
        qreal x = v.x();
        qreal y = v.y();

        QPointF v2((transform.applyX(x,y)) * radius,-(transform.applyY(x,y)) * radius);
        pgon.replace(i,v2);
    }
}
QRectF FeatureViewPaint::boundingRect() const
{
    Canvas * canvas = Canvas:: getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void   FeatureViewPaint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qDebug() << "FeatureViewPaint::paint";

    TransformPtr tr = getLayerTransform();
    QPointF loc     = tr->trans();
    painter->translate(loc);
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(QPen(Qt::green,3));

    paintFeature(painter, placedFeature, true, Qt::red, 100.0);
}

void   FeatureViewPaint::receive(GeoGraphics * gg,int a1, int a2 )
{
    Q_UNUSED(gg);
    Q_UNUSED(a1);
    Q_UNUSED(a2);
    qFatal("receive: not implemented");
}

void   FeatureViewPaint::draw( GeoGraphics * gg )
{
    Q_UNUSED(gg);
    qFatal("draw: not implemented");
}
