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

#define BKGD_IMG_ZLEVEL -20
#define STANDARD_ZLEVEL   0
#define MEASURE_ZLEVEL    9
#define BORDER_ZLEVEL    10


#endif // EVIEWTYPE_H
