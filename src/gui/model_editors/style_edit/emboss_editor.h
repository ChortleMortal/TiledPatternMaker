#pragma once
#ifndef EMBOSS_EDITOR_H
#define EMBOSS_EDITOR_H

#include "gui/model_editors/style_edit/thick_editor.h"

class Emboss;

class EmbossEditor : public ThickEditor
{
    Q_OBJECT

public:
    EmbossEditor(StylePtr style,eStyleType user);

public slots:
     virtual void slot_colorsChanged() override;
     virtual void slot_opacityChanged(qreal val) override;

private slots:
    void slot_anlgeChanged(int angle);

private:
    weak_ptr<Emboss>    wemboss;
    SliderSet         * angle_slider;
};

#endif
