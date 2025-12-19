#pragma once
#ifndef DEBUG_MAP_H
#define DEBUG_MAP_H

#include <QColor>
#include <QPainter>
#include "sys/geometry/edge.h"

class Map;

class cDebugCircle
{
public:
    cDebugCircle() { m_radius = -1.0; }
    QColor  color;
    QPointF m_pt;       // model point
    qreal   m_radius;   // model radius
};

struct sDebugCurve
{
    QColor  color;
    QPointF pt1;
    QPointF pt2;
    QPointF mcenter;
    eCurveType ctype;
    bool    outer;
};

struct sDebugPoint
{
    QColor color;
    QPointF pt;
};

struct sDebugLine
{
    QLineF  line;
    QColor  color;
    int     width;  // pixels
};

class cDebugMark
{
public:
    cDebugMark(bool screenPts = false) { this->screenPts = screenPts; }
    QPointF pt;
    QVector<QPair<QColor,QString>> mdata;
    bool screenPts;
};

enum eDebugMapRows
{
    ROW_TOP,
    ROW_MARKS,
    ROW_PTS,
    ROW_CIRCS,
    ROW_LINES,
    ROW_DIRN,
    ROW_CURVES,
    ROW_ARC_CEN,
    ROW_BOT,
    ROW_SIZE
};

enum eDebugMapCols
{
    D_COL_CHK,
    D_COL_CREATE,
    D_COL_PAINT,
};

class DebugMap
{
public:
    DebugMap();

    void set (const Map * regularMap,QColor lineColor, QColor pointColor);

    void paint(QPainter * painter, QTransform & tr);

    void        insertDebugMark(QPointF m, QString txt, QColor color = Qt::black, bool screenPts = false);
    void        insertDebugPoint(QPointF pt, QColor color);
    void        insertDebugLine(QPointF p1, QPointF p2, QColor color, int width = 1);
    void        insertDebugLine(QLineF l1, QColor color, int width = 1);
    void        insertDebugCircle(QPointF mpt, QColor color, qreal radius = 0.0);
    void        insertDebugCurve(QPointF mp1, QPointF mp2, eCurveType ctype, QPointF mArcCenter, QColor color, bool outer);
    void        insertDebugEdge(EdgePtr edge, QColor lineColor, QColor pointColor, bool outer);        // and edge is a line and two points

    virtual void    wipeout();
    virtual QString info() const;
    QList<int>      numInfo() const;

protected:

private:
    QString                          name;
    QVector<cDebugMark>              marks;
    QVector<cDebugCircle>            circles;
    QVector<sDebugCurve>             curves;
    QVector<sDebugPoint>             points;
    QVector<sDebugLine>              lines;
};

#endif // DEBUG_MAP_H
