#pragma once
#ifndef MOUSEEDITBORDER_H
#define MOUSEEDITBORDER_H

#include <QPointF>
#include <QPainter>

class BorderView;

typedef std::shared_ptr<class Crop> CropPtr;

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
    MouseEditBorder(BorderView * view, QPointF spt, CropPtr crop);

    virtual void updateDragging(QPointF spt);
    virtual void endDragging(QPointF spt);
    virtual void draw(QPainter * painter, QPointF mousePos);

protected:
    QPointF * start;
    QPointF * end;
    CropPtr   mcrop; // model units

    eEditCropMode   ecMode;
    eCropCorner     ecCorner;
    eCircleMode     ecCircleMode;

private:
    BorderView * bview;
};


#endif // MOUSEEDITBORDER_H
