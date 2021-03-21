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

#ifndef EXTENDED_ROSETTE_H
#define EXTENDED_ROSETTE_H

#include "tapp/rosette.h"

class ExtendedRosette : public Rosette
{
    friend class FigureView;
public:
    ExtendedRosette(const Figure & fig,
                    int n, qreal q, int s, qreal k = 0.0,
                    qreal figureRotate = 0,
                    bool  extendPeripheralVertices = false,
                    bool  extendFreeVertices       = true,
                    bool  connectBoundaryVertices  = false);

    ExtendedRosette(int n, qreal q, int s, qreal k = 0.0,
                    qreal figureRotate = 0,
                    bool  extendPeripheralVertices = false,
                    bool  extendFreeVertices       = true,
                    bool  connectBoundaryVertices  = false);

    virtual ~ExtendedRosette() override {}

    void    buildMaps() override;

    void    setExtendPeripheralVertices(bool extend){ extendPeripheralVertices = extend; }
    void    setExtendFreeVertices(bool extend){ extendFreeVertices = extend; }
    void    setConnectBoundaryVertices(bool connect) { connectBoundaryVertices = connect; }

    bool    getExtendPeripheralVertices(){ return extendPeripheralVertices; }
    bool    getExtendFreeVertices(){ return extendFreeVertices; }
    bool    getConnectBoundaryVertices() { return connectBoundaryVertices; }

    virtual QString getFigureDesc() override { return "Extended Rosette";}

protected:
    void    extendMap();
    void    connectOuterVertices(MapPtr map);

    bool    extendFreeVertices;
    bool    extendPeripheralVertices;
    bool    connectBoundaryVertices;

    qreal   len(VertexPtr v1, VertexPtr v2) { return QLineF(v1->pt,v2->pt).length(); }

private:

};
#endif

