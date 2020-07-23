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

////////////////////////////////////////////////////////////////////////////
//
// DesignPreview.java
//
// Boy, am I glad I thought of this class.  The design preview uses the
// same FillRegion algorithm as TilingViewer (used by TilingCard, for
// instance), to draw instead the _figures_ as they would appear in the
// final design.  Because we're not constructing the map, just drawing
// lines, previewing the design is really fast.  Plus, you can use the
// viewport of the preview window to indicate the region to fill for the
// final design -- I was searching for a way to let the user express
// this piece of information.
//
// This class just turns the translational unit into a collection of line
// segments and then draws them repeatedly to fill the window.

#ifndef DESIGN_PROTOVIEW
#define DESIGN_PROTOVIEW

#include <QtCore>
#include "geometry/fill_region.h"
#include "tapp/prototype.h"
#include "base/layer.h"

class ProtoView : public Layer
{
public:
    ProtoView(PrototypePtr proto);
    PrototypePtr getPrototype() { return proto; }

    virtual void   paint(QPainter *painter) override;
    virtual void   draw(GeoGraphics * gg);

protected:
    QPointF             t1;
    QPointF             t2;

    EdgePoly            edges;  // this is not really an EdgePoly it is a vector of Edges
    QVector<PlacedDesignElement> rpfs;

    PrototypePtr        proto;
};
#endif
