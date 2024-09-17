#pragma once
#ifndef TILE_SELECTION_H
#define TILE_SELECTION_H

////////////////////////////////////////////////////////////////////////////
//
// Selection.java
//
// A helper struct that holds information about a selection (see TileView).
// Probably not used in the applet at all.

#include <QPointF>
#include <QLineF>
#include <QString>
#include <QPolygonF>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Edge>          EdgePtr;

#define E2STR(x) #x

enum eSelection
{
    INTERIOR,
    EDGE,
    VERTEX,
    MID_POINT,
    ARC_POINT,
    TILE_CENTER,
    SCREEN_POINT
    };

static QString strTiliingSelection[] =
{
    E2STR(INTERIOR),
    E2STR(EDGE),
    E2STR(VERTEX),
    E2STR(MID_POINT),
    E2STR(ARC_POINT),
    E2STR(TILE_CENTER),
    E2STR(SCREEN_POINT)
};

//class TilingMakerView;

class PlacedTileSelector
{
public:
    eSelection       getType()          { return type; }
    QString          getTypeString()    { return strTiliingSelection[type]; }

    EdgePtr          getModelEdge()     { return edge; }
    QLineF           getModelLine();
    QPointF          getModelPoint()    { return pt; }
    QPolygonF        getModelPolygon();

    PlacedTilePtr    getPlacedTile()    { return pfp; }
    QLineF           getPlacedLine();
    QPointF          getPlacedPoint();
    QPolygonF        getPlacedPolygon();
    EdgePtr          getPlacedEdge();

    bool             isPoint();

protected:
    PlacedTileSelector(QPointF pt);
    PlacedTileSelector(PlacedTilePtr pfp);
    PlacedTileSelector(PlacedTilePtr pfp, QPointF pt);
    PlacedTileSelector(PlacedTilePtr pfp, EdgePtr edge);
    PlacedTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);

    eSelection      type;

private:
    PlacedTilePtr   pfp;
    QPointF         pt;
    EdgePtr         edge;
};


class InteriorTilleSelector : public PlacedTileSelector
{
public:
    InteriorTilleSelector(PlacedTilePtr pfp);
};

class EdgeTileSelector : public PlacedTileSelector
{
public:
    EdgeTileSelector(PlacedTilePtr pfp, EdgePtr edge);
};

class VertexTileSelector : public PlacedTileSelector
{
public:
    VertexTileSelector(PlacedTilePtr pfp, QPointF pt);
};

class MidPointTileSelector : public PlacedTileSelector
{
public:
    MidPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);
};

class ArcPointTileSelector : public PlacedTileSelector
{
public:
    ArcPointTileSelector(PlacedTilePtr pfp, EdgePtr edge, QPointF pt);
};

class CenterTileSelector : public PlacedTileSelector
{
public:
    CenterTileSelector(PlacedTilePtr, QPointF pt);
};

class ScreenTileSelector : public PlacedTileSelector
{
public:
    ScreenTileSelector(QPointF pt);
};

#endif

