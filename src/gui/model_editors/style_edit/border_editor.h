#pragma once

#include "gui/model_editors/style_edit/style_editor.h"

// Editor for a border style: delegates to the Border Maker
class BorderEditor : public StyleEditor
{
    Q_OBJECT

public:
    BorderEditor(StylePtr style, eStyleType user);

signals:
    void sig_editBorder();
};
