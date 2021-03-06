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

#include "viewers/map_editor_view.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_selection.h"
#include "makers/motif_maker/feature_button.h"
#include "tapp/figure.h"
#include "tile/feature.h"
#include "viewers/view.h"
#include "geometry/circle.h"

MapEditorView::MapEditorView() : Layer("MapEditorView")
{
    hideConstructionLines = false;
    hideMap               = false;
    hidePoints            = false;
    hideMidPoints         = true;
    mapLineWidth          = 3.0;
    constructionLineWidth = 1.0;
    selectionWidth        = 3.0;
}

void MapEditorView::paint(QPainter *painter)
{

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);


    if (config->mapEditorMode == MAPED_MODE_FIGURE)
    {
#if 0
        QPointF center = feap->getCenter();
        QTransform t  = QTransform::fromTranslate(-center.x(),-center.y());
        QTransform t1 = QTransform::fromScale(8.0,8.0);  // TODO xform
        viewT         = t * t1;
        QTransform t0 = getLayerTransform();
        viewT         = viewT * t0;
#else
        View * view = View::getInstance();
        viewT = FeatureButton::resetViewport(-3,delp,view->rect());
#endif
    }
    else
    {
        viewT = getLayerTransform();
    }

    //qDebug() << "MapEditorView::paint viewT="  << Transform::toInfoString(viewT);
    viewTinv      = viewT.inverted();

    draw(painter);

    drawCenter(painter);
}

void MapEditorView::drawMap(QPainter * painter)
{
    if (hideMap)
        return;

    if (!map)
        return;

    painter->setPen(QPen(Qt::green,mapLineWidth ));
    for (auto edge : map->getEdges())
    {
        QPointF v1 = edge->v1->pt;
        QPointF v2 = edge->v2->pt;
        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(viewT.map(v1),viewT.map(v2));
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF v1 = viewT.map(edge->v1->pt);
            QPointF v2 = viewT.map(edge->v2->pt);
            QPointF ac = viewT.map(edge->getArcCenter());

            arcData ad = Edge::calcArcData(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawArc(ad.rect, start, span);
        }
    }
}

void MapEditorView::drawDCEL(QPainter * painter)
{
    if (hideMap)
        return;

    if (!dcel)
        return;

    painter->setPen(QPen(Qt::green,mapLineWidth ));

    for (auto edge : qAsConst(dcel->getEdges()))
    {
        QPointF v1 = edge->v1->pt;
        QPointF v2 = edge->v2->pt;
        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(viewT.map(v1),viewT.map(v2));
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF v1 = viewT.map(edge->v1->pt);
            QPointF v2 = viewT.map(edge->v2->pt);
            QPointF ac = viewT.map(edge->getArcCenter());

            arcData ad = Edge::calcArcData(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawArc(ad.rect, start, span);
        }
    }
}

void MapEditorView::drawCropMap(QPainter * painter)
{
    if (hideMap)
        return;

    if (cropRect)
    {
        switch(cropRect->getState())
        {
        case CROP_NONE:
        case CROP_APPLIED:
        case CROP_MASKED:
        case CROP_CONSTRUCTING:
        case CROP_EDITING:
        case CROP_COMPLETE:
            break;

        case CROP_PREPARED:
        case CROP_BORDER_PREPARED:
        case CROP_DEFINED:
        case CROP_BORDER_DEFINED:
        {
            painter->setPen(QPen(Qt::red,mapLineWidth ));
            painter->setBrush(Qt::NoBrush);

            QRectF rect = viewT.mapRect(cropRect->getRect());
            painter->drawRect(rect);
        }
            break;
        }
    }
}

void MapEditorView::drawFeature(QPainter * painter)
{
    QPolygonF pts = feap->getPoints();

        painter->setPen(QPen(Qt::magenta,1));
        painter->drawPolygon(viewT.map(pts)); // not filled
       }

void MapEditorView::drawBoundaries(QPainter *painter)
{
    Q_ASSERT(figp);
    figp->buildExtBoundary();

    QPolygonF & figure   = figp->getRadialFigBoundary();
    QPolygonF extBound   = figp->getExtBoundary();
    bool hasCircle       = figp->hasExtCircleBoundary();
    qreal  boundaryScale = figp->getExtBoundaryScale();

    // show boundaries
    painter->setPen(QPen(QColor(255,127,0),3));   // orange
    painter->drawPolygon(viewT.map(figure));

    painter->setPen(QPen(Qt::yellow,1));
    if (!hasCircle )
    {
        QPointF center = feap->getCenter();
        QTransform t   = QTransform::fromTranslate(center.x(), center.y());
        extBound       = t.map(extBound);
        painter->drawPolygon(viewT.map(extBound));
    }
    else
    {
        qreal scale = Transform::scalex(viewT);
        painter->drawEllipse(viewT.map(QPointF(0,0)),boundaryScale*scale,boundaryScale*scale);
    }
}

void MapEditorView::drawPoints(QPainter * painter,  QVector<pointInfo> & points)
{
    if (hidePoints)
        return;

    qreal radius = 1.0;
    for (auto it = points.begin(); it != points.end(); it++)
    {
        pointInfo & pi = *it;
        switch (pi._type)
        {
        case PT_VERTEX:
            if (hideMap) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::blue,1));
            painter->setBrush(Qt::blue);
            break;
        case PT_VERTEX_MID:
            if (hideMap) continue;
            if (hideMidPoints) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::darkBlue,1));
            painter->setBrush(Qt::darkBlue);
            break;
        case PT_LINE:
            if (hideConstructionLines) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::yellow,1));
            painter->setBrush(Qt::yellow);
            break;
        case PT_CIRCLE_1:
            if (hideConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::magenta,1));
            painter->setBrush(Qt::magenta);
            break;
        case PT_CIRCLE_2:
            if (hideConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::cyan,1));
            painter->setBrush(Qt::cyan);
            break;
        case PT_LINE_MID:
            if (hideConstructionLines) continue;
            if (hideMidPoints) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::darkYellow,1));
            painter->setBrush(Qt::darkYellow);
            break;
        }
        QPointF pos = viewT.map(pi._pt);
        painter->drawEllipse(pos, radius, radius);
    }
}

void MapEditorView::drawConstructionLines(QPainter * painter)
{
    if (hideConstructionLines)
        return;

    painter->setPen(QPen(Qt::white,constructionLineWidth));
    for (auto line : qAsConst(constructionLines))
    {
        painter->drawLine(viewT.map(line));
    }
}

void MapEditorView::drawConstructionCircles(QPainter * painter)
{
    if (hideConstructionLines)
        return;

    for (const auto &circle : qAsConst(constructionCircles))
    {
        QPointF pt = viewT.map(circle->centre);
        painter->setPen(QPen(Qt::white,constructionLineWidth));
        painter->drawEllipse(pt, Transform::scalex(viewT) * circle->radius, Transform::scalex(viewT)  * circle->radius);

        if (!hidePoints)
        {
            painter->setPen(QPen(Qt::red,constructionLineWidth));
            qreal len = 9.0;
            painter->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()));
            painter->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len));
        }
    }
}

void MapEditorView::unload()
{
    styp.reset();
    prototype.reset();
    delp.reset();
    tiling.reset();
    figp.reset();
    feap.reset();
    map.reset();
    dcel.reset();
    border.reset();

    constructionLines.clear();
    constructionCircles.clear();
    cropRect.reset();
}
