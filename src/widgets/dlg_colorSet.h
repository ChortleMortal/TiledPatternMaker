#pragma once
#ifndef DLG_COLOR_SET_H
#define DLG_COLOR_SET_H

#include <QDialog>

class QTableWidget;

class ColorSet;

class DlgColorSet : public QDialog
{
    Q_OBJECT

public:
    DlgColorSet(ColorSet * cset, QWidget * parent = nullptr);

protected:
    void displayTable();

    QTableWidget  * table;

signals:
    void sig_colorsChanged();

private slots:
    void add();
    void modify();
    void del();
    void slot_ok();
    void up();
    void down();

protected:
    void colorVisibilityChanged(int row);

private:
    ColorSet *      colorSet;
    int             currentRow;
};

#endif
