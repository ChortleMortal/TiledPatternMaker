#ifndef MOUSEMODEWIDGET_H
#define MOUSEMODEWIDGET_H

class QCheckBox;

#include "widgets/panel_misc.h"

class MouseModeWidget : public AQWidget
{
    Q_OBJECT

public:
    MouseModeWidget();
    ~MouseModeWidget();

    void    setSetCenterMode(bool checked);
    void    setTranslateMode(bool checked);
    void    setRotateMode(bool checked);
    void    setScaleMode(bool checked);

    void display();

protected:
    QCheckBox * chkCenter;
    QCheckBox * chkPan;
    QCheckBox * chkRot;
    QCheckBox * chkZoom;

 private:
    class ViewControl * view;
};

#endif
