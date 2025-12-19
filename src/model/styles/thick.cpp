#include <QDebug>
#include <QPainter>
#include <QPainterPathStroker>
#include "gui/viewers/geo_graphics.h"
#include "model/makers/mosaic_maker.h"
#include "model/styles/colored.h"
#include "model/styles/thick.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbours.h"

////////////////////////////////////////////////////////////////////////////
//
// Thick
//
// A style that has a thickness and can have its outline drawn.
//
////////////////////////////////////////////////////////////////////////////

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
#endif
}

void Thick::resetStyleRepresentation()
{
    created  = false;
}

void Thick::createStyleRepresentation()
{
    if (!created)
    {
        created = true;
        prototype->getProtoMap(true);
    }
}

void Thick::draw(GeoGraphics * gg )
{
    //qDebug() <<  "Thick::draw";

    if (!isVisible())
    {
        return;
    }

    // Thick simply uses calls to GeoGraphics which in turn calls QPainter
    // If there is a border, this is a thicker line which is painted first
    // then the body is painted
    // Note: the width is multuplied by two because all other styles do this

    // DAC: a problem with this, is that if the opecity is not 100% then it is not
    // an outline that is drawn, but the border color mixes (multiplies) with the
    // line color.  So the Outline style is a solution to this, although it is not
    // producing results as good as the Thick style.  (see sty-InterlaceTest.v5.xml)

    if  (drawOutline != OUTLINE_NONE)
    {
        // paint wider first
        qreal pwidth;
        if (drawOutline == OUTLINE_SET)
            pwidth = width * 2 + outline_width;
        else
            pwidth = width * 2 + 0.05;

        QPen pen(outline_color);
        pen.setJoinStyle(join_style);
        pen.setCapStyle(cap_style);
        draw(gg,pen,pwidth);
    }

    QPen pen(colors.getNextTPColor().color);
    pen.setJoinStyle(join_style);
    pen.setCapStyle(cap_style);
    draw(gg,pen,width*2.0);
}

void Thick::draw(GeoGraphics *gg, QPen & pen, qreal width)
{
    //qDebug() <<  "Thick::draw";

    const MapPtr & map = prototype->getProtoMap();
    for (const auto & edge : std::as_const(map->getEdges()))
    {
        gg->drawThickEdge(edge,width,pen);
    }
}

