#include "eviewtype.h"

#define E2STR(x) #x

const QString sViewerType[] = {
    E2STR(VIEW_LEGACY),
    E2STR(VIEW_MOSAIC),
    E2STR(VIEW_PROTOTYPE),
    E2STR(VIEW_MOTIF_MAKER),
    E2STR(VIEW_TILING),
    E2STR(VIEW_TILING_MAKER),
    E2STR(VIEW_MAP_EDITOR),
    E2STR(VIEW_BKGD_IMG),
    E2STR(VIEW_GRID),
    E2STR(VIEW_CROP),
    E2STR(VIEW_DEBUG),
    E2STR(VIEW_BMP_IMAGE),
};

const QString s2ViewerType[] = {
    "Legacy Design View",
    "Mosaic View",
    "Prototype View",
    "Motif Maker View",
    "Tiling View",
    "Tiling Maker View",
    "Map Editor View",
    "Background Image View",
    "Grid View",
    "Crop View",
    "Debug View",
    "BMP Image View"
};
