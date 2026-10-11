#pragma once

enum eFileType
{
    FILE_MOSAIC,
    FILE_TILING,
    FILE_MAP,
    FILE_GIRIH,
    FILE_TEMPLATE        // map editor construction line templates
};

enum eLoadType
{
    ALL_MOSAICS,
    SELECTED_MOSAICS,
    SINGLE_MOSAIC,
    WORKLIST,
    ALL_TILINGS,
    SELECTED_TILINGS,
    ALL_MOS_EXCEPT_WL,
    ALL_TIL_EXCEPT_WL
};
