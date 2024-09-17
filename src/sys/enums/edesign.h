#pragma once
#ifndef EDESIGN_H
#define EDESIGN_H

#include <QString>
#include <QMap>

extern QMap<int, QString> designs;
extern void init_legacy_designs();

enum eDesign
{
    DESIGN_5,
    DESIGN_6,
    DESIGN_7,
    DESIGN_8,
    DESIGN_9,
    DESIGN_HU_INSERT,
    DESIGN_10,
    DESIGN_11,
    DESIGN_12,
    DESIGN_13,
    DESIGN_14,
    DESIGN_16,
    DESIGN_17,
    DESIGN_18,
    DESIGN_19,
    DESIGN_KUMIKO1,
    DESIGN_KUMIKO2,
    NO_DESIGN
};

#endif // EDESIGN_H
