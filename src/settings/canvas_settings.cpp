#include <QDebug>
#include <QRectF>

#include "settings/canvas_settings.h"
#include "misc/sys.h"

CanvasSettings::CanvasSettings()
{
    _viewSize   = QSize( Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _canvasSize = QSizeF(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);   // default
    _bkgdColor  = (Sys::isDarkTheme) ? QColor(Qt::black) : QColor(Qt::white);
    _startTile  = QPoint(0,0);
}

CanvasSettings::CanvasSettings(const CanvasSettings & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _bkgdColor  = other._bkgdColor;
    _startTile  = other._startTile;
    _fillData   = other._fillData;
}

CanvasSettings::~CanvasSettings()
{}

CanvasSettings & CanvasSettings::operator=(const CanvasSettings & other)
{
    _viewSize   = other._viewSize;
    _canvasSize = other._canvasSize;
    _bkgdColor  = other._bkgdColor;
    _startTile  = other._startTile;
    _fillData   = other._fillData;

   return *this;
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

QPointF CanvasSettings::getCenter()
{
    QRectF r(QPoint(0,0),_viewSize);   // FIXME - use canvas size? But only used by legacy designs
    return r.center();
}



