#ifndef THREAD_H
#define THREAD_H

#include <QtCore>
#include <QColor>
#include "base/colorset.h"

typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Thread>           ThreadPtr;


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
