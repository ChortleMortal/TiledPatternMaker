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

#include "viewers/MapEditorView.h"
#include "base/canvas.h"
#include "base/utilities.h"
#include "base/view.h"
#include "geometry/Map.h"
#include "geometry/Point.h"
#include "geometry/Intersect.h"
#include "geometry/Transform.h"
#include "tapp/Figure.h"

MapEditorView::MapEditorView() : Layer("MapEditorView")
{
    hideConstructionLines = false;
    hideMap               = false;
    hidePoints            = false;
    hideMidPoints         = false;
    mapLineWidth          = 3.0;
    constructionLineWidth = 1.0;
    selectionWidth        = 3.0;
}

QRectF MapEditorView::boundingRect() const
{
    Canvas * canvas = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}

void MapEditorView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!map)
        return;

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform t;
    if (feap)
    {
        QPointF center = feap->getCenter();
        t              = QTransform::fromTranslate(-center.x(),-center.y());
    }
    QTransform t0 = getLayerTransform();
    QTransform t1 = QTransform::fromScale(8.0,8.0);  // TODO xform
    viewT         = t * t1;
    viewT         = viewT * t0;
    viewTinv      = viewT.inverted();

    qDebug() << "MapEditorView::paint viewT="  << Transform::toInfoString(viewT);

    draw(painter);

    if (config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}

void MapEditorView::drawMap(QPainter * painter)
{
    if (hideMap)
        return;

    painter->setPen(QPen(Qt::green,mapLineWidth ));
    for (auto edge : map->getEdges())
    {
        QPointF v1 = edge->getV1()->getPosition();
        QPointF v2 = edge->getV2()->getPosition();
        painter->drawLine(viewT.map(v1),viewT.map(v2));
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

    for (auto it = constructionLines.begin(); it != constructionLines.end(); it++)
    {
        QLineF line = *it;
        painter->setPen(QPen(Qt::white,constructionLineWidth));
        painter->drawLine(viewT.map(line));
    }
}

void MapEditorView::drawConstructionCircles(QPainter * painter)
{
    if (hideConstructionLines)
        return;

    for (auto it = constructionCircles.begin(); it != constructionCircles.end(); it++)
    {
        CirclePtr c = *it;
        QPointF pt = viewT.map(c->centre);
        painter->setPen(QPen(Qt::white,constructionLineWidth));
        painter->drawEllipse(pt, Transform::scalex(viewT) * c->radius, Transform::scalex(viewT)  * c->radius);

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
    delp.reset();
    prop.reset();
    styp.reset();

    figp.reset();
    feap.reset();
    map.reset();

    constructionLines.clear();
    constructionCircles.clear();

    inputMode = ME_INPUT_UNDEFINED;
}
