#include "misc/utilities.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsItem>
#include <QtMath>

QString Utils::addr(void * address)
{
  return QString::number(reinterpret_cast<uint64_t>(address),16);
}

QString Utils::addr(const void * address)
{
  return QString::number(reinterpret_cast<uint64_t>(address),16);
}

QString Utils::str(QPointF pt)
{
    return QString("(%1,%2)").arg(QString::number(pt.x()),QString::number(pt.y()));
}

QString Utils::str(QSize sz)
{
    return QString("%1 x %2").arg(QString::number(sz.width()),QString::number(sz.height()));
}


