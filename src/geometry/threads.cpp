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
    for (auto edge : map->getEdges())
    {
        InterlaceInfo & ii = edge->getInterlaceInfo();
        ii.initThread();
    }

    for (auto edge : map->getEdges())
    {
        if (edge->getInterlaceInfo().visited)
            continue;
        ThreadPtr thread = make_shared<Thread>();
        this->push_back(thread);
        findThread(thread,map,edge,edge->getV2());
    }
    qDebug() << "interlace: num threads =" << size();
}

void Threads::findThread(ThreadPtr thread, MapPtr map, EdgePtr edge, VertexPtr touchPt)
{
    qDebug() << "Threads::findThread Edge" << edge->getTmpEdgeIndex() << "Thread" << thread.get();

    edge->getInterlaceInfo().thread  = thread;
    edge->getInterlaceInfo().visited = true;
    thread->push_back(edge);

    NeighboursPtr nbs = map->getNeighbourMap().getNeighbours(touchPt);

    // first pass looking for colinear
    for (auto edge2 : nbs->getNeighbours())
    {
        //qDebug() << "Edge2" << edge2->getTmpEdgeIndex() << "Thread" << edge2->getInterlaceInfo().threadNumber << "visited"  << edge2->getInterlaceInfo().visited;
        if (edge2->getInterlaceInfo().visited)
            continue;
        if (edge->isColinearAndTouching(edge2))
        {
            findThread(thread, map,edge2,edge2->getOtherV(touchPt));       // recurse
            return;
        }
    }

    // second pass looking for anything can make a turn
    auto edge3 = nbs->getFirstNonvisitedNeighbour(edge);
    if (edge3)
    {
        findThread(thread, map,edge3,edge3->getOtherV(touchPt));           // recurse
    }
}

void Threads::assignColors(ColorSet & colors)
{
    colors.resetIndex();
    QVector<ThreadPtr> & threads = *this;
    for (auto thread : threads)
    {
        thread->color = colors.getNextColor().color;
    }
}
