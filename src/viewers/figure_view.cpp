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

#include "figure_view.h"
#include "base/configuration.h"
#include "tapp/design_element.h"
#include "tapp/figure.h"
#include "tapp/radial_figure.h"
#include "tapp/extended_star.h"
#include "tapp/extended_rosette.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/motif_maker/feature_button.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "geometry/transform.h"

FigureView::FigureView() : Layer("FigureView",LTYPE_VIEW)
{
    motifMaker = MotifMaker::getInstance();

    debugContacts = false;


    View * view = View::getInstance();
    _T = FeatureButton::resetViewport(-2,_dep,view->rect());
    qDebug().noquote() << "FigureView::transform" << Transform::toInfoString(_T);
}

FigureView::~FigureView()
{
    qDebug() << "FigureView destructor";
}

void FigureView::paint(QPainter *painter)
{
    qDebug() << "FigureView::paint";
    DesignElementPtr del = motifMaker->getSelectedDesignElement();
    if (!del)
    {
        qDebug() << "FigureView - no design element";
        return;
    }
    _dep    = del;
    _fig    = del->getFigure();     // for now just a single figure
    _feat   = del->getFeature();
    _fig->buildExtBoundary();
    qDebug() << "FigureView  this=" << this << "del:" << _dep.get() << "fig:" << _fig.get();

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    painter->translate(0,view->height());
    painter->scale(1.0, -1.0);

    View * view = View::getInstance();
    _T = FeatureButton::resetViewport(-2,_dep,view->rect());
    qDebug().noquote() << "FigureView::transform" << Transform::toInfoString(_T);

    // paint boundaries
    painter->setPen(QPen(Qt::magenta,2.0));
    paintFeatureBoundary(painter);

    painter->setPen(QPen(Qt::red,1.0));
    paintRadialFigureBoundary(painter);

    painter->setPen(QPen(Qt::yellow,3.0));
    paintExtendedBoundary(painter);

    // paint figure
    if (_fig->isRadial())
    {
        paintRadialFigureMap(painter, QPen(Qt::green, 2.0));
    }
    else
    {
        paintExplicitFigureMap(painter, QPen(Qt::green, 2.0));
    }

    drawCenter(painter);

    if (debugContacts)
    {
        painter->setPen(Qt::white);
        for( int idx = 0; idx < debugPts.size(); ++idx )
        {
            painter->drawLine( _T.map(debugPts[idx]), _T.map(debugPts[ (idx+1) % debugPts.size()]) );
        }

        painter->setPen(Qt::blue);
        for( int idx = 0; idx < debugContactPts.size(); ++idx )
        {
            contact * c = debugContactPts.at(idx);
            painter->drawLine(_T.map(c->other), _T.map(c->position));
        }
    }
}

void FigureView::paintExplicitFigureMap(QPainter *painter, QPen pen)
{
    qDebug() << "paintExplicitFigure" << _fig->getFigureDesc();

    MapPtr map = _fig->getFigureMap();
    //map->verifyMap( "paintExplicitFigure");
    paintMap(painter,map,pen);
}

void FigureView::paintRadialFigureMap(QPainter *painter, QPen pen)
{
    qDebug() << "paintRadialFigure" << _fig->getFigureDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(_fig);

    MapPtr map = rp->useBuiltMap();
    if (!map || map->isEmpty())
    {
        map = rp->getFigureMap();
    }

    paintMap(painter,map,pen);

    map = rp->useDebugMap();
    if (map)
    {
        paintMap(painter,map,QPen(Qt::white,1.0));
    }

    if (config->highlightUnit)
    {
        map = rp->useUnitMap();
        paintMap(painter,map,QPen(Qt::red,3.0));
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

    for (auto edge : map->getEdges())
    {
        QPointF p1 = _T.map(edge->getV1()->getPosition());
        QPointF p2 = _T.map(edge->getV2()->getPosition());

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF ArcCenter = _T.map(edge->getArcCenter());
            arcData ad = edge->calcArcData(p1,p2,ArcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span*16.0));
        }
    }

    for (auto stxt : map->texts)
    {
        painter->save();
        painter->scale(1.0, -1.0);
        QTransform t2 = _T * QTransform().scale(1.0,-1.0);
        QPointF pt = t2.map(stxt.pt);
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
        painter->restore();
    }
}

void FigureView::setDebugContacts(bool enb, QPolygonF pts, QVector<contact*> contacts)
{
    debugContacts   = enb;
    debugPts        = pts;
    debugContactPts = contacts;
}

