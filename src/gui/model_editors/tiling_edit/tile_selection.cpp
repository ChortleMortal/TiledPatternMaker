////////////////////////////////////////////////////////////////////////////
//
// Selection.java
//
// A helper struct that holds information about a selection (see TileView).
// Probably not used in the applet at all.

#include  "gui/model_editors/tiling_edit/tile_selection.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "sys/geometry/edge.h"

InteriorTilleSelector::InteriorTilleSelector(PlacedTilePtr pfp) : PlacedTileSelector(pfp)
{
    type = INTERIOR;
}

EdgeTileSelector::EdgeTileSelector(PlacedTilePtr pfp, EdgePtr edge) : PlacedTileSelector(pfp,edge)
{
    type = EDGE;
}

VertexTileSelector::VertexTileSelector(PlacedTilePtr pfp, QPointF pt) : PlacedTileSelector(pfp,pt)
{
    type = VERTEX;
}

MidPointTileSelector::MidPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt) : PlacedTileSelector(pfp,edge,pt)
{
    type = MID_POINT;
}

ArcPointTileSelector::ArcPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt) : PlacedTileSelector(pfp,edge,pt)
{
    type = ARC_POINT;
}

CenterTileSelector::CenterTileSelector(PlacedTilePtr pfp, QPointF pt) : PlacedTileSelector(pfp,pt)
{
    type = TILE_CENTER;
}

ScreenTileSelector::ScreenTileSelector(QPointF pt) : PlacedTileSelector(pt)
{
    type = SCREEN_POINT;
}

PlacedTileSelector::PlacedTileSelector(QPointF pt)
{
    this->pt = pt;
}

PlacedTileSelector::PlacedTileSelector(PlacedTilePtr pfp)
{
    this->pfp  = pfp;
    pt = pfp->getPlacement().map(pfp->getTile()->getCenter());
}

PlacedTileSelector::PlacedTileSelector(PlacedTilePtr pfp, QPointF pt)
{
    this->pfp  = pfp;
    this->pt   = pt;
}

PlacedTileSelector::PlacedTileSelector(PlacedTilePtr pfp, EdgePtr edge)
{
    this->pfp  = pfp;
    this->edge = edge;
    pt = edge->getLine().center();
}

PlacedTileSelector::PlacedTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt)
{
    this->pfp  = pfp;
    this->edge = edge;
    this->pt   = pt;
}

QLineF PlacedTileSelector::getModelLine()
{
    return edge->getLine();
}

QLineF PlacedTileSelector::getPlacedLine(bool distort)
{
    return pfp->getWorldTransform(distort ? Sys::getActiveDistortion() : QTransform()).map(edge->getLine());
}

EdgePtr PlacedTileSelector::getPlacedEdge(bool distort)
{
    return std::make_shared<Edge>(edge,pfp->getWorldTransform(distort ? Sys::getActiveDistortion() : QTransform()));
}

QPointF PlacedTileSelector::getPlacedPoint(bool distort)
{
    return pfp->getWorldTransform(distort ? Sys::getActiveDistortion() : QTransform()).map(pt);
}

QPolygonF PlacedTileSelector::getPlacedPolygon(bool distort)
{
    return pfp->getWorldPoints(distort ? Sys::getActiveDistortion() : QTransform());
}

QPolygonF PlacedTileSelector::getModelPolygon()
{
    TilePtr tile = pfp->getTile();
    if (!tile)
        return QPolygonF();
    return tile->getPoints();
}

bool PlacedTileSelector::isPoint()
{
    if (type == VERTEX || type == MID_POINT  || type == TILE_CENTER)
        return  true;
    else
        return false;
}
