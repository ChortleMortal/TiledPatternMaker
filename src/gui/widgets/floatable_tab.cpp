#include <QCloseEvent>
#include <QGuiApplication>
#include <QSettings>
#include <QTabWidget>

#include "gui/panels/panel_page.h"
#include "gui/top/controlpanel.h"
#include "gui/widgets/floatable_tab.h"
#include "sys/sys.h"

FloatableTab::FloatableTab(panel_page * owner)
{
    floating = false;
    owner->registerTab(this);
    connect(Sys::controlPanel, &ControlPanel::sig_raiseDetached, this, &FloatableTab::slot_raiseDetached);
}

void FloatableTab::detach(QTabWidget * tabwidget, QString title)
{
    // floating
    parent   = tabwidget;
    this->title = title;
    floating = true;
    setParent(nullptr);
    setWindowTitle(title);
    show();

    // positioning
    QString name = QString("panel2Tab/%1/pagePos").arg(title);
    QSettings s = Sys::settings();
    QPoint pt    = s.value(name,QPointF()).toPoint();
    if (!pt.isNull())
    {
        QScreen *screenAtOrigin = QGuiApplication::screenAt(pt);
        QScreen *currentScreen = screen();
        if (screenAtOrigin != currentScreen)
        {
            pt = QCursor::pos();
        }
    }
    else
    {
        pt = QCursor::pos();
    }
    move(pt);
}

void FloatableTab::reattach()
{
    savePosition();

    // reattach
    setParent(parent);
    parent->addTab(this,title);
    floating = false;
}

// captures current position without reattaching, so a floated tab that
// is never manually re-docked still has its position saved on exit
void FloatableTab::savePosition()
{
    QString name = QString("panel2Tab/%1/pagePos").arg(title);
    QPoint pt = pos();
    QSettings s = Sys::settings();
    s.setValue(name,pt);
}

void FloatableTab::closeEvent(QCloseEvent * event)
{
    if (floating)
    {
        event->setAccepted(false);
        reattach();
    }
}

void FloatableTab::slot_raiseDetached()
{
    if (floating)
    {
        raise();
    }
}
