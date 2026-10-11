#pragma once

#include <QDialog>
#include <QAbstractButton>
#include <QGridLayout>

#include "model/motifs/rosette2.h"

class Rosette2Editor;

typedef std::shared_ptr<class Rosette2>  Rosette2Ptr;

class DlgRosetteTips : public QDialog
{
    Q_OBJECT

public:
    DlgRosetteTips(Rosette2Ptr rose, Rosette2Editor * parent);

    QList<TipType> tips;

protected:
    void displayTable();

    void setTips();

private slots:
    void add();
    void del();
    void slot_ok();
    void up();
    void down();
    void selClicked(QAbstractButton *btn);
    void selectRow(QAbstractButton * btn);

private:
    Rosette2Ptr      rose;
    Rosette2Editor * parent;
    int              currentRow;

    QWidget        * tipWidget;
    QGridLayout    * tgrid;

    QVector<QPushButton*> btns;
    QVector<QButtonGroup*> radios;
    QVector<QButtonGroup*> indepbtns;
};
