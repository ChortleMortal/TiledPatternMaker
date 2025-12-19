#pragma once
#ifndef TPM_THREAD_H
#define TPM_THREAD_H

#include <QColor>
#include "model/styles/colorset.h"
#include "model/styles/casing.h"

typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Thread>           ThreadPtr;

//#define  DEBUG_THREADS
//#define  THREAD_LIMITS

class Thread : public QVector<CasingPtr>
{
public:
    Thread() {}
    QColor color;
};

class Threads : public QVector<ThreadPtr>
{
public:
    Threads();

    void        createThreads(CasingSet &casings);
    void        assignColors(ColorSet & colors);

    uint        chainLimit;

protected:
    void        addToThread(ThreadPtr &thread, CasingSet &casings, CasingPtr &casing, VertexPtr &v);

    bool        reachesBoundary(EdgePtr edge);
    bool        meetsThis(CasingSet &casings, EdgePtr edge, ThreadPtr & thread, NeighboursPtr n);
    EdgeSet     choices(CasingSet&casings, EdgePtr edge, NeighboursPtr n);
    EdgePtr     isLinear(EdgePtr edge, EdgeSet & eset);
    EdgePtr     getContinuation(EdgePtr edge, EdgeSet & eset, int type);

private:
    uint        chainCount;

};

#endif // THREAD_H
