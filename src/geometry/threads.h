#ifndef THREAD_H
#define THREAD_H

#include <QtCore>
#include <QColor>
#include "geometry/edge.h"
#include "base/colorset.h"

class Thread : public QVector<EdgePtr>
{
public:
    Thread();
    QColor color;
};


class Threads : public QVector<ThreadPtr>
{
public:
    Threads();
    void   findThreads(MapPtr map);
    void   assignColors(ColorSet & colors);

protected:
    void   findThread(ThreadPtr thread, MapPtr map, EdgePtr edge, VertexPtr touchPt);
};

#endif // THREAD_H
