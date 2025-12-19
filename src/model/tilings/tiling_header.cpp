#include <QtWidgets>
#include "model/tilings/tiling_header.h"

using std::make_shared;

TilingHeader::TilingHeader(const TilingHeader & other)
{
    _t1             = other._t1;
    _t2             = other._t2;
    _settings       = other._settings;
}

TilingHeader::~TilingHeader()
{
}

TilingHeader & TilingHeader::operator=(const TilingHeader & other)
{
    _t1             = other._t1;
    _t2             = other._t2;
    _settings       = other._settings;
    return *this;
}

bool TilingHeader::operator==(const TilingHeader & other) const
{
    if (_t1 != other._t1)
        return false;
    if (_t2 != other._t2)
        return false;
    if ( _settings != other._settings)
        return false;

    return true;
}

TilingHeader TilingHeader::uniqueCopy()
{
    TilingHeader td;
    td._settings        = _settings;
    td._t1              = _t1;
    td._t2              = _t2;
    td._translateOrigin = _translateOrigin;
    return td;
}

void TilingHeader::setTranslationVectors(QPointF t1, QPointF t2, QPointF origin)
{
    _t1 = t1;
    _t2 = t2;
    _translateOrigin = origin;
}

void  TilingHeader::setCanvasSettings(const CanvasSettings & settings)
{
    _settings = settings;
}

bool TilingHeader::isEmpty()
{
    if (_t1.isNull() && _t2.isNull())
        return true;
    else
        return false;
}

Placements TilingHeader::getFillPlacements()
{
    Placements placements;
    const FillData & fd = _settings.getFillData();
    int minX,minY,maxX,maxY;
    bool singleton;
    fd.get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                QPointF pt   = (_t1 * static_cast<qreal>(h)) + (_t2 * static_cast<qreal>(v));
                QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
                placements << T;
            }
        }
    }
    else
    {
        placements << QTransform();
    }
    return placements;
}

QString TilingHeader::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "t1=" << _t1 << "t2=" << _t2;
    return astring;
}


