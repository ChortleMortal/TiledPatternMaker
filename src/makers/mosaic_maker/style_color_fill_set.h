#pragma once
#ifndef STYLE_COLOR_FILL_SET_H
#define STYLE_COLOR_FILL_SET_H

#include <QVBoxLayout>
#include "misc/colorset.h"

class AQTableWidget;

typedef std::shared_ptr<class Filled>       FilledPtr;

class StyleColorFillSet : public QObject
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
        COL_COLOR_PATCH = 7
    };

public:
    StyleColorFillSet(FilledPtr style, QVBoxLayout * vbox);
    void display();
    void select(QPointF mpt);
    void setColor(QColor color);

signals:
    void sig_colorsChanged();

private slots:
    void modify();
    void up();
    void down();
    void rptColor();
    void copyColor();
    void pasteColor();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

protected:
    void colorChanged(int row);
    void colorVisibilityChanged(int row);

private:
    FilledPtr       filled;
    QTableWidget  * table;
    TPColor         copyPasteColor;
};
#endif
