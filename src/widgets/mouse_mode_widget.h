#pragma once
#ifndef MOUSEMODEWIDGET_H
#define MOUSEMODEWIDGET_H

#include <QWidget>

class QCheckBox;

class MouseModeWidget : public QWidget
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
    class View * view;
};

#endif
