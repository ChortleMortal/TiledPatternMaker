﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#include "base/configuration.h"
#include "base/canvas.h"
#include "viewers/PrototypeView.h"
#include "geometry/Point.h"

ProtoView::ProtoView(PrototypePtr proto) : Layer("ProtoView")
{
    qDebug() << "ProtoView::constructor";
    Q_ASSERT(proto);

    pp = proto;
    //proto->walk();

    TilingPtr tiling = proto->getTiling();
    Q_ASSERT(tiling);
    //tiling->dump();

    t1 = tiling->getTrans1();
    t2 = tiling->getTrans2();

    for(auto i = tiling->getPlacedFeatures().begin(); i != tiling->getPlacedFeatures().end(); i++)
    {
        PlacedFeaturePtr pf = *i;
        FeaturePtr feature  = pf->getFeature();
        Transform T         = pf->getTransform();

        FigurePtr fig       = proto->getFigure(feature );
        if (!fig)
        {
            qWarning("Proto feature does not yet have a figuie");
            continue;
        }
        MapPtr map = fig->getFigureMap();
        for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
        {
            EdgePtr edge = *e;

            QPointF v1 = T.apply( edge->getV1()->getPosition() );
            QPointF v2 = T.apply( edge->getV2()->getPosition() );

            lines.push_back(QLineF(v1,v2));
        }
    }
    //qDebug() << "num lines = " << lines.size();
    forceRedraw();
}

QRectF ProtoView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void ProtoView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qDebug() << "PrototypeView::paint";

    int line_width = 3;
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(QPen(Qt::black,line_width));

    Transform tr = *getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
}

void ProtoView::draw( GeoGraphics * gg )
{
    gg->setColor(QColor(20,150,210));
    fill(gg, pp->getTiling()->getFillData());
}

void ProtoView::receive(GeoGraphics *gg, int h, int v )
{
    Transform T  = Transform::translate((t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v))) ;

    if (h==0 && v==0)
        gg->setColor(Qt::yellow);
    else
        gg->setColor(QColor(20,150,210));

    gg->pushAndCompose( T );

    for( int idx = 0; idx < lines.size(); ++idx )
    {
        QLineF ls = lines.at(idx);
        gg->drawLine(ls);
    }

    gg->pop();
}
