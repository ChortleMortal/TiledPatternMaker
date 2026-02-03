#pragma once
#ifndef EMAPEDITOR_H
#define EMAPEDITOR_H

#include <QString>

extern const QString sMapEditorMapType[];
extern const QString sMapEditorMode[];
extern const QString sMapEditorMouseMode[];
extern const QString sViewMode[];
extern const QString sEditMode[];

enum eMapEditorMapType
{
    MAPED_TYPE_NONE,
    MAPED_LOADED_FROM_STYLE,
    MAPED_LOADED_FROM_PROTOTMAKER,
    MAPED_LOADED_FROM_MOTIF,
    MAPED_LOADED_FROM_TILING_UNIT,
    MAPED_LOADED_FROM_TILING_REPEATED,
    MAPED_LOADED_FROM_FILE,
    MAPED_LOADED_FROM_FILE_MOTIF,
    MAPED_TYPE_CREATED,
    MAPED_TYPE_COMPOSITE,
    MAPED_TYPE_CROP,
    MAPED_TYPE_COMPOSITE_MOTIF
};

enum eMapEditorMode
{
    MAPED_MODE_MAP,
    MAPED_MODE_DCEL,
    MAPED_MODE_MAX = MAPED_MODE_DCEL
};

enum eMapEditorMouseMode
{
    MAPED_MOUSE_NONE,
    MAPED_MOUSE_DRAW_LINE,
    MAPED_MOUSE_DELETE,
    MAPED_MOUSE_SPLIT_LINE,
    MAPED_MOUSE_CONSTRUCTION_LINES,
    MAPED_MOUSE_EXTEND_LINE_P1,
    MAPED_MOUSE_EXTEND_LINE_P2,
    MAPED_MOUSE_CONSTRUCTION_CIRCLES,
    MAPED_MOUSE_CREATE_LINE
 };

enum eMapedLayer
{
    NO_MAP      = 0x00,
    LAYER_1     = 0x01,
    LAYER_2     = 0x02,
    LAYER_3     = 0x04,
    COMPOSITE   = 0x08
};

#endif // EMAPEDITOR_H
