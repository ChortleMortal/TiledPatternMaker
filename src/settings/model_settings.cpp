#include <QDebug>
#include <QRectF>

#include "settings/model_settings.h"
#include "misc/defaults.h"

ModelSettings::ModelSettings()
{
    _size       = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);   // default
    _zsize      = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);   // default
    _bkgdColor  = QColor(Qt::white);
    _startTile  = QPoint(0,0);
}

ModelSettings::ModelSettings(const ModelSettings & other)
{
    _size      = other._size;
    _zsize     = other._zsize;
    _bkgdColor = other._bkgdColor;
    _startTile = other._startTile;
    _fillData  = other._fillData;
}

ModelSettings::~ModelSettings()
{}

ModelSettings & ModelSettings::operator=(const ModelSettings & other)
{
    _size      = other._size;
    _zsize     = other._zsize;
    _bkgdColor = other._bkgdColor;
    _startTile = other._startTile;
    _fillData  = other._fillData;

   return *this;
}

void ModelSettings::clear()
{
    _size        = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);
    _zsize       = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);
    _bkgdColor   = QColor(Qt::black);
    _startTile   = QPoint(0,0);
}

void ModelSettings::setSize(QSize size)
{
    _size = size;
}

QPointF ModelSettings::getStartTile()
{
    return _startTile;
}

void ModelSettings::setStartTile(QPointF pt)
{
    _startTile = pt;
}

QColor ModelSettings::getBackgroundColor()
{
    return _bkgdColor;
}

void  ModelSettings::setBackgroundColor(QColor color)
{
    qDebug() << "CanvasSettings::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}

QPointF ModelSettings::getCenter()
{
    QRectF r(QPoint(0,0),_size);
    return r.center();
}

void  ModelSettings::setFillData(const FillData & fd)
{
    _fillData = fd;
}

const FillData & ModelSettings::getFillDataAccess() const
{
    return _fillData;
}

FillData & ModelSettings::getFillData()
{
    return _fillData;
}



