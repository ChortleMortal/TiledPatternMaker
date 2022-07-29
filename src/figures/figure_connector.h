#ifndef FIGURECONNECTOR_H
#define FIGURECONNECTOR_H

#include <memory>
#include <QString>

class RadialFigure;

typedef std::shared_ptr<class Map>        MapPtr;
typedef std::shared_ptr<class Vertex>     VertexPtr;

class FigureConnector
{
public:
    FigureConnector(RadialFigure * rp);

    void  connectFigure(MapPtr unitMap);
    qreal computeScale(MapPtr cunit);

protected:
    void dumpM(QString s,  QMap<VertexPtr, VertexPtr> &movers);
    void rotateHalf(MapPtr cunit );
    void scaleToUnit( MapPtr cunit);

private:
    RadialFigure * rp;
};

#endif // FIGURECONNECTOR_H
