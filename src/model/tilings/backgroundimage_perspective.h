#pragma once
#ifndef BACKGROUNDIMAGE_PERSPECTIVE_H
#define BACKGROUNDIMAGE_PERSPECTIVE_H

#include <QTransform>
#include <QGraphicsPixmapItem>

#include "sys/geometry/edge_poly.h"

class BackgroundImagePerspective
{
public:
    BackgroundImagePerspective();

    bool startDragging(QPointF spt);
    bool updateDragging(QPointF spt );
    bool endDragging(QPointF spt );
    bool addPoint(QPointF spt);

    void drawPerspective(QPainter *painter );

    void       setSkewMode(bool enb);
    bool       getSkewMode() { return skewMode; }
    EdgeSet  & getAccum()    { return sAccum; }

protected:
    EdgeSet     sAccum;       // screen points

private:
    bool        skewMode;
    QPointF     spt;
    QPointF     sLastDrag;   // screen point
    QPolygonF   poly;
};


#endif
