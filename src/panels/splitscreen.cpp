#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QScreen>
#include "misc/sys.h"
#include "panels/splitscreen.h"
#include "panels/controlpanel.h"
#include "viewers/view.h"
#include "panels/panel_page.h"

SplitScreen::SplitScreen(QWidget *parent) : QFrame(parent)
{
    setWindowFlag(Qt::Window,true);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    panel   = ControlPanel::getInstance();
    view    = Sys::view;
    floater = nullptr;

    // left
    vboxL = new QVBoxLayout();
    vboxL->addWidget(panel,0,Qt::AlignTop);
    dummy = new QWidget();
    vboxL->addWidget(dummy,0,Qt::AlignHCenter);
    vboxL->addStretch();

    LHS = new QWidget();
    LHS->setLayout(vboxL);

    // together
    hbox  = new QHBoxLayout();
    hbox->addWidget(LHS,0,Qt::AlignLeft);
    hbox->addWidget(view,0,Qt::AlignLeft | Qt::AlignTop);
    setLayout(hbox);

    QScreen * sc = qApp->screenAt(panel->pos());
    QRect rec    = sc->geometry();
    move(rec.topLeft());

    connect(view, &View::sig_viewSizeChanged, this, &SplitScreen::adjustMe);
    connect(this, &SplitScreen::sig_adjust,   this, &SplitScreen::adjustMe2, Qt::QueuedConnection);
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
