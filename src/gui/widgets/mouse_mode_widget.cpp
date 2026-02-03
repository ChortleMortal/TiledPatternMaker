#include <QCheckBox>

#include "gui/widgets/mouse_mode_widget.h"
#include "sys/sys.h"
#include "gui/panels/panel_misc.h"

MouseModeWidget::MouseModeWidget() : QWidget()
{
    setContentsMargins(0,0,0,0);

    chkPan    = new QCheckBox("Pan");
    chkRot    = new QCheckBox("Rotate");
    chkZoom   = new QCheckBox("Zoom");

    chkPan->setCheckable(true);
    chkRot->setCheckable(true);
    chkZoom->setCheckable(true);

    chkPan->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");
    chkRot->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");
    chkZoom->setStyleSheet("QCheckBox:checked { background-color: yellow; color: red;}");

    QLabel    * l_mouseModes    = new QLabel("Mouse:");

    AQHBoxLayout * layout = new AQHBoxLayout;
    layout->addWidget(l_mouseModes);
    layout->addSpacing(3);
    layout->addWidget(chkPan);
    layout->addSpacing(3);
    layout->addWidget(chkRot);
    layout->addSpacing(3);
    layout->addWidget(chkZoom);
    layout->addSpacing(3);
    setLayout(layout);

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

    chkRot->setChecked(Sys::getSysMouseMode(MOUSE_MODE_ROTATE));
    chkZoom->setChecked(Sys::getSysMouseMode(MOUSE_MODE_SCALE));
    chkPan->setChecked(Sys::getSysMouseMode(MOUSE_MODE_TRANSLATE));

    blockSignals(false);
}

void MouseModeWidget::setTranslateMode(bool checked)
{
    Sys::setSysMouseMode(MOUSE_MODE_TRANSLATE,checked);
    display();
}

void MouseModeWidget::setRotateMode(bool checked)
{
    Sys::setSysMouseMode(MOUSE_MODE_ROTATE,checked);
    display();
}

void MouseModeWidget::setScaleMode(bool checked)
{
    Sys::setSysMouseMode(MOUSE_MODE_SCALE,checked);
    display();
}

