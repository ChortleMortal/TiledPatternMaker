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

FigureView::FigureView(DesignElementPtr dep) : Layer("FigureView")
{
    Q_ASSERT(dep);

    _dep    = dep;
    _fig    = dep->getFigure();     // for now just a single figure
    _fig->buildBoundary();

    qDebug() << "FigureView dep=" << Utils::addr(_dep.get()) << "fig=" << Utils::addr(_fig.get());

    canvas  = Canvas::getInstance();
}

QRectF FigureView::boundingRect() const
{
    return canvas->getCanvasSettings().getRectF();
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

    if (_fig == nullptr) return;

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    painter->setPen(_pen);

    FeaturePtr f        = _dep->getFeature();
    QPolygonF & feature = f->getPolygon();
    QRectF rect         = feature.boundingRect();
    QPointF center      = rect.center();

    Transform t     = Transform::translate(-center);
    TransformPtr t0 = getLayerTransform();
    Transform t1    = Transform::scale(8.0);
    _T = t0->compose(t1.compose(t));

    if (_fig->isRadial())
    {
        paintRadialFigure(painter);
    }
    else
    {
        paintExplicitFigure(painter);
    }

    paintBoundaries(painter);

    Configuration * config = Configuration::getInstance();
    if (config->boundingRects)
    {
        painter->setPen(QPen(Qt::green,1.0));
        QRectF rect = boundingRect();
        painter->drawRect(_T.apply(rect));
    }
}

void FigureView::paintExplicitFigure(QPainter *painter)
{
    qDebug() << "paintExplicitFigure" << _fig->getFigureDesc();

    MapPtr map = _fig->getFigureMap();
    paintMap(painter,map);
}

void FigureView::paintRadialFigure(QPainter *painter)
{
    qDebug() << "paintRadialFigure" << _fig->getFigureDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(_fig);

    MapPtr map = rp->useBuiltMap();
    if (!map)
    {
        map = rp->getFigureMap();
    }
    painter->setPen(QPen(Qt::green,1.0));
    paintMap(painter,map);

    MapPtr dmap = rp->getDebugMap();
    if (dmap)
    {
        painter->setPen(QPen(Qt::red,1.0));
        paintMap(painter,dmap);
    }
}

void FigureView::paintBoundaries(QPainter *painter)
{
    // draw feature
    FeaturePtr f = _dep->getFeature();
    QPolygonF & feature = f->getPolygon();
    painter->setPen(QPen(Qt::magenta,3.0));
    painter->drawPolygon(_T.apply(feature));

    if (!_fig)
    {
        return;
    }

    // show boundaries
    QPolygonF p1        = _fig->radialFigBoundary;
    painter->setPen(QPen(Qt::red,1.0));
    painter->drawPolygon(_T.apply(p1));

    QPolygonF p2        = _fig->extBoundary;
    bool drawCircle     = _fig->hasCircleBoundary;
    qreal boundaryScale = _fig->getExtBoundaryScale();
    painter->setPen(QPen(Qt::yellow,1.0));
    if (!drawCircle)
    {
        painter->drawPolygon(_T.apply(p2));
    }
    else
    {
        qreal scale = _T.scalex();
        painter->drawEllipse(_T.apply(QPointF(0,0)),boundaryScale*scale,boundaryScale*scale);
    }
}

void FigureView::paintMap(QPainter * painter, MapPtr map)
{
    //map->verify("figure", true, true, true);
    qDebug() << _T.toString();

    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        QPointF p1 = edge->getV1()->getPosition();
        QPointF p2 = edge->getV2()->getPosition();
        painter->drawLine(_T.apply(p1), _T.apply(p2));
    }

    if (map->texts.count())
    {
        for (auto t = map->texts.begin(); t != map->texts.end(); t++)
        {
            sText stxt = *t;
            QPointF pt = _T.apply(stxt.pt);
            painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
        }
    }
}
