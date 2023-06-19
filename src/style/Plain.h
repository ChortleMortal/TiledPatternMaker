#pragma once
#ifndef PLAIN_H
#define PLAIN_H

#include "style/colored.h"

////////////////////////////////////////////////////////////////////////////
//
// Plain.java
//
// The trivial rendering style.  Render the map as a collection of
// line segments.  Not very useful considering that DesignPreview does
// this better.  But there needs to be a default style for the RenderView.
// Who knows -- maybe some diagnostic information could be added later.


// Internal representations of the rendering.
class Plain : public Colored
{
public:
    Plain(ProtoPtr proto);
    Plain(StylePtr other);
    virtual ~Plain();

    void resetStyleRepresentation() override;
    void createStyleRepresentation() override;

    void draw(GeoGraphics * gg) override;

    virtual eStyleType getStyleType() const override { return STYLE_PLAIN; }
    QString            getStyleDesc() const override {return "Plain";}
    virtual void       report()       const override { qDebug().noquote() << getStyleDesc() << colors.colorsString(); }

protected:
private:
};
#endif

