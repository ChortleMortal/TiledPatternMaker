/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INTERLACE_H
#define INTERLACE_H

#include "style/thick.h"
#include "geometry/threads.h"

////////////////////////////////////////////////////////////////////////////
//
// Interlace.java
//
// Probably the most important rendering style from an historical point
// of view.  RenderInterlace assigns an over-under rule to the edges
// of the map and renders a weave the follows that assignment.  Getting
// the over-under rule is conceptually simple but difficult in practice,
// especially since we want to have some robust against degenerate maps
// being produced by other parts of the program (*sigh*).
//
// Basically, if a diagram can be interlaced, you can just choose an
// over-under relationship at one vertex and propagate it to all other
// vertices using a depth-first search.
//
// Drawing the interlacing takes a bit of trig, but it's doable.  It's
// just a pain when crossing edges don't cross in a perfect X.  I
// might get this wrong.

class piece
{
public:
    piece() { shadow = false; }
    QPointF below;
    QPointF cen;
    QPointF above;
    bool    shadow;
};

class segment
{
public:
    segment() {}
    segment(QColor color) { c = color; }

    QPolygonF toPoly();

    piece A;
    piece B;
    QColor c;
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
    bool    getIncludeTipVertices() { return includeTipVertices; }

    void    setGap(qreal Gap)       { gap = Gap; resetStyleRepresentation(); }
    void    setShadow(qreal Shadow) { shadow = Shadow; resetStyleRepresentation(); }
    void    setIncludeTipVertices(bool include) { includeTipVertices = include; resetStyleRepresentation(); }

protected:


private:
    // Private magic to make it all happen.
    QPointF getShadowVector(QPointF from, QPointF to);

    qreal capGap(QPointF p, QPointF base, qreal gap);

    void getPoints(EdgePtr edge, VertexPtr from, VertexPtr to, piece *p);

    // Propagate the over-under relationship from a vertices to its
    // adjacent edges.  The relationship is encapsulated in the
    // "edge_under_at_vert" variable, which says whether the
    // edge passed in is in the under state at this vertex.
    // The whole trick is to manage how neighbours receive modifications
    // of edge_under_at_vert.
    void propagate(VertexPtr vertex, EdgePtr edge, bool edge_under_at_vert);

    // Propagate the over-under relation from an edge to its incident vertices.
    void assignInterlacing();
    void initializeMap();
    void buildFrom();

    // Parameters of the rende   ring.
    qreal  gap;
    qreal  shadow;
    bool   includeTipVertices;

    // Internal representations of the rendering.
    QVector<segment>    pts;
    QStack<EdgePtr>     todo;
    Threads             threads;
};
#endif

