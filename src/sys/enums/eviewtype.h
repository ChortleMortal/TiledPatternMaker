#pragma once
#ifndef EVIEWTYPE_H
#define EVIEWTYPE_H

#include <QString>

extern const QString sViewerType[];
extern const QString s2ViewerType[];
extern const QString sViewAlign[];
extern const QString sLayerType[];

enum eViewType
{
    VIEW_LEGACY,

    VIEW_MOSAIC,
    VIEW_PROTOTYPE,
    VIEW_MOTIF_MAKER,
    VIEW_TILING,
    VIEW_TILING_MAKER,
    VIEW_MAP_EDITOR,
    VIEW_BKGD_IMG,
    VIEW_GRID,
    VIEW_CROP,
    VIEW_DEBUG,
    VIEW_BMP_IMAGE
};

enum eModelType
{
    PRIMARY,        // VIEW_MOSAIC,VIEW_TILING,VIEW_BKGD_IMAGE & VIEW_BMP_IMAGE
    DERIVED         // all other followers
};

#define NUM_VIEW_TYPES  (VIEW_BMP_IMAGE +1)

enum eZLevel
{
    BKGD_IMG_ZLEVEL     = -20,
    STANDARD_ZLEVEL     = 0,
    IMAGE_ZLEVEL        = 1,
    LEGACY_Z1           = 1,
    LEGACY_Z2           = 2,
    LEGACY_Z3           = 3,
    LEGACY_Z4           = 4,
    LEGACY_Z5           = 5,
    GRID_ZLEVEL         = 5,
    MEASURE_ZLEVEL      = 9,
    BORDER_ZLEVEL       = 10,
    CROP_VIEW_ZLEVEL    = 11,
    BKGD_IMG__LIFTED_ZLEVEL = 25,
};

#endif // EVIEWTYPE_H
