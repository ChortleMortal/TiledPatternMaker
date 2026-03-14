#pragma once
#ifndef STYLE_COLOR_FILL_FACE_H
#define STYLE_COLOR_FILL_FACE_H

#include <QVBoxLayout>
#include <QPushButton>

#include "gui/model_editors/style_edit/fill_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "model/styles/filled.h"

class QTableWidget;

#define DEPRECATE_FILL_TYPE_4

class FillFaceEditor : public FilledSubTypeEditor
{
    Q_OBJECT

    enum eCol
    {
        COL_STATUS      = 0,
        COL_COLOR       = 1
    };

public:
    FillFaceEditor(FilledEditor * parent, FilledPtr style, DirectColoring * cm,  QVBoxLayout * vbox);
    ~FillFaceEditor();

    virtual void mousePressed(QPointF mpt, Qt::MouseButton btn) override;
    virtual void colorPick(QColor color)                        override;
    virtual void refresh()                                      override;
    virtual void notify()                                       override;

signals:
    void sig_updateView();

private slots:
    void slot_cellClicked(int row,int column);
    void slot_resetColorMap();
    void slot_add();
    void slot_modify();
    void slot_delete();

protected:
    void create();
    void modify(int row);

private:
    AQTableWidget   * table;

    int               iPaletteSelection;

    DirectColoring  * dcm;
};

#endif
