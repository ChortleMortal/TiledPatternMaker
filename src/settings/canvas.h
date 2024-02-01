#pragma once
#ifndef CANVAS_MODEL_H
#define CANVAS_MODEL_H

#include <QTransform>
#include <QColor>
#include "geometry/bounds.h"
#include "settings/filldata.h"

enum eAlignment
{
    M_ALIGN_NONE,
    M_ALIGN_MOSAIC,
    M_ALIGN_TILING,
};

class Canvas
{
    friend class ViewController;

public:
    Canvas();

    void            reInit();

    QColor          getBkgdColor() const                { return _bkgdColor; }

    void            initCanvasSize(QSizeF size);
    QSizeF          getSize() const                     { return _canvasSize + _deltaSize; }
    QSizeF          getBaseSize() const                 { return _canvasSize; }
    QSizeF          getDeltaSize() const                { return _deltaSize; }

    Bounds          getBounds() const                   { return _bounds; }
    void            setBounds(Bounds & b)               { _bounds = b; computeCanvasTransform(); }

    void            setDeltaCanvasSize(QSizeF size);

    void            setFillData(FillData & fd)          { _fillData = fd; }
    FillData        getFillData()                       { return _fillData; }

    void            setModelAlignment(eAlignment mode)  { _alignment = mode; }
    eAlignment      getModelAlignment() const           { return _alignment; }

    QTransform      getCanvasTransform() const          { return _canvasTransform; }
    QTransform      getInvertedCanvasTransform() const  { return _canvasInverted; }
    qreal           getScale()                          { return _scale; }

protected:
    void            computeCanvasTransform();

private:
    void            setBkgdColor(QColor color)          { _bkgdColor = color; }

    QTransform      _canvasTransform;
    QTransform      _canvasInverted;

    QColor          _bkgdColor;

    Bounds          _bounds;
    Bounds          _defaultBounds;

    QSizeF           _canvasSize;
    QSizeF           _deltaSize;
    QSizeF           _defaultSize;
    qreal            _scale;

    FillData        _fillData;
    eAlignment      _alignment;
};


#endif
