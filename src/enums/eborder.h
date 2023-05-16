#pragma once
#ifndef EBORDER_H
#define EBORDER_H

#include <QString>

extern QString sBorderType[];
extern QString sCropType[];
extern QString sCropState[];

static const QChar MathSymbolSquareRoot(0x221A);
static const QChar MathSymbolPi(0x03A0);
static const QChar MathSymbolDelta(0x0394);
static const QChar MathSymbolSigma(0x03A3);

enum  eAspectRatio
{
    ASPECT_UNCONSTRAINED,
    ASPECT_SQRT_2,
    ASPECT_SQRT_3,
    ASPECT_SQRT_4,
    ASPECT_SQRT_5,
    ASPECT_SQRT_6,
    ASPECT_SQRT_7,
    ASPECT_SQUARE,
    ASPECT_SD,
    ASPECT_HD
};

enum eCropType
{
    CROP_UNDEFINED,
    CROP_RECTANGLE,
    CROP_POLYGON,
    CROP_CIRCLE         // yep crop circles are real
};

// these enum numbers are use in xml writer/reader - do not change
enum eBorderType
{
    BORDER_NONE         = 0,
    BORDER_PLAIN        = 1,
    BORDER_TWO_COLOR    = 2,
    BORDER_BLOCKS       = 3,
};

#endif
