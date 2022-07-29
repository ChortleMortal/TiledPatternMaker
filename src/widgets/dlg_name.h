#ifndef DLG_NAME_H
#define DLG_NAME_H

class QLineEdit;

#include <QDialog>

class DlgName : public QDialog
{
public:
    DlgName(QWidget * parent = nullptr);

    QLineEdit * newEdit;
};

#endif
