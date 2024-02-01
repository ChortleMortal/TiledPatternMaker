#pragma once
#ifndef CANVAS_SETTINGS_H
#define CANVAS_SETTINGS_H

#include <QSize>
#include <QColor>
#include <QPointF>
#include "settings/filldata.h"

class CanvasSettings
{
public:
    CanvasSettings();
    CanvasSettings(const CanvasSettings & other);
    ~CanvasSettings();

    CanvasSettings & operator=(const CanvasSettings & other);

    QColor          getBackgroundColor() const;
    void            setBackgroundColor(QColor color);

    void            setViewSize(QSize size)     { _viewSize = size; }
    QSize           getViewSize() const         { return _viewSize; }

    void            setCanvasSize(QSizeF size)  { _canvasSize = size; }
    QSizeF          getCanvasSize() const       { return _canvasSize; }

    void            setStartTile(QPointF pt)    { _startTile = pt;  }
    QPointF         getStartTile() const        { return _startTile;}

    void            setFillData(FillData & fd)  { _fillData = fd; }
    FillData        getFillData() const         { return _fillData; };

    QPointF         getCenter();

protected:

private:
    QSize           _viewSize;
    QSizeF          _canvasSize;         // zoom size
    QColor          _bkgdColor;
    QPointF         _startTile;
    FillData        _fillData;
};

#endif
