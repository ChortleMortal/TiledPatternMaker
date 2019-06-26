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

#ifndef DACPATS_TILE_H
#define DACPATS_TILE_H

#include <QtCore>
#include <QtWidgets>
#include "layer.h"


#define TileBlue    "#39426d"
#define TileGreen   "#34554a"
#define TileBlack   "#10100e"
#define TileWhite   "#c2bcb0"
#define TileBrown   "#382310"
#define TileBrown2  "#632E1C"
#define TileGold    "#ffe05b"
#define AlhambraBrown "#a35807"
#define AlhambraBlue  "#1840b2"
#define AlhambraGreen "#234b30"
#define AlhambraGold  "#c59c0c"

class Tile : public QGraphicsItemGroup
{
public:
    Tile(int Row = 0, int Col = 0);
    ~Tile();

    void setTilePosition(int Row, int Col) {row = Row; col = Col;}

    void  setLoc(QPointF loc)       { setPos(loc); }
    void  setLoc(qreal x, qreal y)  { setPos(x,y); }
    QPointF getLoc() { return pos(); }

    int getRow() { return row;}
    int getCol() { return col;}

    QVector<Layer*> & getLayers() {return tileLayers;}
    Layer *  addLayer(qreal zLevel = 0.0);
    void     addLayer(Layer * layer, qreal zlevel = 0.0);
    Layer *  getLayer(int index);

    virtual bool doStep(int index);
    virtual void reset() {}

    void info();

    static int refs;

protected:
    qreal getSLinearPos(int step, int duration);

    bool odd (int i) { return ((i&1) == 1);}
    bool even(int i) { return ((i&1) == 0);}

    QVector<Layer*>   tileLayers;

    int     stepIndex;

    int     instance;
    int     row;
    int     col;

private:

};

#endif // TILE_H
