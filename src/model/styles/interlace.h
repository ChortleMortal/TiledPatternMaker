#pragma once
#ifndef INTERLACE_H
#define INTERLACE_H

#include <QStack>
#include <QPainterPath>

#include "model/styles/interlace_casing.h"
#include "model/styles/thick.h"
#include "sys/sys/debugflags.h"
#include "sys/geometry/threads.h"

class Interlace : public Thick
{
    friend InterlaceCasing;

public:
    Interlace(ProtoPtr proto);
    Interlace(StylePtr other);
    virtual ~Interlace();

    void    resetStyleRepresentation() override;
    void    createStyleRepresentation() override;

    void    draw(GeoGraphics *gg) override;

    qreal   getGap()                { return gap; }
    qreal   getShadow()             { return shadow; }
    bool    getInitialStartUnder()  { return interlace_start_under; }
    bool    getIncludeTipVertices() { return includeTipVertices; }

    void    setGap(qreal Gap)                   { gap = Gap; }
    void    setShadow(qreal Shadow)             { shadow = Shadow; }
    void    setInitialStartUnder(bool sunder)   { interlace_start_under = sunder; }
    void    setIncludeTipVertices(bool include) { includeTipVertices = include; }

    virtual MapPtr  getStyleMap()  override;
    virtual void    setStyleMap(MapPtr map) override { casings.setMap(map); }

    eStyleType      getStyleType() const override { return STYLE_INTERLACED; }
    QString         getStyleDesc() const override { return "Interlaced"; }
    void            dump()         const override { qDebug().noquote() << getStyleDesc() << "gap" << gap
                                                    << "shadow" << shadow << "tipVerts" << includeTipVertices
                                                    << "start_under" << interlace_start_under
                                                    << "width:" << width << "outline:" << drawOutline << outline_width
                                                    << "outlineColor" << outline_color << colors.colorsString(); }

    static uint dbgDump2;
    static uint iTrigger;
    static uint iCount;

public slots:
    void    slot_dbgChanged(eDbgType type);
    void    slot_dbgTrigger(int val);
    void    slot_styleMapUpdated(MapPtr map) override;

protected:
  // void    createUnders();
  //void    alignCurvedEdges(eDbgFlag flag);

    void    buildFrom();
    void    propagate(InterlaceCasingPtr &cp, InterlaceSide *s, bool edge_under_at_vert);
    void    drawDebugInterlace(bool solo, int index);
    bool    dbgBreak(InterlaceCasingPtr &casing, QString msg);

private:
    // Parameters of the rendering.
    qreal  gap;
    qreal  shadow;
    bool   includeTipVertices;
    bool   interlace_start_under;
    QColor defaultColor;


    // Internal representations of the rendering.
    InterlaceCasingSet            casings;       // these are drawn
    Threads                       threads;
    QStack<InterlaceCasingPtr>    todo;
};
#endif

