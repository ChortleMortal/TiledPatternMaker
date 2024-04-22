////////////////////////////////////////////////////////////////////////////
//
// Selection.java
//
// A helper struct that holds information about a selection (see TileView).
// Probably not used in the applet at all.

#include "makers/tiling_maker/tile_selection.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "geometry/edge.h"

InteriorTilleSelector::InteriorTilleSelector(PlacedTilePtr pfp) : TileSelector(pfp)
{
    type = INTERIOR;
}

EdgeTileSelector::EdgeTileSelector(PlacedTilePtr pfp, EdgePtr edge) : TileSelector(pfp,edge)
{
    type = EDGE;
}

VertexTileSelector::VertexTileSelector(PlacedTilePtr pfp, QPointF pt) : TileSelector(pfp,pt)
{
    type = VERTEX;
}

MidPointTileSelector::MidPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt) : TileSelector(pfp,edge,pt)
{
    type = MID_POINT;
}

ArcPointTileSelector::ArcPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt) : TileSelector(pfp,edge,pt)
{
    type = ARC_POINT;
}

CenterTileSelector::CenterTileSelector(PlacedTilePtr pfp, QPointF pt) : TileSelector(pfp,pt)
{
    type = TILE_CENTER;
}

ScreenTileSelector::ScreenTileSelector(QPointF pt) : TileSelector(pt)
{
    type = SCREEN_POINT;
}

TileSelector::TileSelector(QPointF pt)
{
    this->pt = pt;
}

TileSelector::TileSelector(PlacedTilePtr pfp)
{
    this->pfp  = pfp;
    pt = pfp->getTransform().map(pfp->getTile()->getCenter()); // TODO is this  right
}

TileSelector::TileSelector(PlacedTilePtr pfp, QPointF pt)
{
    this->pfp  = pfp;
    this->pt   = pt;
}

TileSelector::TileSelector(PlacedTilePtr pfp, EdgePtr edge)
{
    this->pfp  = pfp;
    this->edge = edge;
    pt = edge->getLine().center();
}

TileSelector::TileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt)
{
    this->pfp  = pfp;
    this->edge = edge;
    this->pt   = pt;
}

QLineF TileSelector::getModelLine()
{
    return edge->getLine();
}

QLineF TileSelector::getPlacedLine()
{
    return pfp->getTransform().map(edge->getLine());
}

EdgePtr TileSelector::getPlacedEdge()
{
    return std::make_shared<Edge>(edge,pfp->getTransform());
}

QPointF TileSelector::getPlacedPoint()
{
    return pfp->getTransform().map(pt);
}

QPolygonF TileSelector::getPlacedPolygon()
{
    return pfp->getPlacedPoints();
}

QPolygonF TileSelector::getModelPolygon()
{
    return pfp->getTilePoints();
}

bool TileSelector::isPoint()
{
    if (type == VERTEX || type == MID_POINT  || type == TILE_CENTER)
        return  true;
    else
        return false;
}
