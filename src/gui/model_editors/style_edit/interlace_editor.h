#pragma once
#ifndef INTERLACE_EDITOR_H
#define INTERLACE_EDITOR_H

#include "gui/model_editors/style_edit/thick_editor.h"

class Interlace;

// Editor for teh interlaced style.
class InterlaceEditor : public ThickEditor
{
    Q_OBJECT

public:
    InterlaceEditor(StylePtr style, eStyleType user);

private slots :
    void slot_gapChanged(qreal gap);
    void slot_shadowChanged(qreal shadow);
    void slot_startUnderChanged(bool checked);
    void slot_includeTipVerticesChanged(bool checked);

private:
    weak_ptr<Interlace> winterlace;

    DoubleSliderSet * gap_slider;
    DoubleSliderSet * shadow_slider;
    QCheckBox       * tipVert_checkbox;
    QCheckBox       * sunder_checkbox;      // start under
};

#endif
