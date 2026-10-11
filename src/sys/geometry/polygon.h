#pragma once

#include <QPolygonF>
#include <QPointF>

class APolygon
{
public:
    APolygon();
    APolygon(QPolygonF & poly);
    APolygon(int sides, qreal rotDegrees, qreal scale);
    APolygon(const APolygon & other);

    APolygon & operator=(const APolygon & other);

    QPolygonF get();

    void    set(QPolygonF & poly)       { source = poly; hasSource = true; }
    void    setSides(int n)             { sides = n; hasSource = false; };
    void    setRotate(qreal degrees)    { rot = degrees; }
    void    setScale(qreal scale)       { this->scale = scale; }
    void    setPos(QPointF mpt)         { pos = mpt; }

    int     getSides()                  { return sides; }
    qreal   getRotate()                 { return rot; }
    qreal   getScale()                  { return scale; }
    QPointF getPos()                    { return pos; }
    bool    usesSource()                { return hasSource; }
    QPolygonF getSource()               { return source; }

private:
    QPointF     pos;        // model units
    int         sides;
    qreal       rot;        // degrees
    qreal       scale;
    QPolygonF   source;
    bool        hasSource;
};
