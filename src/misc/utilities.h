#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QPointF>
#include <QSize>

class Utils
{
public:
    static QString  addr(void * address);
    static QString  addr(const void * address);
    static QString  str(QPointF pt);
    static QString  str(QSize sz);
    static qreal    aspect(QSize  sz) { return (qreal)sz.width()/(qreal)sz.height(); }
    static qreal    aspect(QSizeF sz) { return        sz.width()/       sz.height(); }
};

#endif
