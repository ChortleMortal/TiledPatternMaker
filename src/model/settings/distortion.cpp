#include "model/settings/distortion.h"

Distortion::Distortion()
{
    init();
}

void Distortion::init()
{
    _set    = false;
    mode    = DISTORT_NONE;
    scaleX  = 1.0;
    scaleY  = 1.0;
}

Distortion::Distortion(const Distortion & other)
{
    *this = other;
}

Distortion & Distortion::operator=(const Distortion & other)
{
    mode   = other.mode;
    scaleX = other.scaleX;
    scaleY = other.scaleY;
    _set   = other._set;

    return *this;
}

bool Distortion::operator==(const Distortion & other) const
{
    if (mode   != other.mode)   return false;
    if (scaleX != other.scaleX) return false;
    if (scaleY != other.scaleY) return false;

    return true;
}

void Distortion::set(eDistortionMode mode, qreal scaleX, qreal scaleY)
{
    _set         = true;
    this->mode   = mode;
    this->scaleX = scaleX;
    this->scaleY = scaleY;
}

void Distortion::get(eDistortionMode & mode, qreal & scaleX, qreal & scaleY) const
{
    mode   = this->mode;
    scaleX = this->scaleX;
    scaleY = this->scaleY;
}

QTransform Distortion::getTransform() const
{
    if (mode == DISTORT_NONE)
    {
        return QTransform();
    }
    return QTransform::fromScale(scaleX,scaleY);
}
