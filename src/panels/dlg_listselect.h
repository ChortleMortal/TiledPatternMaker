#ifndef DLG_LISTSELECT_H
#define DLG_LISTSELECT_H

#include <QDialog>
#include "panels/panel_misc.h"

class DlgListSelect : public QDialog
{
    Q_OBJECT

public:
    DlgListSelect(QStringList files);

    LoaderListWidget * list;

    QString selectedFile;

protected slots:
    void slot_currentRow(int row);
};

#endif
