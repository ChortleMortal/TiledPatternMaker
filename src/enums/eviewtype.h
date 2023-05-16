#pragma once
#ifndef EVIEWTYPE_H
#define EVIEWTYPE_H

#include <QString>

extern const QString sViewerType[];

enum eViewType
{
    VIEW_DESIGN,
    VIEW_START = VIEW_DESIGN,
    VIEW_MOSAIC,
    VIEW_PROTOTYPE,
    VIEW_MOTIF_MAKER,
    VIEW_TILING,
    VIEW_TILING_MAKER,
    VIEW_BORDER,
    VIEW_CROP,
    VIEW_DESIGNER_MAX = VIEW_CROP,
    VIEW_MAP_EDITOR,
    VIEW_MAX = VIEW_MAP_EDITOR,
    VIEW_BKGD_IMG,
    VIEW_GRID,
};

#endif // EVIEWTYPE_H
