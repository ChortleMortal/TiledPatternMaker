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

#include "tapp/Figure.h"
#include "tile/Feature.h"

int Figure::refs = 0;

Figure::Figure()
{
    refs++;
    figType           = FIG_TYPE_UNDEFINED;
    extBoundarySides  = 1;  // defaults to a circle
    extBoundaryScale  = 1.0;
    figureScale       = 1.0;
    figureRotate      = 0.0;
    hasCircleBoundary = true;

    figureMap = make_shared<Map>("figureMap");
}

Figure::Figure(const Figure & other)
{
    refs++;

    figType           = FIG_TYPE_UNDEFINED;

    extBoundarySides  = other.extBoundarySides;
    extBoundaryScale  = other.extBoundaryScale;
    figureScale       = other.figureScale;
    figureRotate      = other.figureRotate;

    radialFigBoundary = other.radialFigBoundary;
    extBoundary       = other.extBoundary;
    hasCircleBoundary = other.hasCircleBoundary;

    figureMap         = other.figureMap;
}

Figure::~Figure()
{
    //qDebug() << "Figure destructor" << this;
    refs--;
}

void Figure::resetMaps()
{
    if (figureMap)
        figureMap->wipeout();
    if (debugMap)
        debugMap->wipeout();
}

bool Figure::isExplicit()
{
    switch (figType)
    {
    case FIG_TYPE_EXPLICIT:
    case FIG_TYPE_INFER:
    case FIG_TYPE_EXPLICIT_ROSETTE:
    case FIG_TYPE_HOURGLASS:
    case FIG_TYPE_INTERSECT:
    case FIG_TYPE_GIRIH:
    case FIG_TYPE_EXPLICIT_STAR:
    case FIG_TYPE_FEATURE:
        return true;
    default:
        return false;
    }
}

bool Figure::isRadial()
{
    // assume undefined is radial
    return !isExplicit();
}

void Figure::setExtBoundarySides(int sides)
{
    extBoundarySides = sides;
    hasCircleBoundary = (sides < 3) ? true : false;
}

void Figure::buildExtBoundary()
{
    QTransform qTrans;
    qTrans.scale(extBoundaryScale,extBoundaryScale);
    if (extBoundarySides >= 3)
    {
        Feature f2(extBoundarySides,0);
        extBoundary = f2.getPoints();
        extBoundary = qTrans.map(extBoundary);
        //qDebug() << "Ext boundary:" << extBoundary;
        hasCircleBoundary = false;
    }
    else
    {
        //qDebug() << "circular boundary";
        hasCircleBoundary = true;
    }
}

// uses existing tmpIndices
void Figure::annotateEdges()
{
    if (!figureMap)
    {
        return;
    }

    for (auto edge : figureMap->getEdges())
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(edge->getTmpEdgeIndex()));
    }
    debugMap->dumpMap(false);
}
