#pragma once
#ifndef DLG_LIST_NAME_SELECT_H
#define DLG_LIST_NAME_SELECT_H

#include <QDialog>

#include "gui/widgets/versioned_list_widget.h"
#include "sys/sys/versioning.h"

class DlgListNameSelect : public QDialog
{
    Q_OBJECT

public:
    DlgListNameSelect(VersionFileList & files, int version);

    LoaderListWidget * list;

    QLineEdit * newEdit;

protected slots:
    void slot_currentRow(int row);
};

#endif
