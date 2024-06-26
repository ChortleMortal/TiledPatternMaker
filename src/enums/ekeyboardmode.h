#pragma once
#ifndef EKEYBOARDMODE_H
#define EKEYBOARDMODE_H

#include <QString>

extern const QString sKbdMode[];

enum eKbdMode
{
    KBD_MODE_XFORM_VIEW,
    KBD_MODE_XFORM_SELECTED,
    KBD_MODE_XFORM_BKGD,
    KBD_MODE_XFORM_TILING,
    KBD_MODE_XFORM_UNIQUE_TILE,
    KBD_MODE_XFORM_PLACED_TILE,
    KBD_MODE_XFORM_GRID,
    KBD_MODE_DES_POS,
    KBD_MODE_DES_LAYER_SELECT,
    KBD_MODE_DES_ZLEVEL,
    KBD_MODE_DES_STEP,
    KBD_MODE_DES_SEPARATION,
    KBD_MODE_DES_ORIGIN,
    KBD_MODE_DES_OFFSET,
    KBD_MODE_MOVE
};

#endif // EKEYBOARDMODE_H
