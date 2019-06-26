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

#include "tile.h"
#include "configuration.h"

int Tile::refs = 0;

Tile::Tile(int Row, int Col)
{
    row = Row;
    col = Col;

    instance = refs++;
}

Tile::~Tile()
{
    refs--;
}

Layer * Tile::addLayer(qreal zLevel)
{
    Layer * layer = new Layer("Tile");
    tileLayers.push_back(layer);
    layer->setZValue(zLevel);
    addToGroup(layer);
    return layer;
}

void Tile::addLayer(Layer * layer, qreal zlevel)
{
    tileLayers.push_back(layer);
    layer->setZValue(zlevel);
    //addToGroup(layer);    // is added by caller
}

Layer * Tile::getLayer(int index)
{
    int sz = tileLayers.size();
    if (index >= 0 && index < sz)
    {
        return tileLayers[index];
    }
    else
    {
        return nullptr;
    }

}

bool Tile::doStep(int index)
{
    qDebug() << "Tile::step" << index;
    return false;
}

qreal Tile::getSLinearPos(int step, int duration)
{
    if (step == 0)
    {
        return 0.0;
    }
    if (step == duration)
    {
        return 1.0;
    }
    // put position in range -6 to +6
    qreal x  = (qreal)step * (13.0/((qreal)duration)) - 6.0;
    qreal normalizedValue = 1.0 / (1 + exp(-x));
    return normalizedValue;
}

void Tile::info()
{
    for (int i=0; i < tileLayers.size(); i++)
    {
        qDebug() << "layer" << i << ":" << tileLayers[i]->childItems().count();
    }
}
