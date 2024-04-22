#include "style/tile_colors.h"
#include "geometry/fill_region.h"
#include "geometry/crop.h"
#include "misc/colorset.h"
#include "misc/geo_graphics.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "mosaic/mosaic.h"
#include "tile/tile.h"

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

void TileColors::resetStyleRepresentation()
{
    colorSets.clear();
    coloredPlacements.clear();
}

void TileColors::createStyleRepresentation()
{
    if (coloredPlacements.size() > 0)
    {
        return;
    }

    ProtoPtr  pp              = getPrototype();
    TilingPtr tp              = pp->getTiling();
    QVector<TilePtr> uniques  = tp->getUniqueTiles();
    int unSize                = uniques.size();
    int tcSize                = tileColors.size();

    if (tileColors.size() == 0)
    {
        qWarning() << "TileColors:: empty ColorGroup - addding" << unSize << "default ColorSets";
        ColorSet cset;
        for (int i = 0; i < unSize; i++)
        {
            tileColors.addColorSet(cset);
        }
    }
    else if (unSize > tcSize)
    {
        int diff = unSize - tcSize;
        qWarning() << "adding" << diff << "ColorSets";
        ColorSet cset;
        for (int i = 0; i < diff; i++)
        {
            tileColors.addColorSet(cset);
        }
    }
    else if (unSize < tcSize)
    {
        int diff = tcSize - unSize;
        qWarning() << "removing" << diff << "ColorSets";
        tileColors.resize(unSize);
    }

    Q_ASSERT(tileColors.size() == uniques.size());

    tileColors.resetIndex();

    FillRegion flood(tp.get(),viewControl->getCanvas().getFillData());
    Placements placements = flood.getPlacements(config->repeatMode);
    qDebug() << "num placements   =" << placements.size();

    const auto & placedTiles = tp->getInTiling();
    qDebug() << "num placed tiles=" << placedTiles.size();

    for (const auto & placedTile : std::as_const(placedTiles))
    {
        TilePtr        tile = placedTile->getTile();
        uint tileIndex      = uniques.indexOf(tile);
        const EdgePoly & ep = tile->getEdgePoly();
        QTransform T1       = placedTile->getTransform();
        ColorSet * cset     = tileColors.getColorSet(tileIndex);

        TileColorSet tcs(tile,cset);
        colorSets << tcs;

        // this is twitchy code - reset needs to be done here - do not move
        cset->resetIndex();
        for (auto & T2 : std::as_const(placements))
        {
            // fetch tile colors and store
            QTransform T3  = T1 * T2;
            EdgePoly  ep2  =  ep.map(T3);
            QColor color   = cset->getNextTPColor().color;
            ColoredPlacement pl(ep2, color);
            coloredPlacements << pl;
        }
    }
}

eStyleType TileColors::getStyleType() const
{
    return STYLE_TILECOLORS;
}

QString TileColors::getStyleDesc() const
{
    return "Tile Colors";
}

ColorSet * TileColors::getColorSet(TilePtr tile)
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
                rect = modelToScreen(rect);
                qDebug() << "Crop rect screen" << rect;
                painter->setClipRect(rect);
            }
        }
    }

    for (const auto & bpc : std::as_const(coloredPlacements))
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


ColoredPlacement::ColoredPlacement(EdgePoly & ep, QColor col)
{
    epoly = ep;
    color = col;
};

TileColorSet::TileColorSet(TilePtr tile, ColorSet * colorset)
{
    wtile = tile;
    cset  = colorset;
};
