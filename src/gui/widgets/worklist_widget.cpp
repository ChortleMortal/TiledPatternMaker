#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QLineEdit>

#include "gui/widgets/worklist_widget.h"
#include "gui/widgets/dlg_name.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"

//////////////////////////////////////////////////////////
///
///  WorklistWidget
///
//////////////////////////////////////////////////////////

WorklistWidget::WorklistWidget(QWidget *parent) : QListWidget(parent)
{
    setFixedHeight(700);
    setSortingEnabled(false);
}

void WorklistWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
    {
        QPoint pt = event->position().toPoint();
        QMenu menu(this);
        QListWidgetItem * qlwi = itemAt(pt);
        if (qlwi)
        {
            QString name = qlwi->text();
            qDebug() << "right click=" << name;

            menu.addSection(name);
            menu.addAction("Insert",this,&WorklistWidget::slot_insertAction);
            menu.addAction("Rename",this,&WorklistWidget::slot_editAction);
            menu.addAction("Delete",this,&WorklistWidget::slot_deleteAction);
        }
        else
        {
            menu.addAction("Insert",this,&WorklistWidget::slot_insertAction);
        }
        menu.exec(event->globalPosition().toPoint());
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

void WorklistWidget::slot_insertAction()
{
    DlgName dlg(this);
    dlg.newEdit->setText("No-Name");
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString newName = dlg.newEdit->text();
    if (newName.isEmpty())
        return;

    VersionedName vn(newName);
    Sys::config->worklist.add(vn);

    clear();
    addItems(Sys::config->worklist.get().getNames());
    setCurrentRow(Sys::config->worklist.count() - 1);
    update();
}

void WorklistWidget::slot_editAction()
{
    int row = currentRow();
    if (row == -1)
        return;

    QListWidgetItem * qlwi = item(row);
    QString name = qlwi->text();
    qDebug() << "trigger edit" << row << name;

    DlgName dlg(this);
    dlg.newEdit->setText(name);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    QString newName = dlg.newEdit->text();
    if (newName.isEmpty())
        return;

    const VersionList & list = Sys::config->worklist.get();

    VersionList newList;
    for (const VersionedName & vname : list)
    {
        if (vname.get() != name)
        {
            newList.add(vname);
        }
        else
        {
            VersionedName vn(newName);
            newList.add(vn);
        }
    }

    newList.sort();

    auto existingName = Sys::config->worklist.getName();
    Sys::config->worklist.set(existingName,newList);

    clear();
    addItems(newList.getNames());
    setCurrentRow(row);
    update();
}

void WorklistWidget::slot_deleteAction()
{
    int row = currentRow();
    if (row == -1)
        return;

    QListWidgetItem * qlwi = item(row);
    QString name = qlwi->text();
    qDebug() << "trigger delete" << row << name;

    const VersionList & list = Sys::config->worklist.get();

    VersionList newList;
    for (const VersionedName & vname : list)
    {
        if (vname.get() != name)
        {
            newList.add(vname);
        }
    }

    auto existingName = Sys::config->worklist.getName();
    Sys::config->worklist.set(existingName,newList);

    clear();
    addItems(newList.getNames());
    setCurrentRow(row);
    update();
}


