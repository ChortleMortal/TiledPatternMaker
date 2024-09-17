#pragma once
#ifndef SKETCH_H
#define SKETCH_H

#include "model/styles/plain.h"

////////////////////////////////////////////////////////////////////////////
//
// Sketch.java
//
// One day, it occured to me that I might be able to get a sketchy
// hand-drawn effect by drawing an edge as a set of line segments whose
// endpoints are jittered relative to the original edge.  And it worked!
// Also, since the map is fixed, we can just reset the random seed every
// time we draw the map to get coherence.  Note that coherence might not
// be a good thing -- some animations work well precisely because the
// random lines that make up some object change from frame to frame (c.f.
// Bill Plympton).  It's just a design decision, and easy to reverse
// (or provide a UI for).
//
// I haven't tried it yet, but I doubt this looks any good as postscript.
// the resolution is too high and it would probably look like, well,
// a bunch of lines.

class Sketch : public Plain
{
protected:

public:
    Sketch(ProtoPtr proto);
    Sketch(StylePtr other);
    virtual ~Sketch();

    void draw(GeoGraphics *gg) override;

    eStyleType  getStyleType() const override { return STYLE_SKETCHED; }
    QString     getStyleDesc() const override { return "Sketched"; }
    void        dump()         const override { qDebug().noquote() << getStyleDesc() << colors.colorsString(); }
};
#endif

