#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QHeaderView>

#include "widgets/panel_misc.h"

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
    //setMargin(0);
    setContentsMargins(0,0,0,0);
}

AQHBoxLayout::AQHBoxLayout() : QHBoxLayout()
{
    setAlignment(Qt::AlignLeft);
    setSpacing(0);
    //setMargin(0);
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

AQComboBox::AQComboBox(QWidget * parent) : QComboBox(parent)
{}

void AQComboBox::select(int id)
{
    blockSignals(true);
    int row = findData(id);
    setCurrentIndex(row);
    blockSignals(false);
}

//////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////

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
    setCustomColor(index++,0x39426d);
    setCustomColor(index++,0x34554a);
    setCustomColor(index++,0x10100e);
    setCustomColor(index++,0xc2bcb0);
    setCustomColor(index++,0x382310);
    setCustomColor(index++,0x632E1C);
    setCustomColor(index++,0xffe05b);
    setCustomColor(index++,0xa35807);
    setCustomColor(index++,0x1840b2);
    setCustomColor(index++,0x234b30);
    setCustomColor(index++,0xc59c0c);
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

void AQTableWidget::selectRow(int row)
{
    if (row < rowCount())
    {
        qDebug() << "select row" << row;
        QTableWidget::selectRow(row);
    }
}

void AQTableWidget::adjustTableSize(int maxWidth, int maxHeight)
{
    int w = getTableWidth(maxWidth);
    int h = getTableHeight(maxHeight);
    QSize size(w,h);
    setFixedSize(size);
    //qDebug() << "adjusted size" << size;
    resize(size);
    updateGeometry();
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

    QHeaderView * hh = horizontalHeader();
    if (hh->count())
    {
        //qDebug() << "header" << hh->height();
        h += hh->height();
    }

    QScrollBar * sb = horizontalScrollBar();
    if (sb->isVisible())
    {
        //qDebug() << "scroll" << sb->height();
        h += sb->height();
    }

    //qDebug() << "rows" << rowCount();
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

AQPushButton::AQPushButton(const QString &text, QWidget * parent) : QPushButton(text,parent)
{
    setCheckable(true);
    setStyleSheet("QPushButton:checked{background-color:LightGreen}");
}

BQPushButton::BQPushButton(const QString &text, QWidget * parent) : QPushButton(text,parent)
{
    setStyleSheet("QPushButton:pressed{background-color:LightGreen}");
}
