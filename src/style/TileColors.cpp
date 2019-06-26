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

#include "style/TileColors.h"

TileColors::TileColors(PrototypePtr proto, PolyPtr bounds ) : Style(proto,bounds)
{
}

TileColors::TileColors(const Style *  other ) : Style(other)
{
}


TileColors::~TileColors()
{
}

void TileColors::createStyleRepresentation()
{
    if (polys.size() > 0)
    {
        return;
    }

    setupStyleMap();

    PrototypePtr pp                  = getPrototype();
    TilingPtr    tp                  = pp->getTiling();
    QVector<Transform> & locations   = pp->getLocations();
    QList<PlacedFeaturePtr> & qlfp   = tp->getPlacedFeatures();
    qDebug() << "num placed features=" << qlfp.size();
    qDebug() << "num locs=" << locations.size();

    for (auto it = qlfp.begin(); it != qlfp.end(); it++)
    {
        PlacedFeaturePtr fp = *it;
        FeaturePtr        f = fp->getFeature();
        QPolygonF poly      = f->getPoints();
        Transform T         = fp->getTransform();
        //qDebug().noquote() << "poly num pts=" << poly.size();
        //qDebug().noquote() << "T" << T.toString();

        ColorSet & bkgdColors = f->getBkgdColors();
        for (auto e = locations.begin(); e != locations.end(); e++)
        {
            Transform T2  = *e;
            //qDebug() << "T2" << T2.toString();
            T2.composeD(T);
            QMatrix matrix = T2.getQMatrix();

            QPolygonF p2 = matrix.map(poly);

            bkgdPolyColor bpc;
            bpc.poly   = p2;
            bpc.color  = bkgdColors.getNextColor().color;
            polys << bpc;

        }
    }
}

void TileColors::resetStyleRepresentation()
{
    polys.clear();
}

eStyleType TileColors::getStyleType() const
{
    return STYLE_TILECOLORS;
}

QString TileColors::getStyleDesc() const
{
    return "Tile Colors";
}

void TileColors::draw(GeoGraphics * gg)
{
    qDebug() << "TileColors::draw";

    if (!isVisible())
    {
        return;
    }

    gg->pushAndCompose(*getLayerTransform());

    for (auto it = polys.begin(); it != polys.end(); it++)
    {
        bkgdPolyColor bpc = *it;
        gg->setColor(bpc.color);
        gg->drawPolygon(bpc.poly,true);
    }
    gg->pop();

}
