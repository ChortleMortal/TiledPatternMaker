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

#ifndef EXTENDED_STAR_H
#define EXTENDED_STAR_H

#include "tapp/Star.h"

class ExtendedStar : public Star
{
    friend class FigureView;

public:
    ExtendedStar(int n, qreal d, int s,
                 qreal r = 0.0,
                 bool  extendPeripherals  = false,
                 bool  extendFreeVertices = true);

    ExtendedStar(const Figure &fig,
                 int n, qreal d, int s,
                 qreal r = 0.0,
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
    void    extendMap();
    void    extendLine(MapPtr map, QLineF line);

    bool    extendFreeVertices;
    bool    extendPeripheralVertices;

private:

};
#endif

