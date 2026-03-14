#pragma once
#ifndef THICK_EDITOR_H
#define THICK_EDITOR_H

#include "gui/model_editors/style_edit/colored_editor.h"

class SliderSet;
class QCheckBox;
class QTableWidgetItem;
class QComboBox;
class Thick;

// Editor for the thick style.
class ThickEditor : public ColoredEditor
{
    Q_OBJECT

public:
    ThickEditor(StylePtr style, eStyleType user);

 private slots:
    void  slot_outlineChanged(bool checked);
    void  slot_widthChanged(int width);
    void  slot_outlineWidthChanged(int width);
    void  slot_outlineColor();
    void  slot_joinStyle(int index);
    void  slot_capStyle(int index);

private:
    weak_ptr<Thick>    wthick;
    SliderSet        * width_slider;
    QCheckBox        * outline_checkbox;
    SliderSet        * outline_width_slider;
    QTableWidgetItem * outline_color;
    QPushButton      * outline_color_button;
    QComboBox        * join_style;
    QComboBox        * cap_style;
};

#endif
