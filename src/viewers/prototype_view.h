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

#ifndef DESIGN_PREVIEW2
#define DESIGN_PREVIEW2

#include <QtCore>
#include "geometry/fill_region.h"
#include "base/layer.h"
#include "tapp/prototype.h"

class PrototypeView : public FillRegion, public Layer
{
    enum eProtoMode
    {
        PROTO_DRAW_MAP       =  0x01,
        PROTO_DRAW_FEATURES  =  0x02,
        PROTO_DRAW_FIGURES   =  0x04
    };

public:
    PrototypeView(PrototypePtr proto, int mode);

    PrototypePtr getPrototype() {return proto; }

    virtual void   paint(QPainter *painter) override;
    void           receive(GeoGraphics * gg,int h, int v ) override;
    virtual void   draw( GeoGraphics * gg );

protected:
    int             mode;
    QPointF         t1;
    QPointF         t2;

    PrototypePtr    proto;

    EdgePoly        edges;              // this is not really an EdgePoly it is a vector of Edges
    QColor          feature_interior;
    QColor          feature_border;

    QVector<PlacedDesignElement> rpfs;
};
#endif
