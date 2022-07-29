#ifndef INTERLACE_H
#define INTERLACE_H

#include <QStack>

#include "style/thick.h"
#include "geometry/threads.h"
#include "enums/edgetype.h"

class Piece
{
public:
    Piece() { shadow = false; }

    void getPoints(EdgePtr edge, VertexPtr from, VertexPtr to, qreal width, qreal gap, MapPtr map, bool from_under);

    QPointF below;
    QPointF v;
    QPointF above;
    bool    shadow;

protected:
    qreal 	capGap(QPointF p, QPointF base, qreal gap);
};

class Segment
{
public:
    QPolygonF    getPoly();
    QPainterPath getPainterPath();

    void         dump();
    bool         valid();

    Piece       v1;
    Piece       v2;

    QColor      c;
    eEdgeType   type;
    QPointF     arcCenter;
    bool        convex;
};


class Interlace : public Thick
{
public:
    Interlace(PrototypePtr proto);
    Interlace(StylePtr other);
    virtual ~Interlace() override;

    void    resetStyleRepresentation() override;
    void    createStyleRepresentation() override;
    void    draw(GeoGraphics *gg) override;

    virtual eStyleType getStyleType() const override { return STYLE_INTERLACED; }
    QString getStyleDesc() const override {return "Interlaced";}

    qreal   getGap()                { return gap; }
    qreal   getShadow()             { return shadow; }
    bool    getInitialStartUnder()  { return interlace_start_under; }
    bool    getIncludeTipVertices() { return includeTipVertices; }

    void    setGap(qreal Gap)                   { gap = Gap; }
    void    setShadow(qreal Shadow)             { shadow = Shadow; }
    void    setInitialStartUnder(bool sunder)   { interlace_start_under = sunder; }
    void    setIncludeTipVertices(bool include) { includeTipVertices = include; }

protected:
    void    assignInterlacing();
    void    buildFrom();
    void    propagate(VertexPtr vertex, EdgePtr edge, bool edge_under_at_vert);

    QPointF getShadowVector(QPointF from, QPointF to);

private:
    MapPtr  map;

    // Parameters of the rendering.
    qreal  gap;
    qreal  shadow;
    bool   includeTipVertices;

    // Internal representations of the rendering.
    QVector<Segment>    segments;
    QStack<EdgePtr>     todo;
    Threads             threads;
    bool                interlace_start_under;
};
#endif

