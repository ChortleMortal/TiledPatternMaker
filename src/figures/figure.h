////////////////////////////////////////////////////////////////////////////
//
// Figure.java
//
// Making the user interface operate directly on maps would be
// a hassle.  Maps are a very low level geometry and topological
// structure.  Not so good for interacting with users.  So I
// define a Figure class, which is a higher level structure --
// an object that knows how to build maps.  Subclasses of Feature
// understand different ways of bulding maps, but have the advantage
// of being parameterizable at a high level.

#ifndef FIGURE_H
#define FIGURE_H

#include <QPolygonF>
#include <QGraphicsItem>
#include <QPainter>
#include "enums/efigtype.h"

typedef std::shared_ptr<class Figure>       FigurePtr;
typedef std::shared_ptr<class Map>          MapPtr;

class Figure
{
public:
    Figure();
    Figure(const Figure & other);
    virtual ~Figure();

    virtual MapPtr  getFigureMap()  = 0;
    virtual void    resetMaps()     = 0;
    virtual void    buildMaps()     = 0;
    virtual QString getFigureDesc() = 0;
    virtual MapPtr  useDebugMap() { return debugMap; }

    virtual void    setN(int n) { this->n = n; }
    int             getN();

    QPolygonF       getPoints() { return points; }
    int             size() { return points.size(); }

    void            setFigType( eFigType ft) { figType = ft; }
    eFigType        getFigType()       { return figType; }
    QString         getFigTypeString() { return sFigType[getFigType()]; }
    static QString  getTypeString(eFigType type) { return sFigType[type]; }

    virtual void    setFigureScale(qreal scale)   { figureScale = scale; }
    virtual qreal   getFigureScale()              { return figureScale; }
    void            setFigureRotate(qreal rot)    { figureRotate = rot; }
    qreal           getFigureRotate()             { return figureRotate; }

    void            setRadialFigBoundary(QPolygonF p) { radialFigBoundary = p; }
    QPolygonF &     getRadialFigBoundary()            { return radialFigBoundary; }

    void            setExtBoundarySides(int sides);
    void            setExtBoundaryScale(qreal scale);

    int             getExtBoundarySides() { return extBoundarySides; }
    qreal           getExtBoundaryScale() { return extBoundaryScale; }
    bool            hasExtCircleBoundary(){ return hasCircleBoundary; }
    QPolygonF &     getExtBoundary()      { return extBoundary; }

    bool            isExplicit();
    bool            isRadial();

    virtual bool    equals(const  FigurePtr other);

    static int      refs;

protected:
    virtual void    buildExtBoundary();

    void            annotateEdges();
    void            drawAnnotation(QPainter *painter, QTransform T);

    MapPtr      figureMap;
    MapPtr      debugMap;
    QPolygonF   points;

    int         n;                      // number of sides = number of points

private:
    qreal       figureRotate;
    qreal       figureScale;

    QPolygonF   radialFigBoundary;      // currently only set for radial figures

    int         extBoundarySides;
    qreal       extBoundaryScale;
    QPolygonF   extBoundary;
    bool        hasCircleBoundary;

    eFigType    figType;
};

#endif
