#include "geometry/threads.h"
#include "geometry/map.h"

Thread::Thread()
{

}

Threads::Threads()
{

}

void Threads::findThreads(MapPtr map)
{
    // reset visited
    for (auto edge : map->edges)
    {
        edge->thread.reset();
    }

    for (auto edge : map->edges)
    {
        if (edge->visited)
            continue;
        ThreadPtr thread = make_shared<Thread>();
        this->push_back(thread);
        findThread(thread,map,edge,edge->v2);
    }
    qDebug() << "interlace: num threads =" << size();
}

void Threads::findThread(ThreadPtr thread, MapPtr map, EdgePtr edge, VertexPtr touchPt)
{
    qDebug() << "Threads::findThread Edge" << edge->dump() << "Thread" << thread.get();

    edge->thread  = thread;
    edge->visited = true;
    thread->push_back(edge);

    // first pass looking for colinear
    for (auto edge2 : qAsConst(touchPt->getNeighbours()))
    {
        //qDebug() << "Edge2" << edge2->getTmpEdgeIndex() << "Thread" << edge2->getInterlaceInfo().threadNumber << "visited"  << edge2->getInterlaceInfo().visited;
        if (edge2->visited)
            continue;
        if (edge->isColinearAndTouching(edge2))
        {
            findThread(thread, map,edge2,edge2->getOtherV(touchPt));       // recurse
            return;
        }
    }

    // second pass looking for anything can make a turn
    auto edge3 = touchPt->getFirstNonvisitedNeighbour(edge);
    if (edge3)
    {
        findThread(thread, map,edge3,edge3->getOtherV(touchPt));           // recurse
    }
}

void Threads::assignColors(ColorSet & colors)
{
    colors.resetIndex();
    QVector<ThreadPtr> & threads = *this;
    for (const auto & thread : threads)
    {
        thread->color = colors.getNextColor().color;
    }
}
