#pragma once
#ifndef OUTLINE_H
#define OUTLINE_H

#include "model/styles/outline_casing.h"
#include "model/styles/thick.h"
#include "sys/sys/debugflags.h"

typedef std::shared_ptr<class OutlineCasing> OutlineCasingPtr;

class OutlineCasingSet;

////////////////////////////////////////////////////////////////////////////
//
// Outline.java
//
// The simplest non-trivial rendering style.  Outline just uses
// some trig to fatten up the map's edges, also drawing a line-based
// outline for the resulting fat figure.
//
// The same code that computes the draw elements for Outline can
// be used by other "fat" styles, such as Emboss.

class Outline : public Thick
{
public:
    Outline(const ProtoPtr & proto);
    Outline(const StylePtr & other);
    virtual ~Outline();

    void resetStyleRepresentation() override;
    void createStyleRepresentation() override;
    void draw(GeoGraphics *gg ) override;

    virtual MapPtr     getStyleMap()  override;
    virtual void       setStyleMap(MapPtr map) override { casings.setMap(map); }

    virtual eStyleType getStyleType() const override { return STYLE_OUTLINED; }
    QString            getStyleDesc() const override { return "Outlined"; }
    virtual void       dump()         const override { qDebug().noquote() << getStyleDesc() << "width:" << width << "outline:" << drawOutline << outline_width << "outlineColor" << outline_color << colors.colorsString(); }

public slots:
    void    slot_dbgChanged(eDbgType type);
    void    slot_dbgTrigger(int val);

protected:
    OutlineCasingSet casings;
};
#endif

