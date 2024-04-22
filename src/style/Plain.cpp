#include <QPainter>
#include <QDebug>

#include "style/plain.h"
#include "geometry/map.h"
#include "misc/geo_graphics.h"

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
}

Plain::Plain(StylePtr other) : Colored(other)
{
}

Plain::~Plain()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting plain";
    colors.clear();
#endif
}

// Data.

void Plain::resetStyleRepresentation()
{
}

void Plain::createStyleRepresentation()
{
    getProtoMap();
}

void Plain::draw(GeoGraphics *gg)
{
    //qDebug() << "Plain::draw";

    if (!isVisible())
    {
        return;
    }

    MapPtr map = getProtoMap();
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

