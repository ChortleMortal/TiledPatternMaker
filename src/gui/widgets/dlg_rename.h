#pragma once

class QLineEdit;

#include <QDialog>

class DlgRename : public QDialog
{
public:
    DlgRename(QWidget * parent = nullptr);

    void keyPressEvent(QKeyEvent *evt) override;

    QLineEdit * oldEdit;
    QLineEdit * newEdit;
};
