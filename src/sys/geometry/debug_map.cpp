#include <QPainter>
#include "gui/viewers/debug_view.h"
#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"

using std::make_shared;

/////////////////////////////////////////
///
/// DebugMap - not really a map any more
/// All points are in model space
///
//////////////////////////////////////////

DebugMap::DebugMap()
{
    name = "DebugMap";
}

void DebugMap::set(const Map * map, QColor lineColor, QColor pointColor)
{
    wipeout();
    for (auto & edge : map->edges)
    {
        insertDebugEdge(edge,lineColor,pointColor,true);
    }
}

void DebugMap::paint(QPainter * painter, QTransform & tr)
{
    // set pen before calling this
    //qDebug() << "DebugMap::paint" <<  info() << Transform::info(tr);

    auto dv = Sys::debugView;

    if (dv->getShowLines())
    {
        for (auto & line : std::as_const(lines))
        {
            painter->setPen(QPen(line.color,line.width));
            QPointF p1 = tr.map(line.line.p1());
            QPointF p2 = tr.map(line.line.p2());
            painter->drawLine(p1,p2);
        }
    }

    if (dv->getShowDirection())
    {
        for (auto & line : std::as_const(lines))
        {
            painter->setPen(QPen(line.color,1));
            QLineF l = tr.map(line.line);
            GeoGraphics::drawLineArrowDirect(l,painter->pen(),painter);
        }
    }

    if (dv->getShowPoints())
    {
        qreal radius = 2.0;
        for (auto & pt : std::as_const(points))
        {
            painter->setPen(pt.color);
            painter->setBrush(pt.color);

            QPointF p = tr.map(pt.pt);
            painter->drawEllipse(p, radius, radius);
        }
    }

    if (Sys::debugView->getShowCurves())
    {
        for (auto & curve : curves)
        {
            painter->setPen(curve.color);
            QPointF v1     = tr.map(curve.pt1);
            QPointF v2     = tr.map(curve.pt2);
            QPointF center = tr.map(curve.mcenter);

            ArcData ad;
            if (curve.outer)
                ad.create(QLineF(v1,v2),center,curve.ctype);
            else
                ad.create(QLineF(v2,v1),center,curve.ctype);

            int start = qRound(ad.start() * 16.0);
            int span  = qRound(ad.span()  * 16.0);

            if (curve.outer)
                painter->drawArc(ad.rect(), start, span);
            else
                painter->drawArc(ad.rect(), start, -span);

            if (dv->getShowArcCentres())
            {
                if (curve.ctype == CURVE_CONCAVE)
                {
                    center = Geo::reflectPoint(center,QLineF(v2,v2));
                }
                qreal radius = 8.0;
                painter->save();
                painter->setPen(QPen(Qt::blue,3.0));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(center, radius, radius);
                painter->restore();
            }
        }
    }

    if (Sys::debugView->getShowCircles())
    {
        painter->save();
        painter->setBrush(QBrush());
        for (auto & circle : circles)
        {
            painter->setPen(QPen(circle.color,2));

            QPointF spt  = tr.map(circle.m_pt);
            qreal radius;
            if (circle.m_radius > 0)
                radius = Transform::scalex(tr) * circle.m_radius;
            else
                radius = 5;  // pixels
            painter->drawEllipse(spt,radius,radius);
        }
        painter->restore();
    }

    if (Sys::debugView->getShowMarks())
    {
        if (marks.size())
        {
            QFont font = painter->font();
            font.setPixelSize(14);
            painter->setFont(font);
            for (const cDebugMark & mark : std::as_const(marks))
            {
                painter->setPen(mark.mdata[0].first);

                QPointF pt  = mark.pt;
                qreal x     = pt.x();
                qreal y     = pt.y();

                qreal size = 0.05;
                if (mark.screenPts)
                    size = 5.0;

                QPointF p1(x-size,y);
                QPointF p2(x+size,y);
                QPointF p3(x,y+size);
                QPointF p4(x,y-size);

                if (!mark.screenPts)
                {
                    p1 = tr.map(p1);
                    p2 = tr.map(p2);
                    p3 = tr.map(p3);
                    p4 = tr.map(p4);
                    pt = tr.map(pt);
                }

                painter->drawLine(p1,p2);
                painter->drawLine(p3,p4);

                QString txt;
                for (const QPair<QColor,QString> & pair : mark.mdata)
                {
                    txt += pair.second + " ";
                }
                painter->drawText(QPointF(pt.x()+11,pt.y()+4),txt);
            }
        }
    }
}

void DebugMap::insertDebugMark(QPointF m, QString txt, QColor color, bool screenPts)
{
    if (!Sys::viewController->isEnabled(VIEW_DEBUG))
        return;

    for (auto & mark : marks)
    {
        if (mark.screenPts == screenPts && Loose::equalsPt(mark.pt,m))
        {
            if (!mark.mdata.contains(QPair<QColor,QString>(color,txt)))
                mark.mdata.push_back(QPair<QColor,QString>(color,txt));
            return;
        }
    }
    cDebugMark sdm;
    sdm.screenPts = screenPts;
    sdm.pt    = m;
    sdm.mdata.push_back(QPair<QColor,QString>(color,txt));
    marks.push_back(sdm);
}

void DebugMap::insertDebugPoint(QPointF pt, QColor color)
{
    if (!Sys::viewController->isEnabled(VIEW_DEBUG))
        return;

    sDebugPoint sdp;
    sdp.pt  = pt;
    sdp.color = color;
    points.push_back(sdp);
}

void DebugMap::insertDebugLine(QPointF p1, QPointF p2, QColor color, int width)
{
    insertDebugLine(QLineF(p1,p2),color,width);
}

void DebugMap::insertDebugLine(QLineF l1, QColor color, int width)
{
    if (!Sys::viewController->isEnabled(VIEW_DEBUG))
        return;

    sDebugLine sdl;
    sdl.line  = l1;
    sdl.color = color;
    sdl.width = width;
    lines.push_back(sdl);
}

void DebugMap::insertDebugEdge(EdgePtr edge, QColor lineColor, QColor pointColor, bool outer)
{
    if (!Sys::viewController->isEnabled(VIEW_DEBUG))
        return;

    if (edge->getType() == EDGETYPE_CURVE)
    {
        sDebugCurve sdc;
        sdc.color   = lineColor;
        sdc.pt1     = edge->v1->pt;
        sdc.pt2     = edge->v2->pt;
        sdc.ctype   = edge->getCurveType();
        sdc.mcenter = edge->getArcCenter();
        sdc.outer   = outer;
        curves.push_back(sdc);
    }
    else if (edge->getType() == EDGETYPE_LINE)
    {
        sDebugLine sdl;
        sdl.line  = edge->getLine();
        sdl.color = lineColor;
        lines.push_back(sdl);
    }

    insertDebugPoint(edge->v1->pt,pointColor);
    insertDebugPoint(edge->v2->pt,pointColor);
}


void DebugMap::insertDebugCircle(QPointF mpt, QColor color, qreal radius)
{
    cDebugCircle c;
    c.m_pt     = mpt;
    c.color    = color;
    c.m_radius = radius;
    circles.push_back(c);
}

void DebugMap::insertDebugCurve(QPointF mp1, QPointF mp2, eCurveType ctype, QPointF mArcCenter, QColor color, bool outer)
{
    sDebugCurve c;
    c.color   = color;
    c.pt1     = mp1;
    c.pt2     = mp2;
    c.mcenter = mArcCenter;
    c.ctype   = ctype;
    c.outer   = outer;
    curves.push_back(c);
}

QString DebugMap::info() const
{
    QString astring = QString("marks=%1 circles=%2 lines=%3 curves=%4 points=%5")
                          .arg(marks.count())
                          .arg(circles.count())
                          .arg(lines.count())
                          .arg(curves.count())
                          .arg(points.count());
    return astring;
}

QList<int> DebugMap::numInfo() const
{
    QList<int> res;
    res.resize(ROW_SIZE);

    res[ROW_MARKS]  = marks.count();
    res[ROW_CIRCS]  = circles.count();
    res[ROW_LINES]  = lines.count();
    res[ROW_CURVES] = curves.count();
    res[ROW_PTS]    = points.count();
    res[ROW_DIRN]   = res[ROW_LINES];   // same
    res[ROW_ARC_CEN]= res[ROW_CURVES];  // same
    return res;
}

void DebugMap::wipeout()
{
    marks.clear();
    circles.clear();
    curves.clear();
    lines.clear();
    points.clear();
}
