/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panel_misc.h"

AQWidget::AQWidget(QWidget * parent) : QWidget(parent)
{
    setContentsMargins(0,0,0,0);
}

AQStackedWidget::AQStackedWidget() : QStackedWidget()
{
    setContentsMargins(0,0,0,0);
}

AQVBoxLayout::AQVBoxLayout() : QVBoxLayout()
{
    setAlignment(Qt::AlignTop);
    setSpacing(0);
    setMargin(0);
    setContentsMargins(0,0,0,0);
}

AQHBoxLayout::AQHBoxLayout() : QHBoxLayout()
{
    setAlignment(Qt::AlignLeft);
    setSpacing(0);
    setMargin(0);
    setContentsMargins(0,0,0,0);
}

void AQLineEdit::mousePressEvent(QMouseEvent * event)
{
    event->ignore();
}

AQLineEdit::AQLineEdit(QWidget * parent) : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
}


//////////////////////////////////////////////////////////
/// PanelListWidget
//////////////////////////////////////////////////////////

PanelListWidget::PanelListWidget(QWidget *parent) : QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);

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
        setCurrentItem(qlwi);
    }
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


AQColorDialog::AQColorDialog(QWidget * parent) : QColorDialog(parent)
{
    setOption(QColorDialog::ShowAlphaChannel);
    setOption(QColorDialog::DontUseNativeDialog);
    set_CustomColors();
}

AQColorDialog::AQColorDialog(const QColor & initial, QWidget * parent) : QColorDialog(parent)
{
    setOption(QColorDialog::ShowAlphaChannel);
    setOption(QColorDialog::DontUseNativeDialog);
    setCurrentColor(initial);
    set_CustomColors();
}

void AQColorDialog::set_CustomColors()
{
    int index = 0;
    setCustomColor(index++,"#39426d");
    setCustomColor(index++,"#34554a");
    setCustomColor(index++,"#10100e");
    setCustomColor(index++,"#c2bcb0");
    setCustomColor(index++,"#382310");
    setCustomColor(index++,"#632E1C");
    setCustomColor(index++,"#ffe05b");
    setCustomColor(index++,"#a35807");
    setCustomColor(index++,"#1840b2");
    setCustomColor(index++,"#234b30");
    setCustomColor(index++,"#c59c0c");
}


LoaderListWidget::LoaderListWidget(QWidget *parent) : QListWidget(parent)
{
    setMouseTracking(true);
    setStyleSheet("QListWidget::item:selected { background:yellow; color:red; }");
}

void LoaderListWidget::addItemList(QStringList list)
{
    clear();
    for (int i = 0; i < list.size(); i++)
    {
        addItem(list[i]);
    }
}

bool LoaderListWidget::selectItemByName(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    for (int i=0; i < count(); i++)
    {
        QListWidgetItem * qitem = item(i);
        if (qitem->text() == name)
        {
            qDebug() << "selected" << name;
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
            qDebug() << "selected" << val;
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


ClickableLabel::ClickableLabel(QWidget * parent) : QLabel(parent)
{
}

ClickableLabel::~ClickableLabel()
{
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}

AQLabel::AQLabel() : QLabel()
{
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &AQLabel::sig_close, this, &AQLabel::close, Qt::QueuedConnection);
}

void AQLabel::keyPressEvent( QKeyEvent *k )
{
    int key = k->key();
    if (key == Qt::Key_Space)
    {
        emit sig_takeNext();
        emit sig_close();       // foces take before close
    }
    else if (key == 'Q')
    {
        emit sig_cyclerQuit();
        close();
    }
    else if (key == 'V')
    {
        emit sig_view_images(); // all three are now visible
    }
    else if (key == 'L')
    {
        qWarning() << "FILE LOGGED (needs attention)";
        emit sig_takeNext();
        close();
    }
}

void eraseLayout(QLayout * layout)
{
    while(layout->count() > 0)
    {
        QLayoutItem *item = layout->takeAt(0);

        QWidget* widget = item->widget();
        if(widget)
        {
            delete widget;
        }
        else
        {
            QLayout * layout = item->layout();
            if (layout)
            {
                eraseLayout(layout);
            }
            else
            {
                QSpacerItem * si = item->spacerItem();
                if (si)
                {
                    delete si;
                }
            }
        }
        delete item;
    }
}

AQTableWidget::AQTableWidget(QWidget * parent) : QTableWidget(parent)
{

}

void AQTableWidget::adjustTableSize(int maxWidth, int maxHeight)
{
    int w = getTableWidth(maxWidth);
    int h = getTableHeight(maxHeight);
    QSize size(w,h);
    setMaximumSize(size);
    setMinimumSize(size);
}

void AQTableWidget::adjustTableWidth(int maxWidth)
{
    int w = getTableWidth(maxWidth);
    setMaximumWidth(w);
    setMinimumWidth(w);
}

void AQTableWidget::adjustTableHeight(int maxHeight)
{
    int h = getTableHeight(maxHeight);
    setMaximumHeight(h);
    setMinimumHeight(h);
}

int AQTableWidget::getTableWidth(int maxWidth)
{
    int w = frameWidth() * 2;

    if (verticalHeader()->count())
    {
        w += verticalHeader()->width(); // +4 seems to be needed
    }

    if (verticalScrollBar()->isVisible())
    {
        w += verticalScrollBar()->width();
    }

    for (int i = 0; i < columnCount(); i++)
    {
        w += columnWidth(i);
    }

    if (maxWidth)
    {
        if (w > maxWidth)
        {
            w = maxWidth;
        }
    }

    return w;
}

int AQTableWidget::getTableHeight(int maxHeight)
{
    int h = frameWidth() * 2;    // width and height are same

    if (horizontalHeader()->count())
    {
        h += horizontalHeader()->height();
    }

    if (horizontalScrollBar()->isVisible())
    {
        h += horizontalScrollBar()->height();
    }

    for (int i = 0; i < rowCount(); i++)
    {
        h += rowHeight(i);
    }

    if (maxHeight)
    {
        if (h > maxHeight)
        {
            h = maxHeight;
        }
    }

    return h;
}
