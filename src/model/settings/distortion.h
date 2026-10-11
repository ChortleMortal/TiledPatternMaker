#pragma once

#include <QTransform>
#include "sys/enums/edistortion.h"

class Distortion
{
public:
    Distortion();
    Distortion(const Distortion & other);

    Distortion & operator=(const Distortion & other);
    bool operator==(const Distortion & other) const;
    bool operator != (const Distortion & other) const { return !(*this == other); }

    void        init();
    bool        isSet() const { return _set; }
    void        set(eDistortionMode mode, qreal scaleX, qreal scaleY);
    void        get(eDistortionMode & mode, qreal & scaleX, qreal & scaleY) const;

    eDistortionMode getMode() const { return mode; }
    bool        isEnabled() const { return mode != DISTORT_NONE; }
    QTransform  getTransform() const;

protected:
    bool             _set;
    eDistortionMode  mode;
    qreal            scaleX;
    qreal            scaleY;
};
