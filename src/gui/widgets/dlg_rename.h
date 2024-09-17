#pragma once
#ifndef DLG_RENAME_H
#define DLG_RENAME_H

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

#endif // DLG_RENAME_H
