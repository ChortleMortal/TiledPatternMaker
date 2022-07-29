#ifndef TILE_COLOR_H
#define TILE_COLOR_H

#include <QColor>
#include "style/style.h"
#include "geometry/edgepoly.h"

struct bkgdEPolyColor
{
    EdgePoly  epoly;
    QColor    color;
};

class TileColors : public Style
{
public:
    TileColors(PrototypePtr proto);
    TileColors(StylePtr other);
    ~TileColors() override;

    void        createStyleRepresentation() override;
    void        resetStyleRepresentation() override;

    void        setOutline(bool enable,QColor color = Qt::white, int width = 3);
    bool        getOutline(QColor & color, int & width);

    eStyleType  getStyleType() const override;
    QString     getStyleDesc() const override;
    void        draw(GeoGraphics * gg) override;

protected:
    QVector<bkgdEPolyColor>  epolys;
    bool        outlineEnb;
    QColor      outlineColor;
    int         outlineWidth;

};
#endif

