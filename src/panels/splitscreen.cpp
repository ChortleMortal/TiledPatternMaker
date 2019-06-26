#include "splitscreen.h"


SplitScreen::SplitScreen(QWidget *parent) : QFrame(parent)
{
    grid = nullptr;

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(0);

    setContentsMargins(0,0,0,0);
}

void SplitScreen::addWidgets(QWidget * w1, QWidget * w2)
{
    if (grid)
        delete grid;

    grid = new QGridLayout;
    grid->setSpacing(0);
    grid->setMargin(0);
    setContentsMargins(0,0,0,0);

    grid->addWidget(w1,0,0,1,1,Qt::AlignTop);
    grid->addWidget(w2,0,1,2,1);

    setLayout(grid);
}

