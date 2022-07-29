#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QLineEdit>

#include "widgets/panel_list_widget.h"
#include "widgets/dlg_name.h"
#include "settings/configuration.h"

//////////////////////////////////////////////////////////
///  ListListWidget
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
        QListWidgetItem * qlwi = itemAt(pt);
        QString name = qlwi->text();
        qDebug() << "right click=" << name;

        QMenu menu(this);
        menu.addSection(name);
        menu.addAction("Rename",this,&WorklistWidget::slot_editAction);
        menu.addAction("Delete",this,&WorklistWidget::slot_deleteAction);
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

    QStringList newList;
    Configuration * config = Configuration::getInstance();
    for (int i = 0; i < config->workList.size(); ++i)
    {
        if (config->workList.at(i) != name)
        {
            newList << config->workList.at(i);
        }
        else
        {
            newList << newName;
        }
    }

    newList.sort();
    config->workList = newList;

    clear();
    addItems(config->workList);
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

    QStringList newList;
    for (int i = 0; i < config->workList.size(); ++i)
    {
        if (config->workList.at(i) != name)
        {
            newList << config->workList.at(i);
        }
    }
    config->workList = newList;

    clear();
    addItems(config->workList);
    setCurrentRow(row);
    update();
}


//////////////////////////////////////////////////////////
/// PanelListWidget
//////////////////////////////////////////////////////////

PanelListWidget::PanelListWidget(QWidget *parent) : AQListWidget(parent)
{
}

void PanelListWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        QPoint pt = event->localPos().toPoint();
#else
        QPoint pt = event->position().toPoint();
#endif
        QListWidgetItem * qlwi = itemAt(pt);
        bool found = false;
        int index = 0;
        while (!found && index < count())
        {
            if (item(index) == qlwi)
                found = true;
            else
                index++;
        }

        if (found)
        {
            floatIndex =  index;
            QString name = qlwi->text();
            qDebug() << "right click=" << index << name;

            QMenu menu(this);
            menu.addSection(name);
            menu.addAction("Float",this,&PanelListWidget::slot_floatAction);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            menu.exec(event->screenPos().toPoint());
#else
            menu.exec(event->globalPosition().toPoint());
#endif
        }
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

void PanelListWidget::slot_floatAction()
{
    QListWidgetItem * qlwi = item(floatIndex);
    QString name = qlwi->text();
    qDebug() << "trigger float" << floatIndex << name;

    emit sig_detachWidget(name);
}

//////////////////////////////////////////////////////////
/// AQListWidget
//////////////////////////////////////////////////////////
///
AQListWidget::AQListWidget(QWidget *parent) : QListWidget(parent)
{
    separators = 0;
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizeAdjustPolicy(QListWidget::AdjustToContents);
    setResizeMode(QListWidget::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");
}

void AQListWidget::hide(QString name)
{
    qDebug() << "hiding:" << name;
    QList<QListWidgetItem*> ql = findItems(name, Qt::MatchExactly);
    if (ql.count())
    {
        QListWidgetItem * qlwi = ql[0];
        qlwi->setHidden(true);
    }
}

void AQListWidget::show(QString name)
{
    for (int i=0; i < count(); ++i)
    {
        QListWidgetItem * qlwi = item(i);
        qDebug() << "item:" << qlwi->text();
        if (qlwi->text() == name)
        {
            qlwi->setHidden(false);
            return;
        }
    }
}

void AQListWidget::setCurrentRow(QString name)
{
    qDebug() <<  "Looking for:" << name;
    QList<QListWidgetItem*> ql = findItems(name, Qt::MatchExactly);
    if (ql.count())
    {
        QListWidgetItem * qlwi = ql[0];
        if (qlwi)
        {
            setCurrentItem(qlwi);
            return;
        }
    }
    setCurrentRow("Load");     // default
}

void AQListWidget::setCurrentRow(int row)
{
    QListWidgetItem * litem = item(row);
    while (row && (litem->text().isEmpty() ||  litem->isHidden()) )
    {
        litem = item(--row);  // assumes row 0 is not a separator
    }

    QListWidget::setCurrentRow(row);
}



void AQListWidget::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
}


void AQListWidget::addSeparator()
{
    QListWidgetItem * item = new QListWidgetItem();
    item->setSizeHint(QSize(20,10));
    item->setFlags(Qt::NoItemFlags);
    addItem(item);

    QFrame * frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);

    setItemWidget(item,frame);

    separators++;
}

void AQListWidget::establishSize()
{
    int rowCount = count() - separators;
    setFixedSize(sizeHintForColumn(0) + (2 * frameWidth()), (sizeHintForRow(0) * rowCount) + (10 * separators) + (2 * frameWidth()) + 5);   // 5 is a little pad
}

