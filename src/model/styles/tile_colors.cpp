
#include "gui/top/system_view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/colorset.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/fill_region.h"

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

void TileColors::resetStyleRepresentation()
{
    colorSets.clear();
    coloredPlacements.clear();
    created = false;
}

void TileColors::createStyleRepresentation()
{
    if (created)
    {
        return;
    }

    qInfo() << "TileColors::createStyleRepresentation()";

    ProtoPtr  pp              = getPrototype();
    TilingPtr tiling          = pp->getTiling();
    QVector<TilePtr> uniques  = tiling->unit().getUniqueTiles();
    int unSize                = uniques.size();

    if (tileColors.size() == 0)
    {
        tileColors = tiling->legacyTileColors();
        if (tileColors.size() == 0)
        {
            qWarning() << "TileColors:: empty ColorGroup - addding" << unSize << "default ColorSets";
            ColorSet cset;
            for (int i = 0; i < unSize; i++)
            {
                tileColors.addColorSet(cset);
            }
        }
        else
            qInfo() << "Loaded tile colors from legacy tiling";
    }

    if (unSize > tileColors.size())
    {
        int diff = unSize - tileColors.size();
        qWarning() << "adding" << diff << "ColorSets";
        ColorSet cset;
        for (int i = 0; i < diff; i++)
        {
            tileColors.addColorSet(cset);
        }
    }
    else if (unSize < tileColors.size())
    {
        int diff = tileColors.size() - unSize;
        qWarning() << "removing" << diff << "ColorSets";
        tileColors.resize(unSize);
    }

    Q_ASSERT(tileColors.size() == uniques.size());

    tileColors.resetIndex();

    FillRegion flood(tiling.get(),getPrototype()->getMosaic()->getCanvasSettings().getFillData());
    Placements placements = flood.getPlacements(Sys::config->repeatMode);
    qDebug() << "num placements   =" << placements.size();

    const PlacedTiles tilingUnit = tiling->unit().getIncluded();
    qDebug() << "num placed tiles=" << tilingUnit.size();

    for (const PlacedTilePtr & placedTile : tilingUnit)
    {
        const EdgePoly   ep = placedTile->getPlacedEdgePoly();
        TilePtr tile        = placedTile->getTile();

        uint tileIndex      = uniques.indexOf(tile);
        ColorSet * cset     = tileColors.getColorSet(tileIndex);

        TileColorSet tcs(placedTile ,cset);
        colorSets << tcs;

        // this is twitchy code - reset needs to be done here - do not move
        cset->resetIndex();
        for (QTransform & t : placements)
        {
            // fetch tile colors and store
            QColor color   = cset->getNextTPColor().color;
            EdgePoly  ep2  = ep.map(t);
            ColoredPlacement pl(ep2, color);
            coloredPlacements << pl;
        }
    }
    created = true;
}

eStyleType TileColors::getStyleType() const
{
    return STYLE_TILECOLORS;
}

QString TileColors::getStyleDesc() const
{
    return "Tile Colors";
}

ColorSet * TileColors::getColorSet(PlacedTilePtr tile)
{
    static ColorSet emptyCset;

    for (auto & tcs : colorSets)
    {
        if (tcs.wtile.lock() == tile)
        {
            return tcs.cset;
        }
    }
    return &emptyCset;
}

ColorSet * TileColors::getColorSet(TilePtr tile)
{
    static ColorSet emptyCset;

    for (auto & tcs : colorSets)
    {
        auto placed = tcs.wtile.lock();
        if (placed && (placed->getTile() == tile))
        {
            return tcs.cset;
        }
    }
    return &emptyCset;
}

void TileColors::draw(GeoGraphics * gg)
{
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
                rect = modelToScreen(rect);
                qDebug() << "Crop rect screen" << rect;
                painter->setClipRect(rect);
            }
        }
    }

    for (auto & bpc : coloredPlacements)
    {
        QPen pen(bpc.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(bpc.epoly,pen);
        if (outlineEnb)
        {
            QPen pen2(outlineColor,outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            gg->drawEdgePoly(bpc.epoly,pen2);
        }
    }
    painter->restore();
}


ColoredPlacement::ColoredPlacement(EdgePoly & ep, QColor col)
{
    epoly = ep;
    color = col;
};

TileColorSet::TileColorSet(PlacedTilePtr tile, ColorSet * colorset)
{
    wtile = tile;
    cset  = colorset;
};
