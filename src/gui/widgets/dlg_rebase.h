#pragma once

#include <QDialog>

class DlgRebase : public QDialog
{
public:
    DlgRebase(QWidget * parent = nullptr);

    class SpinSet * oldVersion;
    class SpinSet * newVersion;
};
