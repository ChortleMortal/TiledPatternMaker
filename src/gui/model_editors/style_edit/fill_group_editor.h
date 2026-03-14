#pragma once
#ifndef STYLE_COLOR_FILL_GROUP_H
#define STYLE_COLOR_FILL_GROUP_H

#include "gui/model_editors/style_edit/fill_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "model/styles/colorset.h"
#include "model/styles/filled.h"

class AQTableWidget;

class FillGroupEditor : public FilledSubTypeEditor
{
    Q_OBJECT

    enum eFGCol
    {
        COL_INDEX    = 0,
        COL_COUNT    = 1,
        COL_SIDES    = 2,
        COL_AREA     = 3,
        COL_HIDE     = 4,
        COL_SEL      = 5,
        COL_EDIT_BTN = 6,
        COL_COLORS   = 7
    };

public:
    FillGroupEditor(FilledEditor * parent, FilledPtr style, New3Coloring * cm, QVBoxLayout * vbox);

    virtual void mousePressed(QPointF mpt, Qt::MouseButton btn) override;
    virtual void colorPick(QColor color)                        override;
    virtual void refresh()                                      override;
    virtual void notify()                                       override;

signals:

private slots:
    void up();
    void down();
    void rptSet();
    void copySet();
    void pasteSet();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

protected:
    void create();
    void populateRow(const FaceSetPtr & faceSet, int row);
    void refreshRow( const FaceSetPtr & faceSet, int row);

    void edit(int row);
    void colorSetVisibilityChanged(int row);


private:
    AQTableWidget * table;

    ColorSet        copyPasteSet;

    New3Coloring  * new3cm;
};
#endif
