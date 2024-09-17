#pragma once
#ifndef MOUSEEDITBORDER_H
#define MOUSEEDITBORDER_H

#include <QPointF>
#include <QPainter>

class Border;
class Crop;

class MouseEditBorder
{
    enum eEditCropMode
    {
        EC_READY,
        EC_MOVE,
        CB_RESIZE,
        };

    enum eCropCorner
    {
        UNDEFINED,
        TL,
        TR,
        BL,
        BR
    };

    enum eCircleMode
    {
        CM_OUTSIDE,
        CM_EDGE,
        CM_INSIDE
    };

public:
    MouseEditBorder(Border * view, QPointF spt, Crop * crop);
    virtual ~MouseEditBorder() {}

    virtual void updateDragging(QPointF spt);
    virtual void endDragging(QPointF spt);
    virtual void draw(QPainter * painter, QPointF mousePos);

protected:
    QPointF * start;
    QPointF * end;
    Crop    * mcrop; // model units

    eEditCropMode   ecMode;
    eCropCorner     ecCorner;
    eCircleMode     ecCircleMode;

private:
    Border * bview;
};


#endif // MOUSEEDITBORDER_H
