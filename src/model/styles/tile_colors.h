#pragma once
#ifndef TILE_COLOR_H
#define TILE_COLOR_H

#include <QColor>
#include <QDebug>
#include "model/styles/style.h"
#include "sys/geometry/edge_poly.h"
#include "model/styles/colorset.h"
#include "model/tilings/placed_tile.h"

typedef std::weak_ptr<PlacedTile> wPlacedPtr;

class ColoredPlacement
{
public:
    ColoredPlacement(EdgePoly & ep, QColor col) { epoly = ep; color = col; }

    EdgePoly    epoly;
    QColor      color;
};

class TileColors : public Style
{
public:
    TileColors(ProtoPtr proto);
    TileColors(StylePtr other);
    virtual ~TileColors();

    void            draw(GeoGraphics * gg) override;

    void            resetStyleRepresentation() override;
    void            createStyleRepresentation() override;

    ColorGroup &    getColorGroup()                { return colorGroup; }
    void            setColorGroup(ColorGroup & cg) { colorGroup = cg; }
    ColorSet *      getColorSet(int index)         { return &colorGroup[index % colorGroup.size()]; }

    void            setOutline(bool enable,QColor color = Qt::white, int width = 3);
    bool            getOutline(QColor & color, int & width);

    eStyleType      getStyleType() const override;
    QString         getStyleDesc() const override;
    void            dump()         const override { qDebug().noquote() << getStyleDesc() << "outline:" << outlineEnb  << "width:" << outlineWidth << "color:" << outlineColor; }

protected:
    void            adjustColorGroupSize();

    // defined (XML r/w and used by menu)
    ColorGroup      colorGroup;
    bool            outlineEnb;
    QColor          outlineColor;
    int             outlineWidth;

private:
    QVector<ColoredPlacement> _coloredPlacements;   // calculated - color for each placed tile
};
#endif

