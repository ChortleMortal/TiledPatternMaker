#pragma once
#ifndef CANVAS_SETTINGS_H
#define CANVAS_SETTINGS_H

#include <QSize>
#include <QColor>
#include <QPointF>
#include "model/settings/filldata.h"

class CanvasSettings
{
public:
    CanvasSettings();
    CanvasSettings(const CanvasSettings & other);
    ~CanvasSettings();

    CanvasSettings & operator=(const CanvasSettings & other);
    bool             operator== (const CanvasSettings & other) const;

    QColor          getBackgroundColor() const;
    void            setBackgroundColor(QColor color);

    void            setViewSize(QSize size)     { _viewSize = size; }
    QSize           getViewSize() const         { return _viewSize; }

    void            setCanvasSize(QSize size)   { _canvasSize = size; }
    QSize           getCanvasSize() const       { return _canvasSize; }

    void            setStartTile(QPointF pt)    { _startTile = pt;  }
    QPointF         getStartTile() const        { return _startTile;}

    void            setFillData(FillData & fd)  { _fillData = fd; }
    FillData        getFillData() const         { return _fillData; };

    QPointF         getCenter();

protected:

private:
    QSize           _viewSize;
    QSize           _canvasSize;
    QColor          _bkgdColor;
    QPointF         _startTile;
    FillData        _fillData;
};

#endif
