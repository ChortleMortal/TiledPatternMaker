#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QScreen>
#include "panels/splitscreen.h"
#include "panels/panel.h"
#include "viewers/viewcontrol.h"


SplitScreen::SplitScreen(QWidget *parent) : QFrame(parent)
{
    grid = nullptr;

    panel = ControlPanel::getInstance();
    view  = ViewControl::getInstance();

    //setFrameStyle(QFrame::Box | QFrame::Plain);
    //setLineWidth(0);
    //setContentsMargins(0,0,0,0);

    addWidgets();

    QScreen * sc = qApp->screenAt(panel->pos());
    QRect  rec   = sc->geometry();

    setFixedSize(  rec.width(),rec.height());
    setMinimumSize(rec.width(),rec.height());
    setMaximumSize(rec.width(),rec.height());
    move(rec.topLeft());
}

void SplitScreen::addWidgets()
{
    grid = new QGridLayout;

    //grid->setSpacing(0);
    //grid->setMargin(0);
    //grid->setContentsMargins(0,0,0,0);
    grid->addWidget(panel,0,0,Qt::AlignTop);
    grid->addWidget(view,0,1);

    setLayout(grid);

    connect(panel, &ControlPanel::sig_panelResized, this, &SplitScreen::slot_panelResized, Qt::QueuedConnection);
}

void SplitScreen::slot_panelResized()
{
    grid->addWidget(view,0,1);
}


