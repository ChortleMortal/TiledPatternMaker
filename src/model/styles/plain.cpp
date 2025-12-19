#include <QPainter>
#include <QDebug>

#include "gui/viewers/geo_graphics.h"
#include "model/styles/plain.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/map.h"
#include "sys/sys/debugflags.h"

#undef DEBUG_PLAIN

#ifdef DEBUG_PLAIN
#include "sys/sys/debugflags.h"
#endif

////////////////////////////////////////////////////////////////////////////
//
// Plain.java
//
// The trivial rendering style.  Render the map as a collection of
// line segments.  Not very useful considering that DesignPreview does
// this better.  But there needs to be a default style for the RenderView.
// Who knows -- maybe some diagnostic information could be added later.


// Creation.

Plain::Plain(ProtoPtr proto) : Colored(proto)
{
    created = false;
}

Plain::Plain(StylePtr other) : Colored(other)
{
    created = false;
}

Plain::~Plain()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting plain";
    colors.clear();
#endif
}

void Plain::createStyleRepresentation()
{
    if (created)
        return;

    MapPtr map = prototype->getProtoMap();
    if (!map)
    {
        return;
    }

#ifdef DEBUG_PLAIN
    if (Sys::flags->debugFlag("dbg_plain"))
    {
        Sys::debugMap->wipeout();
        for (auto & edge : std::as_const(map->getEdges()))
        {
            Sys::debugMap->insertDebugMark(edge->getMidPoint(),QString::number(map->edgeIndex(edge)),Qt::red);
        }
    }
#endif
    created = true;
}

void Plain::draw(GeoGraphics *gg)
{
    //qDebug() << "Plain::draw";

    if (!isVisible())
    {
        return;
    }

    MapPtr map = prototype->getProtoMap();
    if (!map)
    {
        return;
    }

    QPen pen(colors.getNextTPColor().color);

    for (const auto &edge : std::as_const(map->getEdges()))
    {
        gg->drawEdge(edge,pen);
    }
}

