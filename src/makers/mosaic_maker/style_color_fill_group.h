#pragma once
#ifndef STYLE_COLOR_FILL_GROUP_H
#define STYLE_COLOR_FILL_GROUP_H

#include <QObject>
#include "misc/colorset.h"

class AQTableWidget;

typedef std::shared_ptr<class Filled>       FilledPtr;

class StyleColorFillGroup : public QObject
{
    Q_OBJECT

    enum eFGCol
    {
        COL_INDEX = 0,
        COL_COUNT = 1,
        COL_SIDES = 2,
        COL_AREA  = 3,
        COL_HIDE  = 4,
        COL_SEL   = 5,
        COL_BTN   = 6,
        COL_COLORS= 7
    };

public:
    StyleColorFillGroup(FilledPtr style, QVBoxLayout * vbox);
    void display();
    void select(QPointF mpt);
    void setColor(QColor color);

signals:
    void sig_colorsChanged();

private slots:

    void modify();
    void up();
    void down();
    void rptSet();
    void copySet();
    void pasteSet();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

protected:
    void edit(int row);
    void colorSetVisibilityChanged(int row);


private:
    FilledPtr       filled;

    QTableWidget  * table;
    ColorSet        copyPasteSet;
};
#endif
