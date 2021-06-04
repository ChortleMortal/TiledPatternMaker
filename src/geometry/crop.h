#ifndef CROP_H
#define CROP_H

#include <QRectF>
#include "base/configuration.h"

enum eCropState
{
    CROP_NONE,
    CROP_CONSTRUCTING,
    CROP_PREPARED,
    CROP_BORDER_PREPARED,
    CROP_EDITING,
    CROP_DEFINED,
    CROP_BORDER_DEFINED,
    CROP_APPLIED,
    CROP_MASKED,
    CROP_COMPLETE
};

class Crop : protected QRectF
{
    friend class CreateCrop;

public:
    Crop();

    void         reset();

    void         setRect(QRectF & rect, eCropState cstate);
    QRectF &     getRect() { return rect; }

    void         setState(eCropState s) { state = s; }
    eCropState   getState() { return state; }
    QString      getStateStr();

    void         setAspect(eAspectRatio ar) { aspect = ar; adjust(); }
    eAspectRatio getAspect() { return aspect; }

    void         setAspectVertical(bool set) { aspectVertical = set; adjust(); }
    bool         getAspectVertical() { return aspectVertical; }

protected:
    void        adjust();

private:
    QRectF       rect;
    eCropState   state;
    eAspectRatio aspect;
    bool         aspectVertical;
};

typedef shared_ptr<Crop> CropPtr;
typedef weak_ptr<Crop>   WeakCropPtr;

#endif // CROP_H
