#pragma once
#ifndef STYLE_COLOR_FILL_NEW1_H
#define STYLE_COLOR_FILL_NEW1_H

#include <QCheckBox>

#include "gui/model_editors/style_edit/fill_editor.h"
#include "model/styles/filled.h"

class QTableWidget;
class ColorSetWidget;
class QTableWidgetItem;

class FillNew1Editor : public FilledSubTypeEditor
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
    FillNew1Editor(FilledEditor * parent, FilledPtr filled, New1Coloring *cm, QVBoxLayout * vbox);

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
    New1Coloring        * n1cm;

    QTableWidget        * table;

    QCheckBox           * inside_checkbox;
    QCheckBox           * outside_checkbox;

    ColorSetWidget      * cswW;
    ColorSetWidget      * cswB;

    QTableWidgetItem    * infoW;
    QTableWidgetItem    * infoB;
};

#endif
