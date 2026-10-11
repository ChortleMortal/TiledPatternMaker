#pragma once

#include <QTransform>
#include <QColor>

#include "model/settings/filldata.h"
#include "model/settings/distortion.h"
#include "sys/geometry/bounds.h"

/*
 * The canvas transform converts any point in model space into a point in screen (pixel) space,
 * and it's invert converts from screen to model
 * Model space is defined by Bounds which although can be varied, in this application are so far fixed.
 * Model space has a width of 20.0  and it's top left coordinate is defined as (-10.0, 10.0) and from
 * this a rectangle can be defined using an aspecst ratio whuich is the same as the aspect ratrio of
 * the screen view
 */

#define OBSOLETE_CANVAS // 03MAY26 system view has shared pointer to controlling canvas

class Canvas
{
    friend class ViewController;

public:
    Canvas();
    Canvas(const Canvas & other);

    Canvas & operator=  (const Canvas & other);
    bool     operator== (const Canvas & other) const;
    bool     operator!= (const Canvas & other) const { return !(*this == other); }

    void            init();
    void            setDefaultSize();

    void            setCanvasSize(QSize size);
    QSize           getCanvasSize() const               { return _canvasSize + _deltaCanvasSize; }
    QSize           getRawCanvasSize() const            { return _canvasSize; }

    void            setDeltaCanvasSize(QSize size);
    QSize           getDeltaCanvasSize() const          { return _deltaCanvasSize; }

    void            setViewSize(QSize size)             { _viewSize = size; }
    QSize           getViewSize() const                 { return _viewSize; }

    void            setFillData(FillData & fd)          { _fillData = fd; }
    FillData        getFillData() const                 { return _fillData; };

    void            setDistortion(Distortion & d)       { _distortion = d; computeCanvasTransform(); }
    Distortion      getDistortion() const               { return _distortion; };

#ifdef VARIABLE_BOUNDS
    Bounds          getBounds() const                   { return _bounds; }
    void            setBounds(Bounds & b)               { _bounds = b; computeCanvasTransform(); }
#endif

    QTransform      getTransform() const                { return _canvasTransform; }
    QTransform      getUndistortedTransform() const     { return _undistortedTransform; }
    QTransform      getDistorionTransform() const       { return _distortion.getMode() == DISTORT_TILE ? _distortion.getTransform() : QTransform(); }
    QPointF         getCenter();
    QPointF         getViewCenter();

    void            dump() const;

protected:
    void            computeCanvasTransform();

private:
    QTransform      _canvasTransform;
    QTransform      _undistortedTransform;   // same as _canvasTransform, but excludes distortion - used by borders and background images
    Bounds          _bounds;
#ifdef VARIABLE_BOUNDS
    Bounds          _defaultBounds;
#endif
    QSize           _viewSize;    // the main window sizeit builds
    QSize           _canvasSize;
    QSize           _deltaCanvasSize;
    FillData        _fillData;
    Distortion      _distortion;
};