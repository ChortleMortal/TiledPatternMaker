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

#ifndef STYLE_H
#define STYLE_H

#include "tapp/Prototype.h"
#include "geometry/Map.h"
#include "viewers/GeoGraphics.h"
#include "base/shared.h"
#include "base/layer.h"

enum eStyleType
{
    STYLE_FILLED,
    STYLE_EMBOSSED,
    STYLE_INTERLACED,
    STYLE_OUTLINED,
    STYLE_PLAIN,
    STYLE_SKETCHED,
    STYLE_STYLE,
    STYLE_THICK,
    STYLE_TILECOLORS,
};



////////////////////////////////////////////////////////////////////////////
//
// Style.java
//
// A style encapsulates drawing a map with some interesting style.

class Style : public Layer
{
    Q_OBJECT

public:
    // Creation.
    Style(PrototypePtr proto,PolyPtr bounds);
    Style(const Style  & other);

    ~Style() override;

    // Geometry data.
    PrototypePtr getPrototype() const {return prototype;}
    void         setPrototype(PrototypePtr pp);
    PolyPtr      getBoundary() const {return boundary;}

    MapPtr       getMap()          { return styleMap; }

    // Retrieve a name describing this style and map.
    QString getDescription() const;
    QString getInfo() const;

    // Overridable behaviours

    virtual void createStyleRepresentation() = 0; // Called to ensure there is an internal map representation, if needed.
    virtual void resetStyleRepresentation()  = 0; // Called when the map is changed to clear any internal map representation.

    virtual QString     getStyleDesc() const = 0; // Retrieve the style description.
    virtual eStyleType  getStyleType() const = 0;

    virtual void    draw(GeoGraphics * gg) = 0;
    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    static int refs;

protected:
    MapPtr setupStyleMap();  // copies shared pointer from prototype
    void   resetStyleMap() { styleMap.reset(); }

    PrototypePtr  prototype; // The input geometry to be rendered
    PolyPtr       boundary;

private:
    MapPtr        styleMap;

};
#endif
