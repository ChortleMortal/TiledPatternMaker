#pragma once
#ifndef TILE_COLOR_H
#define TILE_COLOR_H

#include <QColor>
#include <QDebug>
#include "model/styles/style.h"
#include "sys/geometry/edgepoly.h"
#include "model/styles/colorset.h"
#include "model/tilings/placed_tile.h"

typedef std::weak_ptr<Tile> WeakTilePtr;

class ColoredPlacement
{
public:
    ColoredPlacement(EdgePoly & ep, QColor col);

    EdgePoly    epoly;
    QColor      color;
};

class TileColorSet
{
public:
    TileColorSet(TilePtr tile, ColorSet * colorset);

    WeakTilePtr wtile;
    ColorSet *  cset;
};

class TileColors : public Style
{
public:
    TileColors(ProtoPtr proto);
    TileColors(StylePtr other);
    virtual ~TileColors();

    void            draw(GeoGraphics * gg) override;

    void            createStyleRepresentation() override;
    void            resetStyleRepresentation() override;

    ColorGroup    & getTileColors() { return tileColors; }
    ColorSet *      getColorSet(TilePtr tile);

    void            setOutline(bool enable,QColor color = Qt::white, int width = 3);
    bool            getOutline(QColor & color, int & width);

    eStyleType      getStyleType() const override;
    QString         getStyleDesc() const override;
    void            dump()         const override { qDebug().noquote() << getStyleDesc() << "outline:" << outlineEnb  << "width:" << outlineWidth << "color:" << outlineColor; }

private:
    ColorGroup                tileColors;           // defined
    QVector<TileColorSet>     colorSets;            // calculated
    QVector<ColoredPlacement> coloredPlacements;    // calculated

    bool            outlineEnb;
    QColor          outlineColor;
    int             outlineWidth;

};
#endif

