#pragma once
#ifndef CANVAS_MODEL_H
#define CANVAS_MODEL_H

#include <QTransform>
#include <QColor>
#include "sys/geometry/bounds.h"

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

    void            initCanvasSize(QSize size);
    QSize           getSize() const                     { return _canvasSize + _deltaSize; }
    void            setDeltaCanvasSize(QSize size);

    Bounds          getBounds() const                   { return _bounds; }
    void            setBounds(Bounds & b)               { _bounds = b; computeCanvasTransform(); }

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

    QSize           _canvasSize;
    QSize           _deltaSize;
    QSize           _defaultSize;
    qreal           _scale;

    eAlignment      _alignment;
};


#endif
