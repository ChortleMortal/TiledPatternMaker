#include "panels/view_panel.h"
#include "base/workspace.h"

ViewPanel::ViewPanel() : QWidget()
{
    workspace = Workspace::getInstance();

    btn1 = new QPushButton("Pan");
    btn2 = new QPushButton("Rotate");
    btn3 = new QPushButton("Zoom");

    btn1->setCheckable(true);
    btn2->setCheckable(true);
    btn3->setCheckable(true);

    btn1->setStyleSheet("QPushButton:checked { background-color: yellow; color: red; }");
    btn2->setStyleSheet("QPushButton:checked { background-color: yellow; color: red;}");
    btn3->setStyleSheet("QPushButton:checked { background-color: yellow; color: red;}");

    AQHBoxLayout * layout = new AQHBoxLayout;
    layout->addWidget(btn1);
    layout->addSpacing(5);
    layout->addWidget(btn2);
    layout->addSpacing(5);
    layout->addWidget(btn3);
    setLayout(layout);

    connect (btn1, &QPushButton::clicked, this, &ViewPanel::setTranslateMode);
    connect (btn2, &QPushButton::clicked, this, &ViewPanel::setRotateMode);
    connect (btn3, &QPushButton::clicked, this, &ViewPanel::setScaleMode);
}

ViewPanel::~ViewPanel()
{
}

void  ViewPanel::setTranslateMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btn2->setChecked(false);
        btn3->setChecked(false);
        blockSignals(false);
        workspace->setMouseMode(MOUSE_MODE_TRANSLATE);
    }
    else
    {
        workspace->setMouseMode(MOUSE_MODE_NONE);
    }
}

void  ViewPanel::setRotateMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btn1->setChecked(false);
        btn3->setChecked(false);
        blockSignals(false);
        workspace->setMouseMode(MOUSE_MODE_ROTATE);
    }
    else
    {
        workspace->setMouseMode(MOUSE_MODE_NONE);
    }
}

void  ViewPanel::setScaleMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btn1->setChecked(false);
        btn2->setChecked(false);
        blockSignals(false);
        workspace->setMouseMode(MOUSE_MODE_SCALE);
    }
    else
    {
        workspace->setMouseMode(MOUSE_MODE_NONE);
    }
}


void  ViewPanel::setButtonSize(QSize size)
{
    btn1->setFixedSize(size);
    btn2->setFixedSize(size);
    btn3->setFixedSize(size);
}
