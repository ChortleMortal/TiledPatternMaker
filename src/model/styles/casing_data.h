#pragma once
#ifndef CASING_DATA_H
#define CASING_DATA_H

#include <QPointF>
#include <QDebug>
#include <QPainterPath>

#undef DEBUG_EDGES
#undef DEBUG_THREADS
#undef THREAD_LIMITS

class    CasingData
{
public:
    QPointF outer[2];
    QPointF mid[2];
    QPointF inner[2];
    int     edgeIndex;

    bool operator == (const CasingData & other) const
    {
        if (   outer[0]  != other.outer[0]
            || mid[0]    != other.mid[0]
            ||  inner[0] != other.inner[0] )
        {  qWarning ("s1 changed");  return false; }

        if (   outer[1]  != other.outer[1]
            || mid[1]    != other.mid[1]
            ||  inner[1] != other.inner[1] )
        {  qWarning ("s2 changed");  return false; }
        return true;
    }

    bool operator != (const CasingData & other) const { return !(*this == other); }

    void dumpDiffs(const CasingData & other)
    {
        if ( outer[0]  != other.outer[0])
            qDebug() <<  edgeIndex << "side1  outer changed " << outer[0] << other.outer[0];
        if (mid[0]    != other.mid[0])
            qDebug() << edgeIndex << "side1  mid changed"   <<  mid[0]   << other.mid[0];
        if (inner[0] != other.inner[0] )
            qDebug() <<  edgeIndex << "side1  inner changed" <<  inner[0] << other.inner[0];

        if ( outer[1]  != other.outer[1])
            qDebug() <<  edgeIndex << "side2  outer changed " << outer[1] << other.outer[1];
        if (mid[1]    != other.mid[1])
            qDebug() <<  edgeIndex << "side2  mid changed"   <<  mid[1]   << other.mid[1];
        if (inner[1] != other.inner[1] )
            qDebug() << edgeIndex << "side2  inner changed" <<  inner[1] << other.inner[1];
    }
};
#endif
