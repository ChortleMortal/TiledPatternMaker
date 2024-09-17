#include <QDebug>
#include "sys/geometry/measurement.h"
#include "gui/viewers/layer.h"

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
    wStart = layer->screenToModel(spt);
}

void Measurement::setEnd(QPointF spt)
{
    wEnd = layer->screenToModel(spt);

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
    return layer->modelToScreen(wStart);
}

QPointF Measurement::endS()
{
      return layer->modelToScreen(wEnd);
}

qreal Measurement::lenS()
{
    return QLineF(startS(),endS()).length();
}

qreal Measurement::lenW()
{
    return QLineF(wStart,wEnd).length();
}

