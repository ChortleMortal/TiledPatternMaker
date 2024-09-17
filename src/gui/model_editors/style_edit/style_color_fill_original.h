#pragma once
#ifndef STYLE_COLOR_FILL_ORIGINAL_H
#define STYLE_COLOR_FILL_ORIGINAL_H

#include <QVBoxLayout>
#include <QCheckBox>

class QTableWidget;
class AQTableWidget;
class FilledEditor;

typedef std::shared_ptr<class Filled>       FilledPtr;

class StyleColorFillOriginal : public QObject
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
    StyleColorFillOriginal(FilledEditor * parent, FilledPtr style, QVBoxLayout * vbox);
    void display();

private slots:
    void slot_insideChanged(int state);
    void slot_outsideChanged(int state);

private:
    FilledEditor *  parent;
    FilledPtr       filled;
    QTableWidget  * table;

    QCheckBox     * inside_checkbox;
    QCheckBox     * outside_checkbox;

};
#endif
