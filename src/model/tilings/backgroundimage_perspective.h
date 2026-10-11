#pragma once

#include <QTransform>
#include <QGraphicsPixmapItem>

#include "gui/model_editors/crop_edit/mouse_edit_crop.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/xform.h"

class BackgroundImage;

class BackgroundImagePerspective
{
public:
    BackgroundImagePerspective();

    bool startDragging(QPointF spt);
    bool updateDragging(QPointF spt );
    bool endDragging(QPointF spt );
    bool addPoint(QPointF spt);

    void draw(QPainter *painter );

    void       activate(bool enb, BackgroundImage * parent = nullptr);  // enb=false restores the model xform saved on activation
    bool       isActive()     { return active; }
    bool       hasPoints()    { return sAccum.size() > 0; }
    void       clear();        // discards the accumulated points (e.g. once the adjustment is saved)

    EdgeSet  & getAccum()    { return sAccum; }

protected:
    void drawPerspective(QPainter *painter );

    EdgeSet     sAccum;       // model-space points (see BackgroundImage::keystoneScreenToModel)

private:
    bool        active;
    QPointF     spt;
    QPointF     sLastDrag;   // model-space point
    VertexPtr   dragVertex;  // corner being moved, once all four points are placed
    QPolygonF   poly;

    BackgroundImage * parent;
    Xform       savedXform;  // model xform as it was when this session started
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
