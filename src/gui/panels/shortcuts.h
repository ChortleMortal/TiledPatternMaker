#pragma once
#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QString>

class QWidget;

#include "sys/enums/eviewtype.h"

class Shortcuts
{
public:
    static void  popup(eViewType view);

    static QString getMosaicShortcuts();
    static QString getDesignShortcuts();
    static QString getTilingMakerShortcuts();
    static QString getMapEditorShortcuts();

};

#endif // SHORTCUTS_H
