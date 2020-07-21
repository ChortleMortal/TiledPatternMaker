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

#ifndef SHAPEVIEWER_H
#define SHAPEVIEWER_H

#include <QPainterPath>
#include "base/layer.h"


class Polyform;
class Configuration;

class ShapeViewer : public Layer
{
public:
    ShapeViewer();

    void addPolyform(Polyform * p) { polyforms.push_back(p); }

    QVector<Polyform*> & getPolyforms() { return polyforms;}

    void enableAntialiasing(bool enable) { _antiAliasPolys = enable; }

protected:
    void    paint(QPainter *painter) override;

    QVector<Polyform *>  polyforms;

    bool            _antiAliasPolys;
    QPainterPath    ppath;
    QPen            ppPen;
    QBrush          ppBrush;

private:
    Configuration * config;
};

#endif // SHAPEVIEWER_H
