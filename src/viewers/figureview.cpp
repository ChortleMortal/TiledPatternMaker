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

#include "figureview.h"
#include "base/configuration.h"
#include "tapp/DesignElement.h"
#include "tapp/Figure.h"
#include "tapp/RadialFigure.h"
#include "tapp/ExtendedStar.h"
#include "tapp/ExtendedRosette.h"
#include "makers/FeatureButton.h"
#include "base/utilities.h"
#include "geometry/Transform.h"

FigureView::FigureView(DesignElementPtr dep) : Layer("FigureView")
{
    Q_ASSERT(dep);

    _dep    = dep;
    _fig    = dep->getFigure();     // for now just a single figure
    _feat   = dep->getFeature();
    _fig->buildExtBoundary();

    qDebug() << "FigureView  this=" << Utils::addr(this) << "dep=" << Utils::addr(_dep.get()) << "fig=" << Utils::addr(_fig.get());
}

FigureView::~FigureView()
{
    qDebug() << "FigureView destructor";
}

void FigureView::receive(GeoGraphics * gg,int h, int v )
{
    Q_UNUSED(gg);
    Q_UNUSED(h);
    Q_UNUSED(v);
    qFatal("receive: not implemented");
}

void FigureView::draw( GeoGraphics * gg )
{
    Q_UNUSED(gg);
    qFatal("draw: not implemented");
}

void FigureView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qDebug() << "FigureView::paint";
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!_fig) return;

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QPointF center  = _feat->getCenter();
    QTransform t    = QTransform::fromTranslate(-center.x(),-center.y());
    QTransform t0   = getLayerTransform();
    QTransform t1   = QTransform::fromScale(8.0,8.0);
    _T = t * t1 * t0;
    qDebug().noquote() << "FigureView::" << Transform::toInfoString(_T);

    // paint boundaries
    painter->setPen(QPen(Qt::magenta,2.0));
    paintFeatureBoundary(painter);

    painter->setPen(QPen(Qt::red,1.0));
    paintRadialFigureBoundary(painter);

    painter->setPen(QPen(Qt::yellow,3.0));
    paintExtendedBoundary(painter);

    Configuration * config = Configuration::getInstance();
    if (config->boundingRects)
    {
        painter->setPen(QPen(Qt::green,1.0));
        QRectF rect = boundingRect();
        painter->drawRect(_T.mapRect(rect));
    }

    // paint figure
    if (_fig->isRadial())
    {
        paintRadialFigureMap(painter, QPen(Qt::green, 2.0));
    }
    else
    {
        paintExplicitFigureMap(painter, QPen(Qt::green, 2.0));
    }
}

void FigureView::paintExplicitFigureMap(QPainter *painter, QPen pen)
{
    qDebug() << "paintExplicitFigure" << _fig->getFigureDesc();

    MapPtr map = _fig->getFigureMap();
    map->verify( "paintExplicitFigure", false,true);
    paintMap(painter,map,pen);
}

void FigureView::paintRadialFigureMap(QPainter *painter, QPen pen)
{
    qDebug() << "paintRadialFigure" << _fig->getFigureDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(_fig);

    MapPtr map = rp->useBuiltMap();
    if (!map)
    {
        map = rp->getFigureMap();
    }

    paintMap(painter,map,pen);

    MapPtr dmap = rp->getDebugMap();
    if (dmap)
    {
        paintMap(painter,dmap,QPen(Qt::white,1.0));
    }
}

void FigureView::paintFeatureBoundary(QPainter *painter)
{
    // draw feature
    EdgePoly  ep = _feat->getEdgePoly();
    ep.paint(painter,_T);
}

void FigureView::paintRadialFigureBoundary(QPainter *painter)
{
    // show boundaries
    QPolygonF p1        = _fig->getRadialFigBoundary();
    painter->drawPolygon(_T.map(p1));
}

void FigureView::paintExtendedBoundary(QPainter *painter)
{
    QPolygonF p2        = _fig->getExtBoundary();
    bool drawCircle     = _fig->hasExtCircleBoundary();
    qreal boundaryScale = _fig->getExtBoundaryScale();

    QPointF center      = _feat->getCenter();
    QTransform ft       = QTransform::fromTranslate(center.x(), center.y());
    p2                  = ft.map(p2);

    if (!drawCircle)
    {
        painter->drawPolygon(_T.map(p2));
    }
    else
    {
        qreal scale = Transform::scalex(_T);
        painter->drawEllipse(_T.map(QPointF(0,0)),boundaryScale*scale,boundaryScale*scale);
    }
}

void FigureView::paintMap(QPainter * painter, MapPtr map, QPen pen)
{
    //map->verify("figure", true, true, true);

    painter->setPen(pen);

    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;

        QPointF p1 = _T.map(edge->getV1()->getPosition());
        QPointF p2 = _T.map(edge->getV2()->getPosition());

        if (edge->getType() == EDGE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGE_CURVE)
        {
            QPointF ArcCenter = _T.map(edge->getArcCenter());
            arcData ad = edge->calcArcData(p1,p2,ArcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span*16.0));
        }
    }

    if (map->texts.count())
    {
        for (auto t = map->texts.begin(); t != map->texts.end(); t++)
        {
            sText stxt = *t;
            QPointF pt = _T.map(stxt.pt);
            painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
        }
    }
}


