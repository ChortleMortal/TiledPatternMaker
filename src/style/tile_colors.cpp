#include "style/tile_colors.h"
#include "geometry/fill_region.h"
#include "misc/colorset.h"
#include "misc/geo_graphics.h"
#include "mosaic/prototype.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"

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

    QVector<TilePtr> uniques  = tp->getUniqueTiles();
    for (auto tile : uniques)
    {
        tile->getTileColors()->resetIndex();
    }

    FillRegion flood(tp,ViewControl::getInstance()->getFillData());
    QVector<QTransform> translations = flood.getTransforms();
    qDebug() << "num translations   =" << translations.size();

    auto & placed = tp->getData().getPlacedTiles();
    qDebug() << "num placed tiles=" << placed.size();

    for (auto it = placed.begin(); it != placed.end(); it++)
    {
        PlacedTilePtr fp = *it;
        TilePtr        f = fp->getTile();
        const EdgePoly & ep = f->getEdgePoly();
        QTransform T1       = fp->getTransform();

        ColorSet * tileColors = f->getTileColors();
        for (auto& T2 : translations)
        {
            QTransform T3  = T1 * T2;
            EdgePoly  ep2  =  ep.map(T3);

            bkgdEPolyColor bpc;
            bpc.epoly  = ep2;
            bpc.color  = tileColors->getNextColor().color;
            epolys << bpc;
        }
    }
}

void TileColors::resetStyleRepresentation()
{
    epolys.clear();
    resetStyleMap();
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

    for (const auto & bpc : qAsConst(epolys))
    {
        EdgePoly ep = bpc.epoly;
        gg->fillEdgePoly(ep,bpc.color);
        if (outlineEnb)
        {
            gg->drawEdgePoly(ep,outlineColor,outlineWidth);
        }
    }
}
