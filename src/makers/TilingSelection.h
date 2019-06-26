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
// Selection.java
//
// A helper struct that holds information about a selection (see FeatureView).
// Probably not used in the applet at all.

#ifndef TILING_SELECTION_H
#define TILING_SELECTION_H

#include <QPointF>
#include <QLineF>
#include "base/shared.h"

enum eSelection
{
    NOTHING       = 0,
    INTERIOR      = 1,
    EDGE          = 2,
    VERTEX        = 3,
    MID_POINT     = 4
};

class TilingDesignerView;

class TilingSelection
{
public:
    TilingSelection(eSelection type, PlacedFeaturePtr pfp);
    TilingSelection(eSelection type, PlacedFeaturePtr pfp, QPointF pt);
    TilingSelection(eSelection type, PlacedFeaturePtr pfp, QLineF  line);

    PlacedFeaturePtr getFeature() { return pfp; }
    QLineF           getLine()    { return line; }
    QPointF          getPoint()   { return pt; }

    eSelection type;

private:
    PlacedFeaturePtr pfp;
    QPointF          pt;
    QLineF           line;
};

typedef shared_ptr<TilingSelection> TilingSelectionPtr;

#endif

