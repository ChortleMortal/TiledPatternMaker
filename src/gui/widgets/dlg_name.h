#pragma once

class QLineEdit;

#include <QDialog>

class DlgName : public QDialog
{
public:
    DlgName(QWidget * parent = nullptr);

    QLineEdit * newEdit;
};
