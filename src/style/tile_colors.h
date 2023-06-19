#pragma once
#ifndef TILE_COLOR_H
#define TILE_COLOR_H

#include <QColor>
#include <QDebug>
#include "style/style.h"
#include "geometry/edgepoly.h"

struct bkgdEPolyColor
{
    EdgePoly  epoly;
    QColor    color;
};


///
///  Tile Colors are stored in the tiling XML not the Mosaic XML
///


class TileColors : public Style
{
public:
    TileColors(ProtoPtr proto);
    TileColors(StylePtr other);
    virtual ~TileColors();

    void        draw(GeoGraphics * gg) override;

    void        createStyleRepresentation() override;
    void        resetStyleRepresentation() override;

    void        setOutline(bool enable,QColor color = Qt::white, int width = 3);
    bool        getOutline(QColor & color, int & width);

    eStyleType    getStyleType() const override;
    QString       getStyleDesc() const override;
    virtual void  report()       const override { qDebug().noquote() << getStyleDesc() << "outline:" << outlineEnb  << "width:" << outlineWidth << "color:" << outlineColor; }

protected:
    QVector<bkgdEPolyColor>  epolys;
    bool        outlineEnb;
    QColor      outlineColor;
    int         outlineWidth;

};
#endif

