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
