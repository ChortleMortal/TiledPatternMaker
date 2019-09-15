#include "splitscreen.h"
#include "panels/panel.h"
#include "base/view.h"


SplitScreen::SplitScreen(QWidget *parent) : QFrame(parent)
{
    grid = nullptr;

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(0);

    setContentsMargins(0,0,0,0);

    vw = nullptr;
    cp = nullptr;
}

void SplitScreen::addWidgets(ControlPanel *panel, View *view)
{
    if (grid)
        delete grid;

    cp = panel;
    vw = view;

    grid = new QGridLayout;
    grid->setSpacing(0);
    grid->setMargin(0);

    setContentsMargins(0,0,0,0);

    grid->addWidget(cp,0,0,Qt::AlignTop);
    grid->addWidget(vw,0,1,-1,-1);

    setLayout(grid);

    connect(panel, &ControlPanel::sig_panelResized, this, &SplitScreen::slot_panelResized, Qt::QueuedConnection);
}

void SplitScreen::slot_panelResized()
{
    if (vw)
    {
        grid->addWidget(vw,0,1,-1,-1);
    }

}


