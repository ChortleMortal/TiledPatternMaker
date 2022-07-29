#ifndef EXTENDED_STAR_H
#define EXTENDED_STAR_H

#include "figures/star.h"

typedef std::shared_ptr<class Vertex>           VertexPtr;

class ExtendedStar : public Star
{
    friend class MotifView;

public:
    ExtendedStar(int n, qreal d, int s,
                 bool  extendPeripherals = false,
                 bool  extendFreeVertices = true);

    ExtendedStar(const Figure & fig,
                 int n, qreal d, int s,
                 bool  extendPeripherals  = false,
                 bool  extendFreeVertices = true);
    virtual ~ExtendedStar() override {}

    void    buildMaps() override;

    void    setExtendPeripheralVertices(bool extend){ extendPeripheralVertices = extend; }
    void    setExtendFreeVertices(bool extend){ extendFreeVertices = extend;  }

    bool    getExtendPeripheralVertices(){ return extendPeripheralVertices; }
    bool    getExtendFreeVertices(){ return extendFreeVertices; }

    virtual QString getFigureDesc() override { return "Extended Star";}

protected:
    void    extendPeripheralMap();
    void    extendFreeMap();
    void    extendLine(MapPtr map, QLineF line);

    bool    extendFreeVertices;
    bool    extendPeripheralVertices;

    VertexPtr findVertex(QPointF pt);

private:

};
#endif

