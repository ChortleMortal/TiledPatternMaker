#pragma once

#include <QPointF>

class Layer;

class Measurement
{
public:
    Measurement(Layer * layer);

    void    reset();

    void    setStart(QPointF spt);
    void    setEnd(QPointF spt);

    QPointF startW();
    QPointF endW();
    QPointF startS();
    QPointF endS();

    qreal   lenS();
    qreal   lenW();

    bool    active;

private:
    Layer * layer;

    QPointF wStart;
    QPointF wEnd;
};
