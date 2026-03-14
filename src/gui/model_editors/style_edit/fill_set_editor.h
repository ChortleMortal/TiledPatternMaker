#pragma once
#ifndef STYLE_COLOR_FILL_SET_H
#define STYLE_COLOR_FILL_SET_H

#include "gui/model_editors/style_edit/fill_editor.h"
#include "gui/model_editors/style_edit/style_editor.h"
#include "model/styles/colorset.h"
#include "model/styles/filled.h"

class QTableWidget;

class FillSetEditor : public FilledSubTypeEditor
{
    Q_OBJECT

    enum eCol
    {
        COL_ROW         = 0,
        COL_FACES       = 1,
        COL_SIDES       = 2,
        COL_AREA        = 3,
        COL_HIDE        = 4,
        COL_SEL         = 5,
        COL_COLOR_TEXT  = 6,
        COL_COLOR_PATCH = 7,
        COL_EDIT_BTN    = 8
    };

public:
    FillSetEditor(FilledEditor * parent, FilledPtr filled, New2Coloring * cm, QVBoxLayout * vbox);

    virtual void mousePressed(QPointF mpt, Qt::MouseButton btn) override;
    virtual void colorPick(QColor color)                        override;
    virtual void refresh()                                      override;
    virtual void notify()                                       override;

    void create();

signals:

private slots:
    void edit(int row);
    void up();
    void down();
    void rptColor();
    void copyColor();
    void pasteColor();
    void slot_cellClick(int row, int col);
    void slot_double_click(int row, int col);
    void btnClicked();

protected:
    void populateRow(const FaceSetPtr & faceSet, int row);
    void refreshRow( const FaceSetPtr & faceSet, int row);

    void colorChanged(int row);
    void colorVisibilityChanged(int row);
    QWidget* findCellWidget(QWidget *w, QTableWidget *table);

private:
    AQTableWidget * table;

    TPColor         copyPasteColor;

    New2Coloring  * new2cm;
};

#endif
