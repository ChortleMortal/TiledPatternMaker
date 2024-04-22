#pragma once
#ifndef EFILESYSTEM_H
#define EFILESYSTEM_H

enum eFileType
{
    FILE_MOSAIC,
    FILE_TILING,
    FILE_MAP
};

enum eLoadType
{
    ALL_MOSAICS,
    SELECTED_MOSAICS,
    SINGLE_MOSAIC,
    WORKLIST,
    ALL_TILINGS,
    SELECTED_TILINGS
};

#endif
