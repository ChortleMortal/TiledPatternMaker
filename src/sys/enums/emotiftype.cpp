#include "emotiftype.h"

#define E2STR(x) #x

const QString sMotifType[] =
{
        "Undefined Motif",              //E2STR(MOTIF_TYPE_UNDEFINED),
        "Radial Motif",                 //E2STR(MOTIF_TYPE_RADIAL),
        "Rosette Motif",                //E2STR(MOTIF_TYPE_ROSETTE),
        "Rosette2 Motif",               //E2STR(MOTIF_TYPE_ROSETTE2),

        "Star Motif",                   //E2STR(MOTIF_TYPE_STAR),
        "Star2 Motif",                  //E2STR(MOTIF_TYPE_STAR2),

        "Explicit Map Motif",           //E2STR(MOTIF_TYPE_EXPLICIT_MAP),
        "Irregular Inferred Motif",     //E2STR(MOTIF_TYPE_INFERRED),
        "Irregular Rosette Motif",      //E2STR(MOTIF_TYPE_IRREGULAR_ROSETTE),
        "Irregular Hourglass Motif",    //E2STR(MOTIF_TYPE_HOURGLASS),

        "Irregular Intersect Motif",    //E2STR(MOTIF_TYPE_INTERSECT),
        "Irregular Girih Motif",        //E2STR(MOTIF_TYPE_GIRIH),
        "Irregular Star Motif",         //E2STR(MOTIF_TYPE_IRREGULAR_STAR),

        "Explicit Tile Motif",          //E2STR(MOTIF_TYPE_EXPLCIT_TILE),
        "No Motif (for Tile)"           //1E2STR(MOTIF_TYPE_IRREGULAR_NO_MAP)
};
