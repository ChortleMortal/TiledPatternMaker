#ifndef MOUSEEDITCROP_H
#define MOUSEEDITCROP_H

#include <QPointF>
#include <QPainter>

typedef std::shared_ptr<class Crop>       CropPtr;

class MouseEditCrop
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
    MouseEditCrop(QPointF spt, CropPtr crop, QTransform t);

    virtual void updateDragging(QPointF spt, QTransform t);
    virtual void endDragging(QPointF spt, QTransform t);
    virtual void draw(QPainter * painter, QPointF mousePos, QTransform t);

protected:
    QPointF * start;
    QPointF * end;
    CropPtr  crop;

    eEditCropMode   ecMode;
    eCropCorner     ecCorner;
    eCircleMode     ecCircleMode;

private:
};


#endif // MOUSEEDITCROP_H
