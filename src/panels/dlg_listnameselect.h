#ifndef DLG_LIST_NAME_SELECT_H
#define DLG_LIST_NAME_SELECT_H

#include <QDialog>
#include "panels/versioned_list_widget.h"

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
