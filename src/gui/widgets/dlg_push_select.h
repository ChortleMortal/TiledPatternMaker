#pragma once

class QRadioButton;

#include <QDialog>

class DlgPushSelect : public QDialog
{
    Q_OBJECT

public:
    DlgPushSelect(QWidget * parent = nullptr);

    int retval;

private slots:
    void select();

private:
    QRadioButton * rbMosaic;
    QRadioButton * rbMotif;
    QRadioButton * rbTiling;
};
