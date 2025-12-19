#pragma once
#ifndef LEGACY_BORDER_H
#define LEGACY_BORDER_H

#include "gui/viewers/layer.h"
#include "sys/enums/eborder.h"

typedef std::shared_ptr<class ShapeFactory> ShapeFPtr;

#define LENGTH1        60.0

class LegacyBorder : public Layer
{
public:
    virtual void  construct() = 0;

    eBorderType  getBorderType()       { return borderType; }
    QString      getBorderTypeString() { return sBorderType[borderType]; }

    void    setRect(QRectF rect) { _rect = rect; construct(); }
    void    setWidth(qreal width) { this->width = width; construct(); }
    void    setColor(QColor color){ this->color = color; construct(); }
    void    setColor2(QColor color) { color2 = color;  construct(); }

    qreal   getWidth()  { return width; }
    QColor  getColor()  { return color; }
    QColor  getColor2() { return color2; }

protected:
    LegacyBorder();

    eBorderType     borderType;

    ShapeFPtr       sp;     // boders use shape factory and shape viewer to paint layer

    QRectF          _rect;
    qreal           width;
    QColor          color;
    QColor          color2;
};

class LegacyBorderTwoColor : public LegacyBorder
{
public:
    LegacyBorderTwoColor(QSizeF sz,   QColor color1, QColor color2, qreal width);
    LegacyBorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width);

    void    construct();

    void    get(QColor & color1, QColor & color2, qreal & width);

protected:
    QPen    nextBorderPen();
    QBrush  nextBorderBrush();

private:
};

class LegacyBorderBlocks : public LegacyBorder
{
public:
    LegacyBorderBlocks(QSizeF sz,   QColor color, qreal width, int rows, int cols);
    LegacyBorderBlocks(QRectF rect, QColor color, qreal width, int rows, int cols);

    void    construct();

    void    get(QColor & color1, qreal & diameter, int & rows, int & cols);
    void    setRows(int rows)   { this->rows = rows; construct(); }
    void    setCols(int cols)   { this->cols = cols; construct(); }

protected:

private:
    int         rows;
    int         cols;
};

#endif // LEGACY_BORDER_H
