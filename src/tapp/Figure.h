/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    virtual MapPtr  getFigureMap() = 0;
    virtual MapPtr  useDebugMap() { return debugMap; }
    virtual QString getFigureDesc() = 0;
    virtual void    buildMaps() = 0;

    virtual void    resetMaps();
    virtual void    buildExtBoundary();

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
    void            setExtBoundaryScale(qreal scale) { extBoundaryScale = scale; }

    int             getExtBoundarySides() { return extBoundarySides; }
    qreal           getExtBoundaryScale() { return extBoundaryScale; }
    bool            hasExtCircleBoundary(){ return hasCircleBoundary; }
    QPolygonF &     getExtBoundary()      { return extBoundary; }

    bool            isExplicit();
    bool            isRadial();

    virtual bool    equals(const  FigurePtr other);
    static int refs;

protected:
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
