#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QLineEdit>

#include "widgets/worklist_widget.h"
#include "widgets/dlg_name.h"
#include "settings/configuration.h"

//////////////////////////////////////////////////////////
///
///  WorklistWidget
///
//////////////////////////////////////////////////////////

WorklistWidget::WorklistWidget(QWidget *parent) : QListWidget(parent)
{
    setFixedHeight(700);
}

void WorklistWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        QPoint pt = event->localPos().toPoint();
#else
        QPoint pt = event->position().toPoint();
#endif
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
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        menu.exec(event->screenPos().toPoint());
#else
        menu.exec(event->globalPosition().toPoint());
#endif
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

    Configuration * config = Configuration::getInstance();
    config->worklist.add(newName);

    clear();
    addItems(config->worklist.get());
    setCurrentRow(config->worklist.get().size()-1);
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

    Configuration * config = Configuration::getInstance();
    const QStringList & list = config->worklist.get();

    QStringList newList;
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i) != name)
        {
            newList << list.at(i);
        }
        else
        {
            newList << newName;
        }
    }

    newList.sort();
    auto existingName = config->worklist.getName();
    config->worklist.set(existingName,newList);

    clear();
    addItems(newList);
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

    Configuration * config = Configuration::getInstance();
    const QStringList & list = config->worklist.get();

    QStringList newList;
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i) != name)
        {
            newList << list.at(i);
        }
    }

    auto existingName = config->worklist.getName();
    config->worklist.set(existingName,newList);

    clear();
    addItems(newList);
    setCurrentRow(row);
    update();
}


