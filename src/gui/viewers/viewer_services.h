#pragma once

#include <QBrush>

class GeoGraphics;

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class Motif>         MotifPtr;

class ViewerServices
{
public:
    static void  drawTile(GeoGraphics * gg, TilePtr tile, QBrush brush, QPen pen);
    static void  drawMotif (GeoGraphics * gg, MotifPtr motif, QPen pen);
    static void  drawExtendedBoundary(GeoGraphics * gg, MotifPtr motif, QPen pen);
};
