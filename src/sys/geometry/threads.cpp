#include <QDebug>

#include "sys/geometry/threads.h"
#include "model/styles/interlace_casing.h"
#include "model/styles/casing_neighbours.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/neighbours.h"

extern void stackInfo();

//  Creating threads is recursive
//  Genuinesly complicated recursions can cause stack overflows
//  Putting in qDebug() calls is particularly expenseive
//  so best to be lean anmd mean in terms of code

Threads::Threads()
{
}

void Threads::createThreads(CasingSet &casings)
{
    chainCount =  0;

    for (CasingPtr & casing : casings)
    {
        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
        if (icp->wthread.lock())
            continue;

        ThreadPtr thread = std::make_shared<Thread>();
        push_back(thread);    // create thread

        VertexPtr v = casing->s2->vertex();
        addToThread(thread,casings,casing,v);

#ifdef THREAD_LIMITS
        if (chainCount == chainLimit)
            return;
#endif

        v = casing->s1->vertex();
        addToThread(thread,casings,casing,v);

#ifdef THREAD_LIMITS
        if (chainCount == chainLimit)
            return;
#endif
    }
}

/*
 Which edge is next in the thread
    if we have reached a boundary, then stop
    if there are no choices (already in threads) then stop
    if n=2 then it is a no brainer
    if n=4 then go with straightest
    if n=3, then ????
*/

void Threads::addToThread(ThreadPtr & thread, CasingSet &casings, CasingPtr &casing, VertexPtr & v)
{
#ifdef THREAD_LIMITS
    if (chainCount >= chainLimit)
        return;
    chainCount++;

    qDebug() << "LOG2 adding edge" << casing->edgeIndex;
#endif

    InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
    icp->wthread  = thread;
    thread->push_back(casing);  // add casing into thread

    EdgePtr edge = casing->getEdge();

    if (reachesBoundary(edge))
        return;

    CNeighboursPtr n = casing->getNeighbours(v);

//    if (meetsThis(casings,edge,thread,n))
//        return;

    EdgeSet eset = choices(casings, edge,n);

    if (eset.isEmpty())
        return;

    if (n->size() == 2)
    {
        Q_ASSERT(eset.size() == 1);
        EdgePtr edge2 = eset[0];
        CasingPtr ocp = casings.find(edge2);
        VertexPtr vp2 = edge2->getOtherV(v);
        addToThread(thread, casings, ocp, vp2);
        return;
    }

    // straight
    EdgePtr edge2 = isLinear(edge,eset);
    if (edge2)
    {
        CasingPtr ocp = casings.find(edge2);
        auto v2       = edge2->getOtherV(v);
        addToThread(thread, casings, ocp, v2);       // recurse
        return;
    }

    //EdgePtr edge2 = n->nearest(edge,false);
    //EdgePtr edge2 = n->getContinuation(edge,3);
    edge2 = getContinuation(edge,eset,3);
    if (edge2)
    {
        CasingPtr ocp = casings.find(edge2);
        auto v2 = edge2->getOtherV(v);
        addToThread(thread,casings, ocp,v2);       // recurse
        return;
    }
}

EdgePtr Threads::isLinear(EdgePtr edge, EdgeSet & eset)
{
    for (EdgePtr & edge2 : eset)
    {
        if (edge->isColinear(edge2,1.0))
        {
            return edge2;
        }
    }
    EdgePtr edge3;
    return edge3;
}

bool Threads::reachesBoundary(EdgePtr edge)
{
    // TODO implement reachesBoundary()
    Q_UNUSED(edge)
    return false;
}

EdgeSet  Threads::choices(CasingSet &casings, EdgePtr edge, NeighboursPtr n)
{
    EdgeSet rv;
    for (auto & wedge : *n)
    {
        auto edge2 = wedge.lock();
        if (edge2 == edge)
            continue;
        CasingPtr cp = casings.find(edge2);
        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(cp);
        if (icp->wthread.lock())
            continue;
        rv.push_back(edge2);
    }
    return rv;
}

bool Threads::meetsThis(CasingSet &casings, EdgePtr edge, ThreadPtr & thread, NeighboursPtr n)
{
    for (auto & wedge : *n)
    {
        auto edge2 = wedge.lock();
        if (edge2 == edge)
            continue;
        CasingPtr cp = casings.find(edge2);
        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(cp);
        if (icp->wthread.lock() == thread)
            return true;
    }
    return false;
}

EdgePtr Threads::getContinuation(EdgePtr edge, EdgeSet & eset, int type)
{
    EdgePtr result;
    qreal   angle;

    for (auto & nedge : eset)
    {
        if (!result)
        {
            // first time
            angle = qAbs(edge->getLine().angleTo(nedge->getLine()));
            result = nedge;
        }
        else
        {
            qreal angle2 = qAbs(edge->getLine().angleTo(nedge->getLine()));
            switch (type)
            {
            case 1:
                // this gets most acute (smallest) angle, not nearest to 180 degrees)
                if (angle2 < angle)
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            case 2:
                // this gets largest angle, not nearest to 180 degrees)
                if (angle2 > angle)
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            case 3:
                // this not nearest to 180 degrees)
                if ((180.0 - angle2) >  (180.0 - angle))
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            }
        }
    }
    return result;
}
void Threads::assignColors(ColorSet & colors)
{
    colors.resetIndex();
    QVector<ThreadPtr> & threads = *this;
    for (const auto & thread : std::as_const(threads))
    {
        thread->color = colors.getNextTPColor().color;
    }
}
