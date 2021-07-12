#include "panels/view_panel.h"
#include "panels/panel_misc.h"
#include "viewers/view.h"

ViewPanel::ViewPanel() : AQWidget()
{
    view = View::getInstance();

    btnCenter = new QPushButton("Set Center");
    btnPan = new QPushButton("Pan");
    btnRot = new QPushButton("Rotate");
    btnZoom = new QPushButton("Zoom");

    btnCenter->setCheckable(true);
    btnPan->setCheckable(true);
    btnRot->setCheckable(true);
    btnZoom->setCheckable(true);

    btnCenter->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; } QPushButton:checked { background-color: yellow; color: red;}");
    btnPan->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; } QPushButton:checked { background-color: yellow; color: red;}");
    btnRot->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; } QPushButton:checked { background-color: yellow; color: red;}");
    btnZoom->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; } QPushButton:checked { background-color: yellow; color: red;}");

    QLabel    * l_mouseModes    = new QLabel("Mouse control:");

    AQHBoxLayout * layout = new AQHBoxLayout;
    layout->addWidget(l_mouseModes);
    layout->addSpacing(4);
    layout->addWidget(btnCenter);
    layout->addSpacing(4);
    layout->addWidget(btnPan);
    layout->addSpacing(4);
    layout->addWidget(btnRot);
    layout->addSpacing(4);
    layout->addWidget(btnZoom);
    setLayout(layout);

    connect (btnCenter, &QPushButton::clicked, this, &ViewPanel::setSetCenterMode);
    connect (btnPan,    &QPushButton::clicked, this, &ViewPanel::setTranslateMode);
    connect (btnRot,    &QPushButton::clicked, this, &ViewPanel::setRotateMode);
    connect (btnZoom,   &QPushButton::clicked, this, &ViewPanel::setScaleMode);
}

ViewPanel::~ViewPanel()
{
}

void ViewPanel::setTranslateMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btnCenter->setChecked(false);
        btnRot->setChecked(false);
        btnZoom->setChecked(false);
        blockSignals(false);
        view->setMouseMode(MOUSE_MODE_TRANSLATE);
    }
    else
    {
        view->setMouseMode(MOUSE_MODE_NONE);
    }
}

void ViewPanel::setRotateMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btnCenter->setChecked(false);
        btnPan->setChecked(false);
        btnZoom->setChecked(false);
        blockSignals(false);
        view->setMouseMode(MOUSE_MODE_ROTATE);
    }
    else
    {
        view->setMouseMode(MOUSE_MODE_NONE);
    }
}

void ViewPanel::setScaleMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btnCenter->setChecked(false);
        btnPan->setChecked(false);
        btnRot->setChecked(false);
        blockSignals(false);
        view->setMouseMode(MOUSE_MODE_SCALE);
    }
    else
    {
        view->setMouseMode(MOUSE_MODE_NONE);
    }
}


void ViewPanel::setButtonSize(QSize size)
{
    btnCenter->setFixedSize(size);
    btnPan->setFixedSize(size);
    btnRot->setFixedSize(size);
    btnZoom->setFixedSize(size);
}

void ViewPanel::setSetCenterMode(bool checked)
{
    if (checked)
    {
        blockSignals(true);
        btnPan->setChecked(false);
        btnRot->setChecked(false);
        btnZoom->setChecked(false);
        blockSignals(false);
        view->setMouseMode(MOUSE_MODE_CENTER);
    }
    else
    {
        view->setMouseMode(MOUSE_MODE_NONE);
    }
}
