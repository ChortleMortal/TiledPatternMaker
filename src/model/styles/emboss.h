#pragma once
#ifndef EMBOSS_H
#define EMBOSS_H

////////////////////////////////////////////////////////////////////////////
//
// Emboss.java
//
// A rendering style for maps that pretends that the map is carves out
// of wood with a diamond cross section and shines a directional light
// on the wood from some angle.  The map is drawn as a collection of
// trapezoids, and the darkness of each trapezoid is controlled by the
// angle the trapezoid makes with the light source.  The result is a
// simple but highly effective 3D effect, similar to flat-shaded 3D
// rendering.
//
// In practice, we can make this a subclass of RenderOutline -- it uses the
// same pre-computed point array, and just add one parameter and overloads
// the draw function.

#include "model/styles/outline.h"

class Emboss : public Outline
{
public:
    Emboss(ProtoPtr proto);
    Emboss(StylePtr other);
    virtual ~Emboss();

    eStyleType  getStyleType() const override { return STYLE_EMBOSSED; }
    QString     getStyleDesc() const override { return "Embossed";}
    void        dump()         const override { qDebug().noquote() << getStyleDesc()  << "angle" << angle << "light" << light_x << light_y
                                     << "width:" << width << "outline:" << drawOutline << outline_width << "outlineColor" << outline_color  << colors.colorsString(); }

    qreal   getAngle();
    void    setAngle(qreal angle );
    void    setColorSet(ColorSet & cset);

   void     draw(class GeoGraphics * gg) override;

protected:
   void     setGreys();
   void     drawTrap(class GeoGraphics * gg, QPointF a, QPointF b, QPointF c, QPointF d );

    // Parameters of the rendering.
    qreal 		angle;

    // Internal data derived from the angle.
    qreal 		light_x;
    qreal 		light_y;

private:
    QVector<QColor> greys;
};
#endif



