#pragma once
#ifndef CANVAS_MODEL_H
#define CANVAS_MODEL_H

#include <QTransform>
#include <QColor>
#include "sys/geometry/bounds.h"

/*
 * The canvas transform converts any point in model space into a point in screen (pixel) space,
 * and it's invert converts from screen to model
 * Model space is defined by Bounds which although can be varied, in this application are so far fixed.
 * Model space has a width of 20.0  and it's top left coordinate is defined as (-10.0, 10.0) and from
 * this a rectangle can be defined using an aspecst ratio whuich is the same as the aspect ratrio of
 * the screen view
 */

class Canvas
{
    friend class SystemViewController;

public:
    Canvas();

    void            setDefaultSize();

    QColor          getBkgdColor() const                { return _bkgdColor; }

    void            setCanvasSize(QSize size);
    QSize           getCanvasSize() const               { return _canvasSize + _deltaSize; }
    void            setDeltaCanvasSize(QSize size);

    void            setViewSize(QSize size)             { _viewSize = size; }
    QSize           getViewSize()                       { return _viewSize; }

#ifdef VARIABLE_BOUNDS
    Bounds          getBounds() const                   { return _bounds; }
    void            setBounds(Bounds & b)               { _bounds = b; computeCanvasTransform(); }
#endif

    QTransform      getTransform() const                { return _canvasTransform; }
    QPointF         getCenter();

    void            dump();

protected:
    void            computeCanvasTransform();

private:
    void            setBkgdColor(QColor color)          { _bkgdColor = color; }

    QTransform      _canvasTransform;

    QColor          _bkgdColor;

    Bounds          _bounds;
#ifdef VARIABLE_BOUNDS
    Bounds          _defaultBounds;
#endif
    QSize           _viewSize;    // the main window size
    QSize           _canvasSize;
    QSize           _deltaSize;
    QSize           _defaultSize;
};


#endif
