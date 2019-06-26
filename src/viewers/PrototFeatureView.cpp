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
#include "viewers/designelementview.h"
#include "geometry/Point.h"

ProtoFeatureView::ProtoFeatureView(PrototypePtr proto) : Layer("ProtoFeatureView")
{
    Q_ASSERT(proto);
    qDebug() << "ProtoFeatureView::constructor";

    feature_interior = QColor(240, 240, 255);
    feature_border   = QColor(140, 140, 160);

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

        PlacedDesignElement rpf(fig,feature,T);
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
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qDebug() << "ProtoFeatureView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(QPen(Qt::black,3));

    Transform tr = *getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
}

void ProtoFeatureView::draw( GeoGraphics * gg )
{
    gg->setColor(QColor(20,150,210));
    fill(gg, pp->getTiling()->getFillData());
}

void ProtoFeatureView::receive(GeoGraphics *gg, int h, int v )
{
    for (int i=0; i < rpfs.size(); i++)
    {
        PlacedDesignElement & rpf = rpfs[i];

        Transform T0 = rpf.getTransform();
        Transform T  = Transform::translate((t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v))) ;
        T.composeD(T0);

        PlacedDesignElementPtr pdep = make_shared<PlacedDesignElement>( rpf.getFigure(), rpf.getFeature(), T);
        PlacedDesignElementView::drawPlacedDesignElement(gg, pdep,QColor(Qt::green), feature_interior, feature_border);
    }
}

