#pragma once

#ifndef BORDER_H
#define BORDER_H

#include "geometry/crop.h"
#include "geometry/faces.h"

typedef std::shared_ptr<class Map> MapPtr;

class Border : public QObject, public Crop
{
    Q_OBJECT

public:
    virtual void  draw(QPainter * painter, QTransform t) = 0 ;

    eBorderType  getBorderType()       { return borderType; }
    QString      getBorderTypeString() { return sBorderType[borderType]; }
    QString      getCropTypeString()   { return sCropType[_cropType]; }
    bool         getUseViewSize()      { return _useViewSize; }

    void    setWidth(qreal width)       { borderWidth = width; setRequiresConstruction(true); }
    void    setColor(QColor color)      { this->color = color; setRequiresConstruction(true); }
    void    setColor2(QColor color)     { color2 = color;      setRequiresConstruction(true); }
    void    setUseViewSize(bool use)    { _useViewSize = use;  setRequiresConstruction(true); }

    qreal   getWidth()  { return borderWidth; }
    QColor  getColor()  { return color; }
    QColor  getColor2() { return color2; }

    virtual void legacy_convertToModelUnits() = 0;

    void    setRequiresConversion(bool enb)   { _requiresConversion = enb; }
    bool    getRequiresConversion()           { return _requiresConversion; }

    void    setRequiresConstruction(bool enb) { _requiresConstruction = enb; }
    bool    getRequiresConstruction()         { return _requiresConstruction; }

public slots:
    virtual void viewResized(QSize oldSize, QSize newSize);
    virtual void viewMoved();

protected:
    Border();
    Border(CropPtr crop);

    virtual void  construct() = 0;

    void            convertCropToModelUnits();
    void            setBorderSize(QSize viewSize);

    eBorderType     borderType;

    qreal           borderWidth;
    QColor          color;
    QColor          color2;
    bool            _useViewSize;

private:
    bool            _requiresConversion;
    bool            _requiresConstruction;
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

    void legacy_convertToModelUnits() override;

    MapPtr  bmap;
};

class BorderTwoColor : public Border
{
public:
    BorderTwoColor(QSizeF sz,   QColor color1, QColor color2, qreal width, qreal len);
    BorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width, qreal len);

    virtual void  draw(QPainter * painter, QTransform t) override;

    qreal   getLength()             { return segmentLen; }
    void    setLength(qreal length) { segmentLen = length;  construct(); }

    void    get(QColor & color1, QColor & color2, qreal & width, qreal &length);

    void    legacy_convertToModelUnits() override;

protected:
    void    construct() override;

    QColor  nextBorderColor();
    void    addSegment(qreal x, qreal y, qreal width, qreal height);

    qreal   segmentLen;

private:
    FaceSet faces;
};

class BorderBlocks : public Border
{
public:
    BorderBlocks(QSizeF sz,   QColor color, int rows, int cols, qreal width);
    BorderBlocks(QRectF rect, QColor color, int rows, int cols, qreal width);

    virtual void  draw(QPainter * painter, QTransform t) override;

    void    get(QColor & color1, int & rows, int & cols, qreal & width);
    void    setRows(int rows)   { this->rows = rows; construct(); }
    void    setCols(int cols)   { this->cols = cols; construct(); }

    void    legacy_convertToModelUnits() override;

protected:
    void    construct() override;

private:
    int         rows;
    int         cols;
    FaceSet     faces;
};
#endif

