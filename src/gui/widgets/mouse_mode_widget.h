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

    void    setTranslateMode(bool checked);
    void    setRotateMode(bool checked);
    void    setScaleMode(bool checked);

    void display();

protected:
    QCheckBox * chkPan;
    QCheckBox * chkRot;
    QCheckBox * chkZoom;

 private:
};

#endif
