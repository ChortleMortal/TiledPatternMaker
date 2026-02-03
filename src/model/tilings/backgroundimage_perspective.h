#pragma once
#ifndef BACKGROUNDIMAGE_PERSPECTIVE_H
#define BACKGROUNDIMAGE_PERSPECTIVE_H

#include <QTransform>
#include <QGraphicsPixmapItem>

#include "gui/model_editors/crop_edit/mouse_edit_crop.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/edge_poly.h"

class BackgroundImagePerspective
{
public:
    BackgroundImagePerspective();

    bool startDragging(QPointF spt);
    bool updateDragging(QPointF spt );
    bool endDragging(QPointF spt );
    bool addPoint(QPointF spt);

    void draw(QPainter *painter );

    void       activate(bool enb);
    bool       isActive()     { return active; }

    EdgeSet  & getAccum()    { return sAccum; }

protected:
    void drawPerspective(QPainter *painter );

    EdgeSet     sAccum;       // screen points

private:
    bool        active;
    QPointF     spt;
    QPointF     sLastDrag;   // screen point
    QPolygonF   poly;
};


class BackgroundImageCropper
{
public:
    BackgroundImageCropper();
    ~BackgroundImageCropper();

    bool startDragging(QPointF spt);
    bool updateDragging(QPointF spt );
    bool endDragging(QPointF spt );

    void draw(QPainter *painter );

    void       activate(bool enb, BackgroundImage * parent = nullptr, Crop * crop = nullptr);
    bool       isActive()     { return active; }

protected:
    void    setMousePos(QPointF pt);

    Crop *  crop;

private:
    bool        active;
    QPointF     mousePos;

    BackgroundImage * parent;

    MouseEditCrop   * editor;
};

#endif
