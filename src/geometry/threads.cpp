#include <QDebug>

#include "geometry/threads.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"

extern void stackInfo();

void Threads::createThreads(Map * map)
{
    // reset visited
    for (auto & edge : std::as_const(map->getEdges()))
    {
        edge->thread.reset();
        edge->visited = false;
    }

    for (auto & edge : std::as_const(map->getEdges()))
    {
        if (edge->visited)
            continue;
        ThreadPtr thread = std::make_shared<Thread>();
        this->push_back(thread);
        createThread(map, thread,edge,edge->v2);
    }
    qDebug() << "interlace: num threads =" << size();
}

void Threads::createThread(Map * map, ThreadPtr thread, EdgePtr edge, VertexPtr touchPt)
{
    Q_ASSERT(thread);
    //static int count = 0;
    //qDebug().noquote() << count++ << "Threads::findThread Edge" << edge->dump() << "Thread" << thread.get();
    //stackInfo();

    edge->thread  = thread;
    edge->visited = true;
    thread->push_back(edge);

    // first pass looking for colinear
    NeighboursPtr n = map->getNeighbours(touchPt);
    for (auto & wedge : std::as_const(*n))
    {
        EdgePtr edge2 = wedge.lock();
        Q_ASSERT(edge2);
        //qDebug() << "Edge2" << map->edgeIndex(edge2.lock()) << "visited"  << edge2.lock()->visited;
        if (edge2->visited)
            continue;
        if (edge->isColinearAndTouching(edge2))
        {
            createThread(map,thread,edge2,edge2->getOtherV(touchPt));       // recurse
            return;
        }
    }

    // second pass looking for anything can make a turn
    auto edge3 = n->getFirstNonvisitedNeighbour(edge);
    if (edge3)
    {
        createThread(map,thread,edge3,edge3->getOtherV(touchPt));           // recurse
    }
}

void Threads::assignColors(ColorSet & colors)
{
    colors.resetIndex();
    QVector<ThreadPtr> & threads = *this;
    for (const auto & thread : std::as_const(threads))
    {
        thread->color = colors.getNextColor().color;
    }
}
