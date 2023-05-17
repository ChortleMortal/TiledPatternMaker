#include "enums/emapeditor.h"

#define E2STR(x) #x

const QString sMapEditorMapType[] =
{
        "Unknown",                  //E2STR(MAPED_TYPE_UNKNOWN),
        "Loaded from Mosaic",       //E2STR(MAPED_LOADED_FROM_MOSIAC),
        "Loaded from Motif Proto",  //E2STR(MAPED_LOADED_FROM_MOTIF_PROTOTYPE),
        "Loaded from Motif",        //E2STR(MAPED_LOADED_FROM_MOTIF),
        "Loaded from Tiling (unit)",//E2STR(MAPED_LOADED_FROM_TILING_UNIT),
        "Loaded from Tiling (full)",//E2STR(MAPED_LOADED_FROM_TILING_REPEATED),
        "Loaded from File",         //E2STR(MAPED_LOADED_FROM_FILE),
        "Loaded from File (Motif)", //E2STR(MAPED_LOADED_FROM_FILE_MOTIF),
        "Created here",             //E2STR(MAPED_TYPE_CREATED),
        "Composite",                //E2STR(MAPED_TYPE_COMPOSITE)
        "Crop",                     //E2STR(MAPED_TYPE_CROP)
        "Composite Motif"
};

const QString sMapEditorMode[] =
{
        E2STR(MAPED_MODE_MAP),
        E2STR(MAPED_MODE_DCEL)
};

const QString sMapEditorMouseMode[] =
{
        E2STR(MAPED_MOUSE_NONE),
        E2STR(MAPED_MOUSE_DRAW_LINE),
        E2STR(MAPED_MOUSE_DELETE),
        E2STR(MAPED_MOUSE_SPLIT_LINE),
        E2STR(MAPED_MOUSE_CONSTRUCTION_LINES),
        E2STR(MAPED_MOUSE_EXTEND_LINE_P1),
        E2STR(MAPED_MOUSE_EXTEND_LINE_P2),
        E2STR(MAPED_MOUSE_CONSTRUCTION_CIRCLES),
        E2STR(MAPED_MOUSE_CREATE_LINE)
};

const QString sViewMode[] =
{
    "Composite Map",    //E2STR(VIEW_COMPOSITE),
    "Separate Maps",    //E2STR(VIEW_ALL_LAYERS),
    "Layer 1 Map",      //E2STR(VIEW_LAYER_1),
    "Layer 2 Map",      //E2STR(VIEW_LAYER_2),
    "Layer 3 Map",      //E2STR(VIEW_LAYER_3)
};

const QString  sEditMode[] =
{
    "Composite Map",    //E2STR(EDIT_COMPOSITE),
    "Layer 1 Map",      //E2STR(EDIT_LAYER_1),
    "Layer 2 Map",      //E2STR(EDIT_LAYER_2),
    "Layer 3 Map"       //E2STR(EDIT_LAYER_3)
};