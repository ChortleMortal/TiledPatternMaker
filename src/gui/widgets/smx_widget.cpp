#include <QHBoxLayout>

#include "gui/widgets/panel_misc.h"
#include "gui/widgets/smx_widget.h"
#include "gui/viewers/layer.h"

SMXWidget::SMXWidget(Layer * layer, bool abbrev, bool box)
{
    _layer = layer;
    _box   = box;
    _selected = false;

    setContentsMargins(0,0,0,0);

    if (abbrev)
    {
        chkLock      = new BQCheckBox("Lock");
        chkSolo      = new BQCheckBox("Solo");
        chkBreakaway = new BQCheckBox("Indp");
    }
    else
    {
        chkLock      = new BQCheckBox("Lock View");
        chkSolo      = new BQCheckBox("Solo View");
        chkBreakaway = new BQCheckBox("Independent");
    }

    connect(chkLock,  &QCheckBox::clicked, this, &SMXWidget::slot_lock);
    connect(chkSolo,  &QCheckBox::clicked, this, &SMXWidget::slot_solo);
    connect(chkBreakaway, &QCheckBox::clicked, this, &SMXWidget::slot_breakaway);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addSpacing(4);
    hbox->addWidget(chkBreakaway);
    hbox->addWidget(chkLock);
    hbox->addWidget(chkSolo);

    setLayout(hbox);
}

void SMXWidget::setSelected(bool s)
{
    if (_selected == s)
        return;

    _selected = s;

    chkLock->selected = s;
    chkLock->update();
    chkSolo->selected = s;
    chkSolo->update();
    chkBreakaway->selected = s;
    chkBreakaway->update();
}

void SMXWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (_box)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);

        QPen pen(QColor(0x7f7f7f),1);   // works for both ligh and dasrk themes
        painter.setPen(pen);

        // Draw rectangle around widget’s rect
        QRect r = rect();
        r.adjust(0, 0, -5, -5);
        painter.drawRoundedRect(r,20,15);
    }
}

void SMXWidget::refresh()
{
    if (_layer)
    {
        chkLock->setChecked(_layer->isLocked());
        chkSolo->setChecked(_layer->isSolo());
        chkBreakaway->setChecked(_layer->isBreakaway());
    }
}

void  SMXWidget::slot_breakaway(bool enb)
{
    if (_layer)
    {
        emit Sys::viewController->sig_breakaway(_layer,enb);
        emit _layer->sig_updateView();
    }
}

void SMXWidget::slot_lock(bool enb)
{
    if (_layer)
    {
        emit Sys::viewController->sig_lock(_layer,enb);
        emit _layer->sig_updateView();
    }
}

void SMXWidget::slot_solo(bool enb)
{
    if (_layer)
    {
        emit Sys::viewController->sig_solo(_layer,enb);
        emit _layer->sig_updateView();
    }
}

//////////////////////////////////////////////////////////
///
///   BQCheckBox
///
//////////////////////////////////////////////////////////

BQCheckBox::BQCheckBox(const QString &text, QWidget *parent)  : QCheckBox(text, parent)
{
    selected = false;
}

void BQCheckBox::paintEvent(QPaintEvent *e)
{
    if (Sys::isDarkTheme)
    {
        // Force the text color before painting
        QPalette pal = this->palette();
        pal.setColor(QPalette::WindowText, selected ? Qt::red : Qt::white);
        this->setPalette(pal);
    }
    QCheckBox::paintEvent(e);
}
