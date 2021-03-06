#include "geometry/threads.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"

extern void stackInfo();

Thread::Thread()
{

}

Threads::Threads()
{

}

void Threads::findThreads(MapPtr map)
{
    // reset visited
    for (auto & edge : map->getEdges())
    {
        edge->thread.reset();
    }

    for (auto & edge : map->getEdges())
    {
        if (edge->visited)
            continue;
        ThreadPtr thread = std::make_shared<Thread>();
        this->push_back(thread);
        findThread(thread,map,edge,edge->v2);
    }
    qDebug() << "interlace: num threads =" << size();
}

void Threads::findThread(ThreadPtr thread, MapPtr map, EdgePtr edge, VertexPtr touchPt)
{
    //static int count = 0;
    //qDebug().noquote() << count++ << "Threads::findThread Edge" << edge->dump() << "Thread" << thread.get();
    //stackInfo();

    edge->thread  = thread;
    edge->visited = true;
    thread->push_back(edge);

    // first pass looking for colinear
    NeighboursPtr n = map->getBuiltNeighbours(touchPt);
    std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
    for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
    {
        WeakEdgePtr wedge = *pos;
        EdgePtr edge2 = wedge.lock();
        Q_ASSERT(edge2);
        //qDebug() << "Edge2" << map->edgeIndex(edge2.lock()) << "visited"  << edge2.lock()->visited;
        if (edge2->visited)
            continue;
        if (edge->isColinearAndTouching(edge2))
        {
            findThread(thread, map,edge2,edge2->getOtherV(touchPt));       // recurse
            return;
        }
    }

    // second pass looking for anything can make a turn
    auto edge3 = n->getFirstNonvisitedNeighbour(edge);
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
