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
#include <QString>
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "base/shared.h"

enum eSelection
{
    NOTHING       = 0,
    INTERIOR      = 1,
    EDGE          = 2,
    VERTEX        = 3,
    MID_POINT     = 4,
    ARC_POINT     = 5,
    FEAT_CENTER   = 6,
    SCREEN_POINT  = 7,
};

#define Enum2Str(e)  {QString(#e)}

static QString strTiliingSelection[] =
{
    Enum2Str(NOTHING),
    Enum2Str(INTERIOR),
    Enum2Str(EDGE),
    Enum2Str(VERTEX),
    Enum2Str(MID_POINT),
    Enum2Str(ARC_POINT),
    Enum2Str(FEAT_CENTER),
    Enum2Str(SCREEN_POINT)
};

class TilingMakerView;

class TilingSelection
{
public:
    TilingSelection(eSelection type, PlacedFeaturePtr pfp);
    TilingSelection(eSelection type, PlacedFeaturePtr pfp, QPointF pt);
    TilingSelection(eSelection type, PlacedFeaturePtr pfp, EdgePtr edge);
    TilingSelection(eSelection type, PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt);

    eSelection       getType()          { return type; }

    PlacedFeaturePtr getPlacedFeature() { return pfp; }
    QTransform       getTransform()     { return pfp->getTransform(); }
    FeaturePtr       getFeature()       { return pfp->getFeature(); }

    QPolygonF        getModelPolygon()  { return pfp->getFeaturePolygon(); }
    EdgePtr          getModelEdge()     { return edge; }
    QLineF           getModelLine()     { return edge->getLine(); }
    QPointF          getModelPoint()    { return pt; }

    EdgePtr          getPlacedEdge()    { return make_shared<Edge>(edge,pfp->getTransform()); }
    QPolygonF        getPlacedPolygon() { return pfp->getPlacedPolygon(); }
    QLineF           getPlacedLine()    { return pfp->getTransform().map(edge->getLine()); }
    QPointF          getPlacedPoint()   { return pfp->getTransform().map(pt); }

private:
    PlacedFeaturePtr pfp;
    QPointF          pt;
    EdgePtr          edge;
    eSelection       type;
};

typedef shared_ptr<TilingSelection> TilingSelectionPtr;

#endif

