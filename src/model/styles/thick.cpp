#include <QDebug>

#include "model/styles/thick.h"
#include "sys/geometry/map.h"
#include "model/styles/colored.h"
#include <QPainter>
#include "gui/viewers/geo_graphics.h"

////////////////////////////////////////////////////////////////////////////
//
// Thick.java
//
// A style that has a thickness and can have its outline drawn.
//
////////////////////////////////////////////////////////////////////////////
//
// Creation.

Thick::Thick(const ProtoPtr &  proto): Colored(proto)
{
    width         = 0.05;
    drawOutline   = OUTLINE_NONE;           // DAC added
    outline_width = 0.05;
    outline_color = Qt::black;
    join_style    = Qt::RoundJoin;
    cap_style     = Qt::RoundCap;
}

Thick::Thick(StylePtr other ) : Colored(other)
{
    std::shared_ptr<Thick> thick  = std::dynamic_pointer_cast<Thick>(other);
    if (thick)
    {
        width         = thick->width;
        drawOutline   = thick->drawOutline;
        outline_width = thick->outline_width;
        outline_color = thick->outline_color;
        join_style    = thick->join_style;
        cap_style     = thick->cap_style;
    }
    else
    {
        width         = 0.05;
        drawOutline   = OUTLINE_NONE;           // DAC added
        outline_width = 0.05;
        outline_color = Qt::black;
        join_style    = Qt::RoundJoin;
        cap_style     = Qt::RoundCap;
    }
}

Thick::~Thick()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting thick";
    colors.clear();
#endif
}

#if 0
void Thick::resetStyleRepresentation()
{

}

void Thick::createStyleRepresentation()
{
}
#endif

void Thick::draw(GeoGraphics * gg )
{
    //qDebug() << "Thick::draw";

    if (!isVisible())
    {
        return;
    }

    MapPtr map = getPrototype()->getProtoMap();
    if (!map)
    {
        qDebug() << "Thick::draw EMPTY Map";
        return;
    }

    // Note: we multiply the width by two because all other styles using
    //       the width actully widen the drawing in both perpendicular
    //       directions by that width.

    // paint wider first
    if  (drawOutline != OUTLINE_NONE)
    {
        qreal pwidth;

        if (drawOutline == OUTLINE_SET)
        {
            pwidth = width * 2 + outline_width;
        }
        else
        {
            Q_ASSERT(drawOutline == OUTLINE_DEFAULT);
            pwidth = width * 2 + 0.05;
        }
        //qreal pwidth = (drawOutline == OUTLINE_SET) ? width * 2 + outline_width : width * 2 + 0.05;
        //qreal pwidth = (drawOutline == OUTLINE_SET) ? width * 2 + outline_width : width * 2 + 0.05;
        QPen pen(outline_color);
        pen.setJoinStyle(join_style);
        pen.setCapStyle(cap_style);
        for (const auto & edge : std::as_const(map->getEdges()))
        {
            gg->drawThickEdge(edge,pwidth, pen);
        }
    }

    QPen pen(colors.getNextTPColor().color);
    pen.setJoinStyle(join_style);
    pen.setCapStyle(cap_style);
    for (const auto & edge : std::as_const(map->getEdges()))
    {
        gg->drawThickEdge(edge, width * 2, pen);
    }
}

