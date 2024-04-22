#include <QDebug>

#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_monitor.h"
#include "misc/sys.h"
#include "geometry/geo.h"
#include "tile/placed_tile.h"
#include "viewers/tiling_maker_view.h"
#include "viewers/view.h"

TilingMonitor::TilingMonitor()
{
    doit = false;

    tilingMaker = Sys::tilingMaker;

    connect(this, &TilingMonitor::sig_update, Sys::view, &View::update);
}

bool TilingMonitor::hasChanged(TilingPtr tp)
{
    if (tp == wTiling.lock() && tileChanged)
        return true;
    else
        return false;
}

void TilingMonitor::slot_tilingChanged()
{
    wTiling = Sys::tilingMaker->getSelected();
    tileChanged = true;
    doit = true;
}

void TilingMonitor::slot_tileChanged()
{
    wTiling = Sys::tilingMaker->getSelected();
    tileChanged = true;
    doit = true;
}

void TilingMonitor::slot_monitor(bool reset)
{
    wTiling = Sys::tilingMaker->getSelected();
    if (reset)
    {
        tileChanged = false;
    }
    doit = true;
}

void TilingMonitor::run()
{
    for (;;)
    {
        while (doit)
        {
            doit = false;

            determineOverlapsAndTouching();
        }
        msleep(50);
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Validating that features don't overlap when tiled.
//
// Note: we're not very stringent to avoid flagging polygons that would
//       only slightly overlap. This is more about including completely
//       unnecessary polygons that cover area already covered by another
//       copy, or other gross errors that are hard to see (especially once
//       the plane if fully tiled!).
//
// casper - original is replaced with a more robust implementation which
// also distinguishes between touching and overlapping.

void TilingMonitor::determineOverlapsAndTouching()
{
    PlacedTiles allPlacedTiles;

    TilingMakerView * tmv = Sys::tilingMakerView;

    allPlacedTiles = tmv->getAllTiles();  // makes a local copy

    for (const auto & tile : std::as_const(allPlacedTiles))
    {
        tile->clearViewState();
    }

    for (const auto & tile1 : std::as_const(allPlacedTiles))
    {
        if (!tile1->show())  continue;

        QPolygonF poly1 = tile1->getPlacedPoints();
        for (const auto & tile2 : std::as_const(allPlacedTiles))
        {
            if (!tile2->show())  continue;
            if (tile2 ==  tile1) continue;

            QPolygonF poly2 = tile2->getPlacedPoints();

            if (poly1.intersects(poly2))
            {
                QPolygonF p3 = poly1.intersected(poly2);
                qreal area = Geo::calcArea(p3);
                if (Loose::zero(area))
                {
                    tile1->setTouching();
                    tile2->setTouching();
                }
                else
                {
                    tile1->setOverlapping();
                    tile2->setOverlapping();
                }
            }
        }
    }
    if (Sys::view->isActiveLayer(tmv))
    {
        emit sig_update();
    }
}

