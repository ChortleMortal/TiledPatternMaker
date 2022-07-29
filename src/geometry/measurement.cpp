#include <QDebug>
#include "geometry/measurement.h"
#include "misc/layer.h"

Measurement::Measurement(Layer *layer)
{
    qDebug() << "Measurement";
    this->layer = layer;
    active = false;
}

void Measurement::reset()
{
    wStart = QPointF();
    wEnd   = QPointF();
    active = false;
}
void Measurement::setStart(QPointF spt)
{
    wStart = layer->screenToWorld(spt);
}

void Measurement::setEnd(QPointF spt)
{
    wEnd = layer->screenToWorld(spt);

}

QPointF Measurement::startW()
{
    return wStart;
}

QPointF Measurement::endW()
{
    return wEnd;
}

QPointF Measurement::startS()
{
    return layer->worldToScreen(wStart);
}

QPointF Measurement::endS()
{
      return layer->worldToScreen(wEnd);
}

qreal Measurement::lenS()
{
    return QLineF(startS(),endS()).length();
}

qreal Measurement::lenW()
{
    return QLineF(wStart,wEnd).length();
}

