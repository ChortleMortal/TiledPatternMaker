#include "sys/enums/emapeditor.h"

#define E2STR(x) #x

const QString sMapEditorMapType[] =
{
        "None",              //E2STR(MAPED_TYPE_NONE),
        "Style:",            //E2STR(MAPED_LOADED_FROM_MOSIAC),
        "Proto:",            //E2STR(MAPED_LOADED_FROM_PROTOTMAKER),
        "Motif:",            //E2STR(MAPED_LOADED_FROM_MOTIF),
        "Tiling unit:",      //E2STR(MAPED_LOADED_FROM_TILING_UNIT),
        "Tiling:",           //E2STR(MAPED_LOADED_FROM_TILING_REPEATED),
        "File:",             //E2STR(MAPED_LOADED_FROM_FILE),
        "File motif:",       //E2STR(MAPED_LOADED_FROM_FILE_MOTIF),
        "Created:",          //E2STR(MAPED_TYPE_CREATED),
        "Composite:",        //E2STR(MAPED_TYPE_COMPOSITE),
        "Crop:",             //E2STR(MAPED_TYPE_CROP),
        "Composite Motif:"   //E2STR(MAPED_TYPE_COMPOSITE_MOTIF)
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
