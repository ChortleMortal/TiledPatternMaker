#pragma once
#ifndef BORDER_H
#define BORDER_H

#include "geometry/crop.h"
#include "geometry/faces.h"

typedef std::shared_ptr<class Map> MapPtr;

#define LENGTH1        60.0
#define BORDER_ZLEVEL  10

class Border : public Crop
{
//    Q_OBJECT

public:
    virtual void  construct() = 0;
    virtual void  draw(QPainter * painter, QTransform t) = 0 ;
    virtual void  draw(QPainter * painter, QTransform t, bool active) override { Q_UNUSED(painter); Q_UNUSED(t); Q_UNUSED(active); }

    eBorderType  getBorderType()       { return borderType; }
    QString      getBorderTypeString() { return sBorderType[borderType]; }
    QString      getCropTypeString()   { return sCropType[_cropType]; }

    void    setWidth(qreal width) { this->width = width; construct(); }
    void    setColor(QColor color){ this->color = color; construct(); }
    void    setColor2(QColor color) { color2 = color;  construct(); }

    qreal   getWidth()  { return width; }
    QColor  getColor()  { return color; }
    QColor  getColor2() { return color2; }

public slots:
    void    resize(QSize oldSize, QSize newSize);

protected:
    Border();
    Border(CropPtr crop);

    eBorderType     borderType;

    qreal           width;
    QColor          color;
    QColor          color2;

};

class BorderPlain : public Border
{
public:
    BorderPlain(CropPtr crop,   qreal width, QColor color);
    BorderPlain(Circle c,       qreal width, QColor color);
    BorderPlain(QRectF rect,    qreal width, QColor color);
    BorderPlain(QPolygonF poly, qreal width, QColor color);
    BorderPlain(QSizeF sz,      qreal width, QColor color);

    void construct() override;
    virtual void  draw(QPainter * painter, QTransform t) override;

    void get(qreal & width, QColor & color);

    MapPtr  bmap;
};

class BorderTwoColor : public Border
{
public:
    BorderTwoColor(QSizeF sz,   QColor color1, QColor color2, qreal width);
    BorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width);

    void    construct() override;
    virtual void  draw(QPainter * painter, QTransform t) override;

    void    get(QColor & color1, QColor & color2, qreal & width);

protected:
    QColor  nextBorderColor();

private:
    FaceSet faces;
};

class BorderBlocks : public Border
{
public:
    BorderBlocks(QSizeF sz,   QColor color, qreal width, int rows, int cols);
    BorderBlocks(QRectF rect, QColor color, qreal width, int rows, int cols);

    void    construct() override;
    virtual void  draw(QPainter * painter, QTransform t) override;


    void    get(QColor & color1, qreal & diameter, int & rows, int & cols);
    void    setRows(int rows)   { this->rows = rows; construct(); }
    void    setCols(int cols)   { this->cols = cols; construct(); }

protected:

private:
    int         rows;
    int         cols;
    FaceSet     faces;
};
#endif

