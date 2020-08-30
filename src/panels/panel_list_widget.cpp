#include "panels/panel_list_widget.h"

//////////////////////////////////////////////////////////
/// PanelListWidget
//////////////////////////////////////////////////////////

PanelListWidget::PanelListWidget(QWidget *parent) : QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizeAdjustPolicy(QListWidget::AdjustToContents);
    setResizeMode(QListWidget::Adjust);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");

    floatAction = new QAction("Float",this);
    connect(floatAction, &QAction::triggered,this, &PanelListWidget::slot_floatAction);
}

void PanelListWidget::removeItem(QString name)
{
    QList<QListWidgetItem*> ql = findItems(name, Qt::MatchExactly);
    for (auto it = ql.begin(); it != ql.end(); it++)
    {
        QListWidgetItem * qlwi = *it;
        removeItemWidget(qlwi);
        delete qlwi;
    }
}

void PanelListWidget::setCurrentRow(QString name)
{
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

void PanelListWidget::slot_floatAction()
{
    QListWidgetItem * qlwi = takeItem(floatIndex);
    QString name = qlwi->text();
    qDebug() << "trigger float" << floatIndex << name;

    emit detachWidget(name);
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
            qDebug() << "right click=" << index;

            QMenu menu(this);
            menu.addAction(floatAction);
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
}

QSize PanelListWidget::sizeHint() const
{
    int height = count() * 15;  // kludge alert
    return QSize(92, height);
}

void PanelListWidget::establishHeight()
{
    int height = count() * 15;  // kludge alert
    setFixedHeight(height);
}
