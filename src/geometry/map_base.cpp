#include <QPainter>
#include "geometry/map_base.h"
#include "geometry/edge.h"
#include "geometry/geo.h"
#include "misc/geo_graphics.h"


MapBase::MapBase() {}

void MapBase::paint(QPainter * painter, QTransform & tr,bool shoWDirn, bool showArcCenters, bool showVertices)
{
    //verify("figure", true, true, true);
    //qDebug() << "Map::paint" <<  namedSummary();
    for (auto & edge : std::as_const(edges))
    {
        QPointF p1 = tr.map(edge->v1->pt);
        QPointF p2 = tr.map(edge->v2->pt);

        //qDebug() << "edge" << p1 << p2;

        if (shoWDirn)
        {
            GeoGraphics::drawLineArrowDirect(QLineF(p1,p2),painter->pen(),painter);
        }

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF arcCenter = tr.map(edge->getArcCenter());
            ArcData ad;
            ad.create(p1,p2,arcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span() * 16.0));
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            QPointF arcCenter = tr.map(edge->getArcCenter());
            ArcData ad;
            ad.create(p1,p2,arcCenter,edge->isConvex());
            painter->drawChord(ad.rect, qRound(ad.start * 16.0),qRound(ad.span() * 16.0));
        }

        if (edge->isCurve() && showArcCenters)
        {
            QPointF pt = edge->getArcCenter();
            if (!edge->isConvex())
            {
                pt = Geo::reflectPoint(pt,edge->getLine());
            }
            qreal radius = 8.0;
            painter->save();
            painter->setPen(QPen(Qt::blue,3.0));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(tr.map(pt), radius, radius);
            painter->restore();
        }
    }

    if (showVertices)
    {
        qreal radius = 4.0;
        painter->setPen(QPen(Qt::blue,1));
        painter->setBrush(Qt::blue);
        for (const VertexPtr & v : std::as_const(vertices))
        {
            QPointF pos = tr.map(v->pt);
            painter->drawEllipse(pos, radius, radius);
        }
    }

    if (debugTexts.size())
    {
        QFont font = painter->font();
        font.setPixelSize(14);
        painter->setFont(font);
        for (const auto & pair : std::as_const(debugTexts))
        {
            QPointF pt  = pair.first;
            QString txt = pair.second;
#ifdef INVERT_VIEW
            painter->save();
            painter->scale(1.0, -1.0);
            QTransform t2 = _T * QTransform().scale(1.0,-1.0);
            pt = t2.map(pt);
            painter->drawText(QPointF(pt.x()+7,pt.y()+13),txt);
            painter->restore();
#else
            pt = tr.map(pt);
            painter->drawText(QPointF(pt.x()+7,pt.y()+13),txt);
#endif
        }
    }
}

// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void MapBase::transform(const QTransform & T)
{
    for (const auto & vert : std::as_const(vertices))
    {
        vert->setPt(T.map(vert->pt));
    }
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            QPointF pt = T.map(edge->getArcCenter());
            edge->setCurvedEdge(pt,edge->isConvex(),(edge->getType()==EDGETYPE_CHORD));
        }
    }
    for (auto & mark : debugTexts)
    {
        mark.first = T.map(mark.first);
    }
}

void MapBase::wipeout()
{
    // better to remove edges before removing vertices
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
    debugTexts.clear();
}

bool MapBase::isEmpty() const
{
    return  (vertices.size() < 2);
}
