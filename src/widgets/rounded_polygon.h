#ifndef ROUNDED_POLYGON_H
#define ROUNDED_POLYGON_H

#include <QPolygonF>
#include <QPainterPath>

class RoundedPolygon : public QPolygonF
{
public:
    RoundedPolygon()  { SetRadius(10); }
    void SetRadius(unsigned int iRadius) {  m_iRadius = iRadius; }
    const QPainterPath& GetPath();

private:
    QPointF GetLineStart(int i) const;
    QPointF GetLineEnd(int i) const;
    qreal GetDistance(QPointF pt1, QPointF pt2) const;

private:
    QPainterPath m_path;
    unsigned int m_iRadius;
};

#endif // ROUNDED_POLYGON_H
