#include "geometry/crop.h"

const QString sCropState[] = {
    E2STR(CROP_NONE),
    E2STR(CROP_CONSTRUCTING),
    E2STR(CROP_PREPARED),
    E2STR(CROP_BORDER_PREPARED),
    E2STR(CROP_EDITING),
    E2STR(CROP_DEFINED),
    E2STR(CROP_BORDER_DEFINED),
    E2STR(CROP_APPLIED),
    E2STR(CROP_MASKED),
    E2STR(CROP_COMPLETE)
};

Crop::Crop()
{
    state  = CROP_NONE;
    aspect = ASPECT_UNCONSTRAINED;
    aspectVertical = false;
}

void Crop::reset()
{
    rect    = QRectF() ;
    state   = CROP_NONE;
    aspect  = ASPECT_UNCONSTRAINED;
    aspectVertical = false;
}

void Crop::setRect(QRectF & rect, eCropState cstate)
{
    this->rect = rect.normalized();
    state = cstate;
    adjust();
}

void Crop::adjust()
{
    if (state == CROP_NONE)
        return;

    qreal mult;

    switch (aspect)
    {
    case ASPECT_UNCONSTRAINED:
    default:
        return;

    case ASPECT_SQRT_2:
        mult = M_SQRT2;
        break;

    case ASPECT_SQRT_3:
        mult = qSqrt(3.0);
        break;

    case ASPECT_SQRT_4:
        mult = 2.0;
        break;

    case ASPECT_SQRT_5:
        mult = qSqrt(5.0);
        break;

    case ASPECT_SQRT_6:
        mult = qSqrt(6.0);
        break;

    case ASPECT_SQRT_7:
        mult = qSqrt(7.0);
        break;

    case ASPECT_SQUARE:
        mult = 1.0;
        break;

    case ASPECT_SD:
        mult = 4.0/3.0;
        break;

    case ASPECT_HD:
        mult = 16.0/9.0;
        break;
    }

    if (aspectVertical)
    {
        rect.setHeight(rect.width() * mult);
    }
    else
    {
        rect.setWidth(rect.height() * mult);
    }
}

QString Crop::getStateStr()
{
    return sCropState[state];
}
