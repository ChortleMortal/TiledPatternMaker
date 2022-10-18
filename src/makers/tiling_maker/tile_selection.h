////////////////////////////////////////////////////////////////////////////
//
// Selection.java
//
// A helper struct that holds information about a selection (see TileView).
// Probably not used in the applet at all.

#ifndef TILE_SELECTION_H
#define TILE_SELECTION_H

#include <QPointF>
#include <QLineF>
#include <QString>
#include <QPolygonF>

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;

#define E2STR(x) #x

enum eSelection
{
    INTERIOR,
    EDGE,
    VERTEX,
    MID_POINT,
    ARC_POINT,
    FEAT_CENTER,
    SCREEN_POINT
    };

static QString strTiliingSelection[] =
{
    E2STR(INTERIOR),
    E2STR(EDGE),
    E2STR(VERTEX),
    E2STR(MID_POINT),
    E2STR(ARC_POINT),
    E2STR(FEAT_CENTER),
    E2STR(SCREEN_POINT)
};

//class TilingMakerView;

class TileSelector
{
public:
    eSelection       getType()          { return type; }
    QString          getTypeString()    { return strTiliingSelection[type]; }

    EdgePtr          getModelEdge()     { return edge; }
    QLineF           getModelLine();
    QPointF          getModelPoint()    { return pt; }
    QPolygonF        getModelPolygon();

    PlacedTilePtr getPlacedTile() { return pfp; }
    QLineF           getPlacedLine();
    QPointF          getPlacedPoint();
    QPolygonF        getPlacedPolygon();
    EdgePtr          getPlacedEdge();

protected:
    TileSelector(QPointF pt);
    TileSelector(PlacedTilePtr pfp);
    TileSelector(PlacedTilePtr pfp, QPointF pt);
    TileSelector(PlacedTilePtr pfp, EdgePtr edge);
    TileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);

    eSelection       type;

private:
    PlacedTilePtr pfp;
    QPointF          pt;
    EdgePtr          edge;
};


class InteriorTilingSelector : public TileSelector
{
public:
    InteriorTilingSelector(PlacedTilePtr pfp);
};

class EdgeTilingSelector : public TileSelector
{
public:
    EdgeTilingSelector(PlacedTilePtr pfp, EdgePtr edge);
};

class VertexTilingSelector : public TileSelector
{
public:
    VertexTilingSelector(PlacedTilePtr pfp, QPointF pt);
};

class MidPointTilingSelector : public TileSelector
{
public:
    MidPointTilingSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);
};

class ArcPointTilingSelector : public TileSelector
{
public:
    ArcPointTilingSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);
};

class CenterTilingSelector : public TileSelector
{
public:
    CenterTilingSelector(PlacedTilePtr, QPointF pt);
};

class ScreenTilingSelector : public TileSelector
{
public:
    ScreenTilingSelector(QPointF pt);
};


#endif

