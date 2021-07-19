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

#include "style/tile_colors.h"
#include "tile/placed_feature.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"
#include "base/colorset.h"
#include "tile/feature.h"
#include "base/geo_graphics.h"

TileColors::TileColors(PrototypePtr proto) : Style(proto)
{
    outlineEnb =  false;
    outlineColor = Qt::blue;
    outlineWidth = 3;
}

TileColors::TileColors(StylePtr other) : Style(other)
{
    outlineEnb = false;
    outlineColor = Qt::blue;
    outlineWidth = 3;
}

TileColors::~TileColors()
{}

void TileColors::setOutline(bool enable,QColor color, int width)
{
    outlineEnb      = enable;
    outlineColor = color;
    outlineWidth = width;
}

bool TileColors::getOutline(QColor & color, int & width)
{
    color = outlineColor;
    width = outlineWidth;
    return outlineEnb;
}

void TileColors::createStyleRepresentation()
{
    if (epolys.size() > 0)
    {
        return;
    }

    getMap();

    PrototypePtr pp  = getPrototype();
    TilingPtr    tp  = pp->getTiling();

    QVector<FeaturePtr> uniques  = tp->getUniqueFeatures();
    for (auto feature : uniques)
    {
        feature->getFeatureColors()->resetIndex();
    }

    QVector<QTransform> & translations  = pp->getTranslations();
    const QVector<PlacedFeaturePtr> & qlfp   = tp->getPlacedFeatures();
    qDebug() << "num placed features=" << qlfp.size();
    qDebug() << "num translations   =" << translations.size();

    for (auto it = qlfp.begin(); it != qlfp.end(); it++)
    {
        PlacedFeaturePtr fp = *it;
        FeaturePtr        f = fp->getFeature();
        const EdgePoly & ep = f->getEdgePoly();
        QTransform T1       = fp->getTransform();

        ColorSet * featureColors = f->getFeatureColors();
        for (auto& T2 : translations)
        {
            QTransform T3  = T1 * T2;
            EdgePoly  ep2  =  ep.map(T3);

            bkgdEPolyColor bpc;
            bpc.epoly  = ep2;
            bpc.color  = featureColors->getNextColor().color;
            epolys << bpc;
        }
    }
}

void TileColors::resetStyleRepresentation()
{
    epolys.clear();
    eraseStyleMap();
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

    for (auto bpc : epolys)
    {
        EdgePoly ep = bpc.epoly;
        gg->fillEdgePoly(&ep,bpc.color);
        if (outlineEnb)
        {
            gg->drawEdgePoly(&ep,outlineColor,outlineWidth);
        }
    }
}
