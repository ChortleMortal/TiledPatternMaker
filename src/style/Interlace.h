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

#include "style/Thick.h"

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

class interlaceInfo
{
public:
    interlaceInfo() {visited = false; start_under = false;}
    bool visited;
    bool start_under;
};

class Interlace : public Thick
{
public:
    Interlace(PrototypePtr proto, PolyPtr bounds);
    Interlace(const Style & other);
    virtual ~Interlace() override;

    void resetStyleRepresentation() override;
    void createStyleRepresentation() override;
    void    draw(GeoGraphics *gg) override;

    virtual eStyleType getStyleType() const override { return STYLE_INTERLACED; }
    QString getStyleDesc() const override {return "Interlaced";}

    qreal   getGap();
    qreal   getShadow();
    bool    getIncludeTipVertices() { return includeTipVertices; }

    void    setGap(qreal gap );
    void    setShadow(qreal shadow );
    void    setIncludeTipVertices(bool include);

protected:


private:
    // Private magic to make it all happen.
    QPointF getShadowVector( int fromIndex, int toIndex );

    qreal capGap( QPointF p, QPointF base, qreal gap );

    void getPoints(EdgePtr edge, VertexPtr from, VertexPtr to, qreal width, qreal gap,
            QVector<QPointF> & pts, int ptsIndex, QVector<bool> & shadows, int shadowsIndex );

    // Propagate the over-under relationship from a vertices to its
    // adjacent edges.  The relationship is encapsulated in the
    // "edge_under_at_vert" variable, which says whether the
    // edge passed in is in the under state at this vertex.
    // The whole trick is to manage how neighbours receive modifications
    // of edge_under_at_vert.
    void propagate(VertexPtr vertex, EdgePtr edge, bool edge_under_at_vert, QStack<EdgePtr> &todo );

    // Propagate the over-under relation from an edge to its incident vertices.
    void buildFrom(QStack<EdgePtr> &todo );
    void initializeMap(constMapPtr map );
    void finalizeMap(constMapPtr map );
    void assignInterlacing(constMapPtr map );

    // Parameters of the rendering.
    qreal  gap;
    qreal  shadow;
    bool   includeTipVertices;

    // Internal representations of the rendering.
    QVector<QPointF>    pts;
    QVector<bool>       shadows;
};
#endif

