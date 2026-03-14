#pragma once
#ifndef STYLE_COLOR_FILL_ORIGINAL_H
#define STYLE_COLOR_FILL_ORIGINAL_H

#include "gui/model_editors/style_edit/fill_editor.h"
#include "model/styles/filled.h"

class QTableWidget;
class QTableWidgetItem;
class ColorSetWidget;
class QCheckBox;

class FillOriginalEditor : public FilledSubTypeEditor
{
    Q_OBJECT

    enum eCol
    {
        COL_SHOW    = 0,
        COL_COLORS  = 1,
        COL_EDIT    = 2,
        COL_NUMBER  = 3
    };

public:
    FillOriginalEditor(FilledEditor * parent, FilledPtr filled, OriginalColoring *cm, QVBoxLayout * vbox);

    virtual void mousePressed(QPointF mpt, Qt::MouseButton btn) override;
    virtual void colorPick(QColor color)                        override;
    virtual void refresh()                                      override;
    virtual void notify()                                       override;

private slots:
    void slot_insideChanged(bool checked);
    void slot_outsideChanged(bool checked);
    void slot_editW();
    void slot_editB();

private:
    OriginalColoring    * ocm;

    QTableWidget        * table;

    QCheckBox           * inside_checkbox;
    QCheckBox           * outside_checkbox;

    ColorSetWidget      * cswW;
    ColorSetWidget      * cswB;

    QTableWidgetItem    * infoW;
    QTableWidgetItem    * infoB;
};

#endif