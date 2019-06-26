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

#include "style/Colored.h"

////////////////////////////////////////////////////////////////////////////
//
// A style encapsulates drawing a map with colors.


////////////////////////////////////////////////////////////////////////////
//
// Creation.

Colored::Colored(PrototypePtr proto, PolyPtr bounds ) : Style(proto,bounds)
{
    colors.setColor(QColor( 20, 150, 210 ));
}

Colored::Colored(const Style *  other ) : Style(other)
{
    const Colored * otherColored = dynamic_cast<const Colored*>(other);
    if (otherColored)
    {
        colors =  otherColored->colors;
    }
    else
    {
        colors.setColor(QColor( 20, 150, 210 ));
    }
}

Colored::~Colored()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting colored";
#endif
}

void Colored::setColor(QColor color)
{
    colors.setColor(color);
}

QColor Colored::getColor()
{
    return  colors.getNextColor().color;  // if there is a single color, it just returns that
}
