#pragma once
#ifndef EBKGDIMG_H
#define EBKGDIMG_H

#include <QString>

extern QString sBkgdImageSource[];

enum eBkgdImgSource
{
    BKGD_IMAGE_NONE,
    BKGD_IMAGE_MOSAIC,
    BKGD_IMAGE_TILING,
    BKGD_IMAGE_MAPED,
    BKGD_IMAGE_DEFINED
};

#endif
