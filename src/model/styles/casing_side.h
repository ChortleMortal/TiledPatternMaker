#ifndef CASING_SIDE_H
#define CASING_SIDE_H

#include <QColor>
#include <QDebug>
#include <QPainterPath>

#include "sys/enums/edgetype.h"
#include "sys/sys/debugflags.h"

class Casing;

typedef std::shared_ptr<class Neighbours>       NeighboursPtr;
typedef std::shared_ptr<class CasingNeighbours> CNeighboursPtr;
typedef std::weak_ptr<class CasingNeighbours>   WeakCNeighboursPtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::weak_ptr<class Vertex>             WeakVertexPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::weak_ptr<class Edge>               WeakEdgePtr;

class CasingSide
{
public:
    CasingSide(Casing * parent, eSide side, CNeighboursPtr np, VertexPtr vertex);
    virtual ~CasingSide() {}

    CasingSide & operator =(const CasingSide & other);

    void    createSide1(const EdgePtr &edge, qreal qwidth);
    void    createSide2(const EdgePtr &edge, qreal qwidth);

    VertexPtr vertex()      { return wv.lock(); }

    void    debugPoints();
    bool    validate();
    void    dump()          { qDebug() << outer << mid << inner; }

    QPointF outer;
    QPointF mid;
    QPointF inner;

    eSide   side;

    CNeighboursPtr      cneighbours;
    WeakVertexPtr       wv;

    bool    created;

    Casing * getParent()   { return parent; }

protected:
    QPointF getJoinPoint(const EdgePtr&  from, QPointF mid,  const EdgePtr &  to, qreal qwidth, eLSide lside);

    void    mark(eDbgFlag flag, QPointF & pt, const QString &txt, QColor color = Qt::black);

    Casing * parent;

private:
    QPointF _getJoinPoint(QPointF from, QPointF joint, QPointF to, qreal qwidth, eLSide lside);


};

#endif // CASING_SIDE_H
