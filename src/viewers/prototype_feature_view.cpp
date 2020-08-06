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
#include "viewers/workspace_viewer.h"
#include "viewers/prototype_feature_view.h"
#include "viewers/viewerbase.h"

#include "geometry/point.h"

ProtoFeatureView::ProtoFeatureView(PrototypePtr proto) : Layer("ProtoFeatureView")
{
    qDebug() << "ProtoFeatureView::constructor";
    Q_ASSERT(proto);

    feature_interior = QColor(255, 217, 217, 127);
    feature_border   = QColor(140, 140, 140);

    this->proto = proto;

    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);

    t1 = tiling->getTrans1();
    t2 = tiling->getTrans2();

    for(auto placedFeature : tiling->getPlacedFeatures())
    {
        FeaturePtr feature  = placedFeature->getFeature();
        QTransform T        = placedFeature->getTransform();
        FigurePtr fig       = proto->getFigure(feature );

        PlacedDesignElement rpf(feature,fig,T);
        rpfs.push_back(rpf);
    }

    forceRedraw();
}

void ProtoFeatureView::paint(QPainter *painter)
{
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
    fill(gg);
}

void ProtoFeatureView::receive(GeoGraphics *gg, int h, int v )
{
    for (auto placedDesignElement : rpfs)
    {
        QPointF pt    = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
        QTransform T0 = placedDesignElement.getTransform();
        QTransform T1 = QTransform::fromTranslate(pt.x(),pt.y());
        QTransform T2 = T0 * T1;

        gg->pushAndCompose(T2);

        ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(feature_interior),QPen(feature_border,3));

        ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),QPen(Qt::green,3));

        gg->pop();
    }
}

