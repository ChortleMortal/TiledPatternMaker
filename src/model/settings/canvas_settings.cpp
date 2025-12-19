#include <QDebug>
#include <QRectF>

#include "model/settings/canvas_settings.h"
#include "sys/sys.h"

CanvasSettings::CanvasSettings()
{
    _viewSize   = QSize( Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _canvasSize = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _bkgdColor  = (Sys::isDarkTheme) ? QColor(Qt::black) : QColor(Qt::white);
}

CanvasSettings::CanvasSettings(const CanvasSettings & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _bkgdColor  = other._bkgdColor;
    _fillData   = other._fillData;
}

CanvasSettings::~CanvasSettings()
{}

CanvasSettings & CanvasSettings::operator=(const CanvasSettings & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _bkgdColor  = other._bkgdColor;
    _fillData   = other._fillData;

   return *this;
}

bool CanvasSettings::operator==(const CanvasSettings & other) const
{
    if (_viewSize != other._viewSize) return false;
    if (_canvasSize != other._canvasSize) return false;;
    if (_bkgdColor  != other._bkgdColor) return false;
    if (_fillData   != other._fillData) return false;

    return true;
}

QColor CanvasSettings::getBackgroundColor() const
{
    return _bkgdColor;
}

void  CanvasSettings::setBackgroundColor(QColor color)
{
    qDebug() << "CanvasSettings::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}

QPointF CanvasSettings::getViewCenter()
{
    QRectF r(QPoint(0,0),_viewSize);
    return r.center();
}



