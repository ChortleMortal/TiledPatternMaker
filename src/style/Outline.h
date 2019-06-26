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

#ifndef OUTLINE_H
#define OUTLINE_H

#include "style/Thick.h"

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
    Outline(PrototypePtr proto, PolyPtr bounds );
    Outline(const Style *  other );
    virtual ~Outline() override;

    void resetStyleRepresentation() override;
    void createStyleRepresentation() override;
    void draw(GeoGraphics *gg ) override;

    virtual eStyleType getStyleType() const override { return STYLE_OUTLINED; }
    QString getStyleDesc() const override { return "Outlined"; }

    // Static Helpers

    // Do a mitered join of the two fat lines (a la postscript, for example).
    // The join point on the other side of the joint can be computed by
    // reflecting the point returned by this function through the joint.
    static QPointF getJoinPoint(QPointF joint, QPointF a, QPointF b, qreal width );

    // Look at a given edge and construct a plausible set of points
    // to draw at the edge's 'to' vertex.  Call this twice to get the
    // complete outline of the hexagon to draw for this edge.
    static QVector<QPointF> getPoints(EdgePtr edge, VertexPtr from, VertexPtr to, qreal width );

protected:
    QVector<QPolygonF> pts3; // Internal representations of the rendering.

};
#endif

