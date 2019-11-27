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

#include "viewers/shapeviewer.h"
#include "base/configuration.h"
#include "designs/shapes.h"

ShapeViewer::ShapeViewer()
{
    config = Configuration::getInstance();

    javaCoords = false;
}

void ShapeViewer::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    TRANS;
    if (javaCoords)
    {
        painter->save();
        painter->scale(1.0,-1.0);
    }

    // polyforms
    QVector<Polyform*>::iterator it;
    Polyform * p;
    for (it = polyforms.begin(); it < polyforms.end(); it++)
    {
        p = *it;
        if (p->polytype == POLYGON2)
        {
            // turn off aliasing for polygons and polylines
            painter->setRenderHint(QPainter::Antialiasing ,_antiAliasPolys);
            painter->setRenderHint(QPainter::SmoothPixmapTransform,_antiAliasPolys);

            // the fill

            QPainterPath pp;
            pp.addPolygon(*p);
            painter->fillPath(pp,p->brush);

            // the edge
            painter->setBrush(Qt::NoBrush);

            painter->setPen(p->pen);
            painter->drawPolygon(*p);

            if (p->radials)
            {
                QPointF center(0.0,0.0);
                for (int i=0; i < p->count(); i++)
                {
                    painter->drawLine(center, p->at(i));
                }
            }
        }
        else if (p->polytype == POLYLINE2)
        {
            // turn off aliasing for polygons and polylines
            painter->setRenderHint(QPainter::Antialiasing ,_antiAliasPolys);
            painter->setRenderHint(QPainter::SmoothPixmapTransform,_antiAliasPolys);

            painter->setPen(p->pen);
            painter->drawPolyline(*p);
        }
        else if (p->polytype == CIRCLE2)
        {
            // anti-alias all curved shapes
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

            if (config->hideCircles)
            {
                painter->setPen(QPen(Qt::NoPen));
                painter->setBrush(QBrush(Qt::NoBrush));
            }
            else
            {
                painter->setPen(p->pen);
                painter->setBrush(p->brush);
            }

            if (p->arcSpan == 0 && p->arcStart == 0)
            {
                painter->drawEllipse(QPointF(),p->radius,p->radius);

            }
            else
            {
                if (p->arcType == ARC)
                {
                    painter->drawArc(-p->radius, -p->radius,
                                      p->diameter,p->diameter,
                                      p->arcStart,p->arcSpan);
                }
                else if (p->arcType == CHORD)
                {
                    painter->drawChord(-p->radius, -p->radius,
                                        p->diameter,p->diameter,
                                        p->arcStart,p->arcSpan);
                }
                else if (p->arcType == PIE)
                {
                    painter->drawPie(-p->radius, -p->radius,
                                      p->diameter,p->diameter,
                                      p->arcStart,p->arcSpan);
                }
            }

            if (config->circleX)
            {
                // draw inner X
                painter->setPen(QPen(Qt::green,1.0));
                painter->drawLine( - p->radius, 0.0, p->radius, 0.0);
                painter->drawLine(0.0,- p->radius, 0.0, p->radius);
            }

        }
    }

    // inner pen
    for (it = polyforms.begin(); it < polyforms.end(); it++)
    {
        p = *it;
        painter->setPen(p->innerPen);
        if (p->polytype == POLYGON2)
        {
            painter->drawPolygon(*p);
        }
        else if (p->polytype == POLYLINE2)
        {
            painter->drawPolyline(*p);
        }
    }


    // painter path
    if (!ppath.isEmpty())
    {
        // anti-alias all curved shapes
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        painter->setPen(ppPen);
        painter->setBrush(ppBrush);
        painter->drawPath(ppath);
        painter->fillPath(ppath,ppBrush);
    }

    if (javaCoords)
    {
        painter->restore();
    }
    UNTRANS;
}

QRectF ShapeViewer::boundingRect() const
{
    QRectF rect = _boundingBase;

    Polyform * p;
    int count = polyforms.count();
    for (int i=0; i < count; i++)
    {
        p = polyforms[i];
        rect = rect.united(p->boundingRect());
    }

    rect.translate(_loc);

    //qDebug() << "loc:" << _loc << "Base:" << _boundingBase2 << "All" << rect;

    return rect;
}

