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
