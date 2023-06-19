#pragma once
#ifndef EVIEWTYPE_H
#define EVIEWTYPE_H

#include <QString>

extern const QString sViewerType[];

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
    VIEW_MAX = VIEW_GRID,
    VIEW_BORDER,
    VIEW_CROP,
};

#endif // EVIEWTYPE_H
