#include <QCheckBox>

#include "gui/widgets/mouse_mode_widget.h"
#include "gui/viewers/gui_modes.h"
#include "sys/sys.h"
#include "gui/panels/panel_misc.h"

MouseModeWidget::MouseModeWidget() : QWidget()
{
    setContentsMargins(0,0,0,0);

    chkCenter = new QCheckBox("Set Center");
    chkPan    = new QCheckBox("Pan");
    chkRot    = new QCheckBox("Rotate");
    chkZoom   = new QCheckBox("Zoom");

    chkCenter->setCheckable(true);
    chkPan->setCheckable(true);
    chkRot->setCheckable(true);
    chkZoom->setCheckable(true);

    chkCenter->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");
    chkPan->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");
    chkRot->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");
    chkZoom->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");

    QLabel    * l_mouseModes    = new QLabel("Mouse:");

    AQHBoxLayout * layout = new AQHBoxLayout;
    layout->addWidget(l_mouseModes);
    layout->addSpacing(7);
    layout->addWidget(chkCenter);
    layout->addSpacing(4);
    layout->addWidget(chkPan);
    layout->addSpacing(4);
    layout->addWidget(chkRot);
    layout->addSpacing(4);
    layout->addWidget(chkZoom);
    setLayout(layout);

    connect (chkCenter, &QPushButton::clicked, this, &MouseModeWidget::setSetCenterMode);
    connect (chkPan,    &QPushButton::clicked, this, &MouseModeWidget::setTranslateMode);
    connect (chkRot,    &QPushButton::clicked, this, &MouseModeWidget::setRotateMode);
    connect (chkZoom,   &QPushButton::clicked, this, &MouseModeWidget::setScaleMode);
}

MouseModeWidget::~MouseModeWidget()
{
}

void MouseModeWidget::display()
{
    blockSignals(true);

    chkCenter->setChecked(Sys::guiModes->getMouseMode(MOUSE_MODE_CENTER));
    chkRot->setChecked(Sys::guiModes->getMouseMode(MOUSE_MODE_ROTATE));
    chkZoom->setChecked(Sys::guiModes->getMouseMode(MOUSE_MODE_SCALE));
    chkPan->setChecked(Sys::guiModes->getMouseMode(MOUSE_MODE_TRANSLATE));

    blockSignals(false);
}

void MouseModeWidget::setTranslateMode(bool checked)
{
    Sys::guiModes->setMouseMode(MOUSE_MODE_TRANSLATE,checked);
    display();
}

void MouseModeWidget::setRotateMode(bool checked)
{
    Sys::guiModes->setMouseMode(MOUSE_MODE_ROTATE,checked);
    display();
}

void MouseModeWidget::setScaleMode(bool checked)
{
    Sys::guiModes->setMouseMode(MOUSE_MODE_SCALE,checked);
    display();
}

void MouseModeWidget::setSetCenterMode(bool checked)
{
    Sys::guiModes->setMouseMode(MOUSE_MODE_CENTER,checked);
    display();
}

