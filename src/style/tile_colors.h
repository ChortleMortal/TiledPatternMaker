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

#ifndef TILE_COLOR_H
#define TILE_COLOR_H

#include <QColor>
#include "style/style.h"
#include "geometry/edgepoly.h"

struct bkgdEPolyColor
{
    EdgePoly  epoly;
    QColor    color;
};

class TileColors : public Style
{
public:
    TileColors(PrototypePtr proto);
    TileColors(StylePtr other);
    ~TileColors() override;

    void        createStyleRepresentation() override;
    void        resetStyleRepresentation() override;

    void        setOutline(bool enable,QColor color = Qt::white, int width = 3);
    bool        getOutline(QColor & color, int & width);

    eStyleType  getStyleType() const override;
    QString     getStyleDesc() const override;
    void        draw(GeoGraphics * gg) override;

protected:
    QVector<bkgdEPolyColor>  epolys;
    bool        outlineEnb;
    QColor      outlineColor;
    int         outlineWidth;

};
#endif

