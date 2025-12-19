#pragma once
#ifndef THICK_H
#define THICK_H

#include "model/styles/colored.h"

typedef std::shared_ptr<class MapBase>      MapBasePtr;
typedef std::shared_ptr<class Edge>         EdgePtr;

////////////////////////////////////////////////////////////////////////////
//
// Thick
//
// A style that has a thickness and can have its outline drawn.
//
////////////////////////////////////////////////////////////////////////////

class Thick : public Colored
{
public:
    Thick(const ProtoPtr &proto);
    Thick(StylePtr other);
    virtual ~Thick();

    qreal            getLineWidth()     { return width; }
    qreal            getOutlineWidth()  { return outline_width; }
    eDrawOutline     getDrawOutline()   { return drawOutline; }
    QColor           getOutlineColor()  { return outline_color; }
    Qt::PenJoinStyle getJoinStyle()     { return join_style; }
    Qt::PenCapStyle  getCapStyle()      { return cap_style;}

    void    setLineWidth(qreal width)           { this->width = width; }
    void    setOutlineWidth(qreal width)        { this->outline_width = width; }
    void    setDrawOutline(eDrawOutline outline){ this->drawOutline = outline; }
    void    setOutlineColor(QColor color)       { outline_color = color; };
    void    setJoinStyle(Qt::PenJoinStyle js)   { join_style = js; }
    void    setCapStyle(Qt::PenCapStyle cs)     { cap_style = cs; };

    void    draw(GeoGraphics *gg) override;
    void    draw(GeoGraphics *gg, QPen & pen, qreal width);

    virtual void        resetStyleRepresentation() override;
    virtual void        createStyleRepresentation() override;

    virtual eStyleType getStyleType() const override { return STYLE_THICK; }
    virtual QString    getStyleDesc() const override { return "Thick Lines"; }
    virtual void       dump()         const override { qDebug().noquote() << getStyleDesc() << "width:" << width << "outline:" << drawOutline << outline_width << "outlineColor" << outline_color  << colors.colorsString(); }

protected:
    qreal            width;
    eDrawOutline     drawOutline;
    qreal            outline_width;
    QColor           outline_color;
    Qt::PenJoinStyle join_style;
    Qt::PenCapStyle  cap_style;
};
#endif

