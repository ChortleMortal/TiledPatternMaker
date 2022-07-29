#ifndef DLG_REBASE_H
#define DLG_REBASE_H

#include <QDialog>

class DlgRebase : public QDialog
{
public:
    DlgRebase(QWidget * parent = nullptr);

    class SpinSet * oldVersion;
    class SpinSet * newVersion;
};

#endif // DLG_REBASE_H
