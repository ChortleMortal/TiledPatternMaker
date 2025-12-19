#pragma once
#ifndef INTERLACE_CASING_H
#define INTERLACE_CASING_H

#include "model/styles/casing.h"
#include "model/styles/casing_set.h"
#include "model/styles/interlace_side.h"

typedef std::shared_ptr<class InterlaceCasing> InterlaceCasingPtr;
typedef std::shared_ptr<class CasingNeighbours> CNeighboursPtr;

class InterlaceCasingSet : public CasingSet
{
public:
    InterlaceCasingSet() {}

    void    reset()  { weavings.clear(); this->clear(); }

    void    weave();
    void    useMap();

    void    dumpWeaveStatus();
};

class InterlaceCasing : public Casing
{
    friend class Interlace;

public:
    InterlaceCasing(CasingSet *owner, EdgePtr edge, qreal width);
    ~InterlaceCasing();

    void        init();
    void        createColors(QColor defaultColor);
    void        setUnder(bool set);

    void        drawShadows(GeoGraphics *gg, qreal shadow) const;

    void        setShadowColor();
    void        setGap(qreal gap);
    QColor      getColor() const { return color; }
    QPen     &  getShadowPen()   { return shadowPen; }

    QPointF     getShadowPt(QPointF from, QPointF to, qreal shadow) const;
    QPointF     getCurvedShadowPt(QPointF from, QPointF to, qreal shadow, eLSide lside) const;
	QPointF     getShadowVector(QPointF from, QPointF to, qreal shadow) const;
    QPointF     curveAlign(QPointF pt, eLSide lside) const;

    QString     casingInfo(NeighboursPtr n, InterlaceCasingSet &casings);
    bool        valid();

    void        dumpI(InterlaceCasingSet & casings);

    QPointF     getIsect(QLineF l1, eSide tside, QLineF l2, eSide oside);
    void        setIsectsForOvers(CasingSide * thisSide, CasingSide * otherSide);
    QPointF     connectArcs(QPointF center, qreal radius, qreal from, qreal to);

    void        useMap();

    void        setPainterPath() override;
    bool        validate() override;

    WeakThreadPtr   wthread;

protected:
    void        createBasic();

private:
    QColor      color;
    QColor      shadowColor;
    QPen        shadowPen;

    VertexPtr vs1above;
    VertexPtr vs1mid;
    VertexPtr vs1below;
    VertexPtr vs2above;
    VertexPtr vs2mid ;
    VertexPtr vs2below;
};
#endif
