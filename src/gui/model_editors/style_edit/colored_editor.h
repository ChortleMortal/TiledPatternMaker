#pragma once
#ifndef COLORED_EDITOR_H
#define COLORED_EDITOR_H

#include "gui/model_editors/style_edit/style_editor.h"

class Colored;
class QPushButton;
class DoubleSliderSet;

// Editor for the colored style: allow choosing the color.
class ColoredEditor : public StyleEditor
{
    Q_OBJECT

public:
    ColoredEditor(StylePtr style, eStyleType user);

public slots:
    virtual void slot_colorsChanged();
    virtual void slot_opacityChanged(qreal val);

private slots:
    void slot_selectColor();

protected:
    int             rows;

private:
    void updateOpacity();

    weak_ptr<Colored>  wcolored;
    QWidget         * colorwidget;
    QPushButton     * color_button;
    DoubleSliderSet * opacitySlider;
};

#endif
