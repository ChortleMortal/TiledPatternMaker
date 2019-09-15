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


class GirihListSelect : public DlgListSelect
{
    Q_OBJECT

public:
    GirihListSelect(QStringList names);
    QStringList getSelected();

public slots:
    void slot_rightClick(QPoint pt);

private slots:
    void whereUsed();

protected:
    bool isUsed(QString girihname, QStringList & results);
    bool containsGirih(QString girihName, QString filename);
};
#endif
