#include "style/tile_colors.h"
#include "geometry/fill_region.h"
#include "geometry/crop.h"
#include "misc/colorset.h"
#include "misc/geo_graphics.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "mosaic/mosaic.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"
#include "settings/configuration.h"

TileColors::TileColors(ProtoPtr proto) : Style(proto)
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
    outlineEnb   = enable;
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

    ProtoPtr  pp  = getPrototype();
    TilingPtr tp  = pp->getTiling();

    QVector<TilePtr> uniques  = tp->getUniqueTiles();
    for (const auto & tile : std::as_const(uniques))
    {
        tile->getTileColors()->resetIndex();
    }

    FillRegion flood(tp.get(),viewControl->getCanvas().getFillData());
    Placements placements = flood.getPlacements(config->repeatMode);
    qDebug() << "num placements   =" << placements.size();

    const auto & placedTiles = tp->getInTiling();
    qDebug() << "num placed tiles=" << placedTiles.size();

    for (const auto & placedTile : std::as_const(placedTiles))
    {
        TilePtr        tile = placedTile->getTile();
        const EdgePoly & ep = tile->getEdgePoly();
        QTransform T1       = placedTile->getTransform();

        // fetch tile colors from the tiling
        ColorSet * tileColors = tile->getTileColors();
        for (auto & T2 : std::as_const(placements))
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

    auto painter = gg->getPainter();
    painter->save();

    auto proto = getPrototype();
    if (proto)
    {
        auto crop = proto->getCrop();
        if (crop)
        {
            auto rect = crop->getRect();
            qDebug() << "Crop rect model" << rect;
            if (rect.isValid())
            {
                rect = worldToScreen(rect);
                qDebug() << "Crop rect screen" << rect;
                painter->setClipRect(rect);
            }
        }
    }

    for (const auto & bpc : std::as_const(epolys))
    {
        EdgePoly ep = bpc.epoly;
        QPen pen(bpc.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(ep,pen);
        if (outlineEnb)
        {
            QPen pen2(outlineColor,outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            gg->drawEdgePoly(ep,pen2);
        }
    }

    painter->restore();
}
