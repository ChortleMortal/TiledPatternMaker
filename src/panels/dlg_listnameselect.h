#ifndef DLG_LIST_NAME_SELECT_H
#define DLG_LIST_NAME_SELECT_H

#include <QDialog>
#include "panels/panel_misc.h"

class DlgListNameSelect : public QDialog
{
    Q_OBJECT

public:
    DlgListNameSelect(QStringList files);

    LoaderListWidget * list;

    QLineEdit * newEdit;

protected slots:
    void slot_currentRow(int row);
};

#endif
