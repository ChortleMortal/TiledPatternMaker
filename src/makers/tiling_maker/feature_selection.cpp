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

#include "makers/tiling_maker/feature_selection.h"
#include "tile/placed_feature.h"
#include "tile/feature.h"
#include "geometry/edge.h"

InteriorTilingSelector::InteriorTilingSelector(PlacedFeaturePtr pfp) : TilingSelector(pfp)
{
    type = INTERIOR;
}

EdgeTilingSelector::EdgeTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge) : TilingSelector(pfp,edge)
{
    type = EDGE;
}

VertexTilingSelector::VertexTilingSelector(PlacedFeaturePtr pfp, QPointF pt) : TilingSelector(pfp,pt)
{
    type = VERTEX;
}

MidPointTilingSelector::MidPointTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt) : TilingSelector(pfp,edge,pt)
{
    type = MID_POINT;
}

ArcPointTilingSelector::ArcPointTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt) : TilingSelector(pfp,edge,pt)
{
    type = ARC_POINT;
}

CenterTilingSelector::CenterTilingSelector(PlacedFeaturePtr pfp, QPointF pt) : TilingSelector(pfp,pt)
{
    type = FEAT_CENTER;
}

ScreenTilingSelector::ScreenTilingSelector(QPointF pt) : TilingSelector(pt)
{
    type = SCREEN_POINT;
}

TilingSelector::TilingSelector(QPointF pt)
{
    this->pt = pt;
}

TilingSelector::TilingSelector(PlacedFeaturePtr pfp)
{
    this->pfp  = pfp;
    pt = pfp->getTransform().map(pfp->getFeature()->getCenter()); // FIXME is this  right
}

TilingSelector::TilingSelector(PlacedFeaturePtr pfp, QPointF pt)
{
    this->pfp  = pfp;
    this->pt   = pt;
}

TilingSelector::TilingSelector(PlacedFeaturePtr pfp, EdgePtr edge)
{
    this->pfp  = pfp;
    this->edge = edge;
    pt = edge->getLine().center();
}

TilingSelector::TilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt)
{
    this->pfp  = pfp;
    this->edge = edge;
    this->pt   = pt;
}

QLineF TilingSelector::getModelLine()
{
    return edge->getLine();
}

QLineF TilingSelector::getPlacedLine()
{
    return pfp->getTransform().map(edge->getLine());
}

EdgePtr TilingSelector::getPlacedEdge()
{
    return std::make_shared<Edge>(edge,pfp->getTransform());
}

QPointF TilingSelector::getPlacedPoint()
{
    return pfp->getTransform().map(pt);
}

QPolygonF TilingSelector::getPlacedPolygon()
{
    return pfp->getPlacedPolygon();
}

QPolygonF TilingSelector::getModelPolygon()
{
    return pfp->getFeaturePolygon();
}
