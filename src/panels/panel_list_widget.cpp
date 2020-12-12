#include "panels/panel_list_widget.h"

//////////////////////////////////////////////////////////
/// PanelListWidget
//////////////////////////////////////////////////////////

PanelListWidget::PanelListWidget(QWidget *parent) : QListWidget(parent)
{
    separators = 0;
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizeAdjustPolicy(QListWidget::AdjustToContents);
    setResizeMode(QListWidget::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");


}

void PanelListWidget::hide(QString name)
{
    qDebug() << "hiding:" << name;
    QList<QListWidgetItem*> ql = findItems(name, Qt::MatchExactly);
    if (ql.count())
    {
        QListWidgetItem * qlwi = ql[0];
        qlwi->setHidden(true);
    }
}

void PanelListWidget::show(QString name)
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

void PanelListWidget::setCurrentRow(QString name)
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

void PanelListWidget::setCurrentRow(int row)
{
    QListWidgetItem * litem = item(row);
    while (row && (litem->text().isEmpty() ||  litem->isHidden()) )
    {
        litem = item(--row);  // assumes row 0 is not a separator
    }

    QListWidget::setCurrentRow(row);
}

void PanelListWidget::slot_floatAction()
{
    QListWidgetItem * qlwi = item(floatIndex);
    QString name = qlwi->text();
    qDebug() << "trigger float" << floatIndex << name;

    emit sig_detachWidget(name);
}

void PanelListWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
    {
        QPoint pt = event->localPos().toPoint();
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
            menu.exec(event->globalPos());
         }
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

void PanelListWidget::addSeparator()
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

void PanelListWidget::establishSize()
{
    int rowCount = count() - separators;
    setFixedSize(sizeHintForColumn(0) + (2 * frameWidth()), (sizeHintForRow(0) * rowCount) + (10 * separators) + (2 * frameWidth()) + 5);   // 5 is a little pad
}

