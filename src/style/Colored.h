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

#ifndef COLORED_H
#define COLORED_H

#include <QColor>

#include "style/style.h"
#include "base/colorset.h"

////////////////////////////////////////////////////////////////////////////
//
// A style encapsulates drawing a map with colors.


class Colored : public Style
{
public:
    void   setColor(QColor color);
    void   setColorSet(ColorSet & cset) { colors = cset; }

    ColorSet * getColorSet() { return &colors; }

protected:
    Colored(PrototypePtr proto);
    Colored(StylePtr other);
    ~Colored();

    ColorSet colors;
};
#endif

