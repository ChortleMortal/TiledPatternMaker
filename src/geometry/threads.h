#pragma once
#ifndef THREAD_H
#define THREAD_H

#include <QColor>
#include "misc/colorset.h"

typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Thread>           ThreadPtr;

class Thread : public QVector<EdgePtr>
{
public:
    Thread() {}
    QColor color;
};


class Threads : public QVector<ThreadPtr>
{
public:
    Threads() {}

    void   createThreads(MapPtr map);
    void   assignColors(ColorSet & colors);

protected:
    void   createThread(ThreadPtr thread, EdgePtr edge, VertexPtr touchPt);

private:
    MapPtr map;
};

#endif // THREAD_H
