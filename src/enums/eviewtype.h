#pragma once
#ifndef EVIEWTYPE_H
#define EVIEWTYPE_H

#include <QString>

extern const QString sViewerType[];
extern const QString s2ViewerType[];

enum eViewType
{
    VIEW_DESIGN,
    VIEW_MOSAIC,
    VIEW_PROTOTYPE,
    VIEW_MOTIF_MAKER,
    VIEW_TILING,
    VIEW_TILING_MAKER,
    VIEW_MAP_EDITOR,
    VIEW_BKGD_IMG,
    VIEW_GRID,
    VIEW_BORDER,
    VIEW_CROP,
    VIEW_MEASURE,
    VIEW_CENTER,
    VIEW_IMAGE
};

#define NUM_VIEW_TYPES  (VIEW_IMAGE +1)

#define BKGD_IMG_ZLEVEL -20
#define STANDARD_ZLEVEL   0
#define MEASURE_ZLEVEL    9
#define BORDER_ZLEVEL    10


#endif // EVIEWTYPE_H
