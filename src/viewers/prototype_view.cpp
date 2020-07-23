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
// DesignPreview.java
//
// Boy, am I glad I thought of this class.  The design preview uses the
// same FillRegion algorithm as TilingViewer (used by TilingCard, for
// instance), to draw instead the _figures_ as they would appear in the
// final design.  Because we're not constructing the map, just drawing
// lines, previewing the design is really fast.  Plus, you can use the
// viewport of the preview window to indicate the region to fill for the
// final design -- I was searching for a way to let the user express
// this piece of information.
//
// This class just turns the translational unit into a collection of line
// segments and then draws them repeatedly to fill the window.

#include "viewers/workspace_viewer.h"
#include "viewers/prototype_view.h"
#include "viewers/viewerbase.h"

ProtoView::ProtoView(PrototypePtr proto) : Layer("ProtoView")
{
    qDebug() << "ProtoView::constructor";
    Q_ASSERT(proto);

    this->proto = proto;

    MapPtr map = proto->getProtoMap();

    for(auto edge : map->getEdges())
    {
        edges.push_back(edge);
    }

    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);

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

void ProtoView::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    qDebug() << "PrototypeView::paint"  << Transform::toInfoString(tr);
    GeoGraphics gg(painter,tr);

    draw(&gg);

    if (config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}

void ProtoView::draw( GeoGraphics * gg )
{
    layerPen = QPen(QColor(20,150,210),3);
    edges.draw(gg, layerPen);

    layerPen.setColor(Qt::yellow);
    for (auto placedDesignElement : rpfs)
    {
        QTransform T0 = placedDesignElement.getTransform();

        gg->pushAndCompose(T0);

        ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),layerPen);

        gg->pop();
    }
}
