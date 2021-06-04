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
#include "makers/motif_maker/motif_maker.h"
#include "base/configuration.h"

PrototypeViewPtr PrototypeView::spThis;

PrototypeViewPtr PrototypeView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<PrototypeView>();
    }
    return spThis;
}

PrototypeView::PrototypeView() : Layer("ProtoFeatureView",LTYPE_VIEW)
{
    feature_interior = QColor(255, 217, 217, 127);
    feature_border   = QColor(140, 140, 140);
    layerPen         = QPen(QColor(20,150,210),3);
}

void PrototypeView::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawCenter(painter);
}

void PrototypeView::draw( GeoGraphics * gg )
{
    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);

    t1 = tiling->getTrans1();
    t2 = tiling->getTrans2();

    qDebug() << "ProtoFeatureView  proto="  << proto.get();
    for( auto placedFeature : tiling->getPlacedFeatures())
    {
        FeaturePtr feature  = placedFeature->getFeature();
        QTransform T        = placedFeature->getTransform();
        FigurePtr fig       = proto->getFigure(feature );

        PlacedDesignElement rpf(feature,fig,T);
        rpfs.push_back(rpf);
    }

    int mode = Layer::config->protoViewMode;
    if (mode & PROTO_DRAW_MAP)
    {
        MapPtr map = proto->getProtoMap();
        qDebug() << "ProtoFeatureView  proto="  << proto.get() << "protoMap" << map.get();

        for(auto& edge : map->getEdges())
        {
            edges.push_back(edge);
        }

        layerPen = QPen(QColor(20,150,210),3);
        edges.draw(gg, layerPen);
    }

    if (mode & (PROTO_DRAW_FEATURES | PROTO_DRAW_FIGURES))
    {
        fill(gg);
    }

    layerPen.setColor(Qt::yellow);
    for (auto placedDesignElement : rpfs)
    {
        QTransform T0 = placedDesignElement.getTransform();
        gg->pushAndCompose(T0);

        if (mode & PROTO_HIGHLIGHT_FEATURES)
        {
            layerPen.setColor(Qt::blue);
            QColor acolor(Qt::lightGray);
            acolor.setAlpha(64);
            ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(acolor),layerPen);
        }
        if  (mode & PROTO_HIGHLIGHT_FIGURES)
        {
            layerPen.setColor(Qt::yellow);
            ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),layerPen);
        }
        gg->pop();
    }

    if (mode & PROTO_DRAW_DESIGN_ELEMENT)
    {
        const QVector<PlacedFeaturePtr> & placed = tiling->getPlacedFeatures();
        QVector<DesignElementPtr> &  dels        = proto->getDesignElements();
        QVector<QTransform>       & tforms       = proto->getTranslations();

        qDebug() << "dels=" << dels.size() << "tforms="  << tforms.size();
        for (auto delp : dels)
        {
            FeaturePtr  feature = delp->getFeature();
            bool selected = (feature == vcontrol->getSelectedFeature());
            if (selected)
            {
                continue;       // paint these last
            }
            for (auto pfp : placed)
            {
                if (feature == pfp->getFeature())
                {
                    QTransform tr                  = pfp->getTransform();
                    PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                    drawPlacedDesignElement(gg, pdel, QPen(Qt::blue,3), QBrush(feature_interior), QPen(feature_border,3),selected);
                }
            }
        }
        for (auto delp : dels)
        {
            FeaturePtr  feature = delp->getFeature();
            bool selected = (feature == vcontrol->getSelectedFeature());
            if (!selected)
            {
                continue;   // already painted
            }
            for (auto pfp : placed)
            {
                if (feature == pfp->getFeature())
                {
                    QTransform tr                  = pfp->getTransform();
                    PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                    drawPlacedDesignElement(gg, pdel, QPen(Qt::blue,3), QBrush(feature_interior), QPen(feature_border,3),selected);
                }
            }
        }
    }
}

void PrototypeView::receive(GeoGraphics *gg, int h, int v )
{
    int mode = Layer::config->protoViewMode;

    for (auto placedDesignElement : qAsConst(rpfs))
    {
        QPointF pt    = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
        QTransform T0 = placedDesignElement.getTransform();
        QTransform T1 = QTransform::fromTranslate(pt.x(),pt.y());
        QTransform T2 = T0 * T1;

        gg->pushAndCompose(T2);

        if (mode & PROTO_DRAW_FEATURES)
        {
            ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(),QPen(feature_border,3));
        }

        if (mode & PROTO_DRAW_FIGURES)
        {
            ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),QPen(Qt::green,3));
        }

        gg->pop();
    }
}

void PrototypeView::drawPlacedDesignElement(GeoGraphics * gg, PlacedDesignElementPtr pde, QPen linePen, QBrush interiorBrush, QPen borderPen, bool selected)
{
    QTransform T = pde->getTransform();
    gg->pushAndCompose(T);

    FeaturePtr fp = pde->getFeature();
    QPen pen;
    if (selected)
        pen = QPen(Qt::red,3);
    else
        pen = borderPen;
    ViewerBase::drawFeature(gg,fp,interiorBrush,pen);

    // Draw the figure
    FigurePtr fig = pde->getFigure();
    ViewerBase::drawFigure(gg,fig,linePen);

    gg->pop();
}
