
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

eStyleType TileColors::getStyleType() const
{
    return STYLE_TILECOLORS;
}

QString TileColors::getStyleDesc() const
{
    return "Tile Colors";
}

void TileColors::resetStyleRepresentation()
{
    _coloredPlacements.clear();
    styled = false;
}

void TileColors::createStyleRepresentation()
{
    if (styled)
    {
        return;
    }

    qInfo() << "TileColors::createStyleRepresentation()";

    adjustColorGroupSize();

    TilingPtr tiling          = prototype->getTiling();
    QVector<TilePtr> uniques  = tiling->unit().getUniqueTiles();

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
        ColorSet * cset     = colorGroup.getColorSet(tileIndex);

        // this is twitchy code - do not move
        for (int i=0; i < placements.size(); i++)
        {
            // fetch tile colors and store
            QTransform & t = placements[i];
            EdgePoly  ep2  = ep.map(t);
            QColor color   = cset->getTPColor(i).color;
            ColoredPlacement pl(ep2, color);
            _coloredPlacements << pl;
        }
    }
    styled = true;
}

void TileColors::adjustColorGroupSize()
{
    TilingPtr tiling          = prototype->getTiling();
    QVector<TilePtr> uniques  = tiling->unit().getUniqueTiles();
    int numUniques            = uniques.size();

    if (colorGroup.size() == 0)
    {
        colorGroup = tiling->legacyTileColors();
        if (colorGroup.size() == 0)
        {
            qWarning() << "TileColors:: empty ColorGroup - addding" << numUniques << "default ColorSets";
            ColorSet cset;
            cset.addColor(TPColor());
            for (int i = 0; i < numUniques; i++)
            {
                colorGroup.addColorSet(cset);
            }
        }
        else
            qInfo() << "Loaded tile colors from legacy tiling";
    }

    if (numUniques > colorGroup.size())
    {
        int diff = numUniques - colorGroup.size();
        qWarning() << "adding" << diff << "ColorSets";
        ColorSet cset;
        for (int i = 0; i < diff; i++)
        {
            colorGroup.addColorSet(cset);
        }
    }
    else if (numUniques < colorGroup.size())
    {
        int diff = colorGroup.size() - numUniques;
        qWarning() << "removing" << diff << "ColorSets";
        colorGroup.resize(numUniques);
    }

    Q_ASSERT(colorGroup.size() == uniques.size());
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

    QPen pen2(outlineColor,outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (ColoredPlacement & coloredPlacement : _coloredPlacements)
    {
        QPen pen(coloredPlacement.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(coloredPlacement.epoly,pen);

        if (outlineEnb)
        {
            gg->drawEdgePoly(coloredPlacement.epoly,pen2);
        }
    }
    painter->restore();
}
