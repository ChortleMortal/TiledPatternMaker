#pragma once
#ifndef EMOTIFTYPE_H
#define EMOTIFTYPE_H

#include <QString>

extern const QString sMotifType[];

// these are the types of figure which can be made
enum eMotifType
{
    MOTIF_TYPE_UNDEFINED,
    MOTIF_TYPE_RADIAL,
    MOTIF_TYPE_ROSETTE,
    MOTIF_TYPE_ROSETTE2,

    MOTIF_TYPE_STAR,
    MOTIF_TYPE_STAR2,

    MOTIF_TYPE_EXPLICIT_MAP,
    MOTIF_TYPE_INFERRED,
    MOTIF_TYPE_IRREGULAR_ROSETTE,
    MOTIF_TYPE_HOURGLASS,
    MOTIF_TYPE_INTERSECT,
    MOTIF_TYPE_GIRIH,
    MOTIF_TYPE_IRREGULAR_STAR,
    MOTIF_TYPE_EXPLCIT_TILE,
    MOTIF_TYPE_IRREGULAR_NO_MAP,
    MAX_MOTIF_TYPE = MOTIF_TYPE_IRREGULAR_NO_MAP
};

#endif // EMOTIFTYPE_H
