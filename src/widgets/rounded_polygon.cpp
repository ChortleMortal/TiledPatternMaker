#include "widgets/rounded_polygon.h"
#include <QDebug>

//    Taken from:
// https://www.toptal.com/c-plus-plus/rounded-corners-bezier-curves-qpainter

const QPainterPath& RoundedPolygon::GetPath()
{
    m_path = QPainterPath();

    if (count() < 3)
    {
        qWarning() << "Polygon should have at least 3 points!";
        return m_path;
    }

    QPointF pt1;
    QPointF pt2;
    for (int i = 0; i < count(); i++)
    {
        pt1 = GetLineStart(i);

        if (i == 0)
            m_path.moveTo(pt1);
        else
            m_path.quadTo(at(i), pt1);

        pt2 = GetLineEnd(i);
        m_path.lineTo(pt2);
    }

    // close the last corner
    pt1 = GetLineStart(0);
    m_path.quadTo(at(0), pt1);

    return m_path;
}

QPointF RoundedPolygon::GetLineStart(int i) const
{
    QPointF pt1 = at(i);
    QPointF pt2 = at((i+1) % count());
    qreal fRat = m_iRadius / GetDistance(pt1, pt2);
    if (fRat > 0.5f)
        fRat = 0.5f;

    QPointF pt;
    pt.setX((1.0f-fRat)*pt1.x() + fRat*pt2.x());
    pt.setY((1.0f-fRat)*pt1.y() + fRat*pt2.y());

    return pt;
}

QPointF RoundedPolygon::GetLineEnd(int i) const
{
    QPointF pt1 = at(i);
    QPointF pt2 = at((i+1) % count());
    qreal fRat = m_iRadius / GetDistance(pt1, pt2);
    if (fRat > 0.5f)
        fRat = 0.5f;

    QPointF pt;
    pt.setX(fRat*pt1.x() + (1.0f - fRat)*pt2.x());
    pt.setY(fRat*pt1.y() + (1.0f - fRat)*pt2.y());

    return pt;
}

qreal RoundedPolygon::GetDistance(QPointF pt1, QPointF pt2) const
{
    qreal fD = (pt1.x() - pt2.x())*(pt1.x() - pt2.x()) +
               (pt1.y() - pt2.y())*(pt1.y() - pt2.y());
    return sqrtf(fD);
}

#if 0
#include <QPixmap>
#include <QPainter>
void test()
{
    QPixmap pix1(300, 200);
    QPixmap pix2(300, 200);
    pix1.fill(Qt::white);
    pix2.fill(Qt::white);
    QPainter P1(&pix1);
    QPainter P2(&pix2);

    P1.setRenderHints(QPainter::Antialiasing);
    P2.setRenderHints(QPainter::Antialiasing);
    P1.setPen(QPen(Qt::blue, 2));
    P1.setBrush(Qt::red);

    P2.setPen(QPen(Qt::blue, 2));
    P2.setBrush(Qt::red);

    RoundedPolygon poly;

    poly << QPoint(147, 187) << QPoint(95, 187)
       << QPoint(100, 175) << QPoint(145, 165) << QPoint(140, 95)
       << QPoint(5, 85) << QPoint(5, 70) << QPoint(140, 70) << QPoint(135, 45)
       << QPoint(138, 25) << QPoint(145, 5) << QPoint(155, 5) << QPoint(162, 25)
       << QPoint(165, 45) << QPoint(160, 70) << QPoint(295, 70) << QPoint(295, 85)
       << QPoint(160, 95) << QPoint(155, 165) << QPoint(200, 175)
       << QPoint(205, 187) << QPoint(153, 187) << QPoint(150, 199);

    P1.drawPolygon(poly);
    P2.drawPath(poly.GetPath());

    pix1.save("1.png");
    pix2.save("2.png");
}
#endif
