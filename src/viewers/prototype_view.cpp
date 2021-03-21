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
#include "geometry/point.h"
#include "tile/placed_feature.h"
#include "viewers/prototype_view.h"
#include "viewers/viewcontrol.h"
#include "viewers/viewerbase.h"

PrototypeView::PrototypeView(PrototypePtr proto,int mode) : Layer("ProtoFeatureView",LTYPE_VIEW)
{
    qDebug() << "ProtoFeatureView::constructor";
    Q_ASSERT(proto);

    feature_interior = QColor(255, 217, 217, 127);
    feature_border   = QColor(140, 140, 140);
    layerPen         = QPen(QColor(20,150,210),3);

    this->proto = proto;
    this->mode  = mode;

    if (mode & PROTO_DRAW_MAP)
    {
        MapPtr map = proto->getProtoMap();

        for(auto& edge : map->edges)
        {
           edges.push_back(edge);
        }
    }

    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);

    t1 = tiling->getTrans1();
    t2 = tiling->getTrans2();

    if (mode & (PROTO_DRAW_FEATURES | PROTO_DRAW_FIGURES))
    {
        for(auto placedFeature : tiling->getPlacedFeatures())
        {
            FeaturePtr feature  = placedFeature->getFeature();
            QTransform T        = placedFeature->getTransform();
            FigurePtr fig       = proto->getFigure(feature );

            PlacedDesignElement rpf(feature,fig,T);
            rpfs.push_back(rpf);
        }
    }
    forceRedraw();
}

void PrototypeView::paint(QPainter *painter)
{
    qDebug() << "ProtoFeatureView::paint proto =" << proto.get();

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawCenter(painter);
}

void PrototypeView::draw( GeoGraphics * gg )
{
    if (mode & PROTO_DRAW_MAP)
    {
        layerPen = QPen(QColor(20,150,210),3);
        edges.draw(gg, layerPen);
    }

    if (mode & (PROTO_DRAW_FEATURES | PROTO_DRAW_FIGURES))
    {
        fill(gg);
    }
}

void PrototypeView::receive(GeoGraphics *gg, int h, int v )
{
    for (auto placedDesignElement : qAsConst(rpfs))
    {
        QPointF pt    = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
        QTransform T0 = placedDesignElement.getTransform();
        QTransform T1 = QTransform::fromTranslate(pt.x(),pt.y());
        QTransform T2 = T0 * T1;

        gg->pushAndCompose(T2);

        if (mode & PROTO_DRAW_FEATURES)
        {
            //ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(feature_interior),QPen(feature_border,3));
            ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(),QPen(feature_border,3));
        }

        if (mode & PROTO_DRAW_FIGURES)
        {
            ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),QPen(Qt::green,3));
        }

        ViewControl * vcontrol = ViewControl::getInstance();
        if (vcontrol->getSelectedFeature() == placedDesignElement.getFeature())
        {

        }
        gg->pop();
    }
}

