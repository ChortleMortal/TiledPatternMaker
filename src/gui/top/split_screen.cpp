#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QScreen>
#include "sys/sys.h"
#include "gui/top/split_screen.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/panels/panel_page.h"
#include "gui/panels/panel_misc.h"

SplitScreen::SplitScreen(QWidget *parent) : QFrame(parent)
{
    setWindowFlag(Qt::Window,true);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    floater = nullptr;

    // left
    vboxL = new AQVBoxLayout();
    vboxL->addWidget(Sys::controlPanel,0,Qt::AlignTop);
    dummy = new QWidget();
    vboxL->addWidget(dummy,0,Qt::AlignHCenter);
    vboxL->addStretch();

    LHS = new QWidget();
    LHS->setLayout(vboxL);

    // together
    hbox  = new AQHBoxLayout();
    hbox->addWidget(LHS,0,Qt::AlignLeft);
    hbox->addWidget(Sys::sysview,0,Qt::AlignLeft | Qt::AlignTop);
    setLayout(hbox);

    QList<QScreen *> screens = qApp->screens();
    QScreen * screen         = screens[0];
    QRect rec                = screen->geometry();
    move(rec.topLeft());

    connect(Sys::sysview, &SystemView::sig_viewSizeChanged, this, &SplitScreen::adjustMe);
    connect(this,         &SplitScreen::sig_adjust,         this, &SplitScreen::adjustMe2, Qt::QueuedConnection);
}

void SplitScreen::addFloater(panel_page * pp)
{
    floater = pp;
    vboxL->replaceWidget(dummy,floater);
    emit sig_adjust();
}

panel_page * SplitScreen::removeFloater()
{
    vboxL->replaceWidget(floater,dummy);
    auto f = floater;
    floater = nullptr;
    emit sig_adjust();
    return f;
}

void SplitScreen::setLHSWidth(int width)
{
    LHS->setFixedWidth(width);
}

void  SplitScreen::adjustMe(QSize,QSize)
{
    adjustSize();
}

void  SplitScreen::adjustMe2()
{
    adjustSize();
}
