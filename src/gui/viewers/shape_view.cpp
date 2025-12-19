#include <QPainter>
#include "gui/viewers/shape_view.h"
#include "legacy/shapes.h"

LegacyShapeViewer::LegacyShapeViewer() : Layer(VIEW_LEGACY,PRIMARY,"ShapeViewer")
{
    _antiAliasPolys  = true;
}

void LegacyShapeViewer::paint(QPainter *painter)
{
    //qDebug() << "ShapeViewer::paint" << this;

    painter->save();
    painter->translate(getLoc());
    qreal rot = getModelXform().getRotateDegrees();
    painter->rotate(rot);

    // polyforms
    for (const auto & p : std::as_const(polyforms))
    {
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

            if (Sys::hideCircles)
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
        }
    }

    // inner pen
    for (const auto & p : std::as_const(polyforms))
    {
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

    painter->restore();
}
