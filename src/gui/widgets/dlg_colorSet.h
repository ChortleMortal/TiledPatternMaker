#pragma once

#include <QDialog>

class QTableWidget;

class ColorSet;

class DlgColorSet : public QDialog
{
    Q_OBJECT

public:
    DlgColorSet(ColorSet * cset, QWidget * parent);

protected:
    void displayTable();

    QTableWidget  * dlg_table;

signals:
    void sig_dlg_colorsChanged();

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
