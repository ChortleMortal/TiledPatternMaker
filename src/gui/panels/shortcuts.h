#pragma once

#include <QString>

class QWidget;

#include "gui/viewers/layer.h"

class Shortcuts
{
public:
    static void  popup(Layer::Type type);

    static QString getGeneralShortcuts();
    static QString getLegacyShortcuts();
    static QString getTilingMakerShortcuts();
    static QString getMapEditorShortcuts();
};
