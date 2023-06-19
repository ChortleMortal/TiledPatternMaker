#pragma once
#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

class QGridLayout;

#include <QFrame>

class ControlPanel;
class ViewControl;

class SplitScreen : public QFrame
{
public:
    SplitScreen(QWidget *parent = nullptr);

public slots:
    void slot_panelResized();

protected:
    void addWidgets();

private:
    QGridLayout  * grid;
    ControlPanel * panel;
    ViewControl  * view;
};

#endif // SPLITSCREEN_H
