#include <QMouseEvent>
#include "gui/widgets/versioned_list_widget.h"

LoaderListWidget::LoaderListWidget(QWidget *parent) : QListWidget(parent)
{
    setSortingEnabled(false);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");
}

void LoaderListWidget::addItemList(QStringList & list)
{
    clear();
    addItems(list);
}

bool LoaderListWidget::selectItemByName(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    for (int i=0; i < count(); i++)
    {
        QListWidgetItem * qitem = item(i);
        if (qitem->text() == name)
        {
            //qDebug() << "selected" << name;
            setCurrentItem(qitem);
            return true;
        }
    }

    // not found
    return false;
}

bool LoaderListWidget::selectItemByValue(QVariant val)
{
    for (int i=0; i < count(); i++)
    {
        QListWidgetItem * qitem = item(i);
        if (qitem->data(Qt::UserRole) == val)
        {
            //qDebug() << "selected" << val;
            setCurrentItem(qitem);
            return true;
        }
    }

    // not found
    return false;
}

void LoaderListWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        emit rightClick(event->pos());
    }
    else
    {
        QListWidget::mousePressEvent(event);
    }
}

void LoaderListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit leftDoubleClick(event->pos());
    }
    else
    {
        QListWidget::mouseDoubleClickEvent(event);
    }
}

void LoaderListWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit listEnter();
    }
}

VersionedListWidget::VersionedListWidget()
{}

void VersionedListWidget::addItemList(VersionFileList &list)
{
    QStringList slist = list.getNames();
    //slist.sort(Qt::CaseInsensitive);
    LoaderListWidget::addItemList(slist);
}
