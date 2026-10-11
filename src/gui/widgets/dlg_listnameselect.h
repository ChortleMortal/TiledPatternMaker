#pragma once

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
