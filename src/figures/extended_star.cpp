#include <QDebug>

#include "figures/extended_star.h"
#include "misc/utilities.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

ExtendedStar::ExtendedStar(const Figure & fig,
                           int nn, qreal d, int s,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(fig, nn, d, s)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

ExtendedStar::ExtendedStar(int nn, qreal d, int s,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(nn,d,s)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

void ExtendedStar::buildMaps()
{
    Star::buildMaps();

    if (extendPeripheralVertices)
    {
        extendPeripheralMap();
    }

    if (extendFreeVertices)
    {
        extendFreeMap();
    }
}

void ExtendedStar::extendFreeMap()
{
    if (n != getExtBoundarySides())
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QTransform t = QTransform::fromScale(getFigureScale(),getFigureScale());
    tip = t.map(tip);
    VertexPtr v1 = findVertex(tip);

    QPointF e_tip(1,0);
    QTransform t2 = QTransform::fromScale(getExtBoundaryScale(),getExtBoundaryScale());
    e_tip = t2.map(e_tip);
    VertexPtr v2 = figureMap->insertVertex(e_tip);

    figureMap->insertEdge(v1,v2);

    for (int idx = 1; idx < n; idx++)
    {
        tip   = Tr.map(tip);
        v1    = findVertex(tip);

        e_tip = Tr.map(e_tip);
        v2 = figureMap->insertVertex(e_tip);

        figureMap->insertEdge(v1,v2);
    }
}


void ExtendedStar::extendPeripheralMap()
{
    qDebug() << "ExtendedStar::extendFreeMap";

    qreal radius = getExtBoundaryScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    // extend the lines to exteneded boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    QVector<EdgePtr> ledges = figureMap->getEdges();    // local copy of vector
    for (auto & edge : ledges)
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1 = QLineF(v2->pt,v1->pt);

        bool extend = false;
        QPointF intersect;
        if (!Point::intersectPoly(l1,getRadialFigBoundary(),intersect))
        {
            // point doesn't touch the figure boundary
            if (extendFreeVertices)
            {
                extend = true;
            }
        }
        else
        {
            // point does touch the figure boundary
            if (extendPeripheralVertices)
            {
                extend = true;
            }
        }

        if (extend)
        {
            if (!hasExtCircleBoundary())
            {
                // dont extend lines which already touch boundary
                if (!Point::intersectPoly(l1,getExtBoundary(),intersect))
                {
                    // extend lines
                    QLineF l2 = Point::extendLine(l1,10.0);      // extends
                    l2        = Point::clipLine(l2,getExtBoundary()); // clips
                    // insert Vertex and Insert edge
                    VertexPtr newv = figureMap->insertVertex(l2.p2()); // outer
                    figureMap->insertEdge(newv,v1);
                    //qDebug() << "extension" << newv->pt << v1->pt;
                    //figureMap->verify("Extended figure - mid",true,true,false);
                }
            }
            else
            {
                QPointF a;
                QPointF b;
                int points = Utils::circleLineIntersectionPoints(circle,radius,l1,a,b);
                if (points)
                {
                    VertexPtr newv;
                    if (!a.isNull())
                    {
                        newv = figureMap->insertVertex(a);
                        figureMap->insertEdge(newv,v1);
                    }
                    if (!b.isNull())
                    {
                        newv = figureMap->insertVertex(b);
                        figureMap->insertEdge(newv,v1);
                    }
                }
            }
        }
    }

    figureMap->verify();
}

VertexPtr ExtendedStar::findVertex(QPointF pt)
{
    const QVector<VertexPtr> & vertices = figureMap->getVertices();
    for (auto v : vertices)
    {
        if (Loose::equalsPt(v->pt,pt))
        {
            return v;
        }
    }
    VertexPtr vp;
    return vp;
}
