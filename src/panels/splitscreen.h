#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <QtWidgets>

class ControlPanel;
class View;

class SplitScreen : public QFrame
{
public:
    SplitScreen(QWidget *parent = nullptr);
    void addWidgets(ControlPanel * panel, View * view);

public slots:
    void slot_panelResized();

protected:

private:
    QGridLayout  * grid;
    ControlPanel * cp;
    View         * vw;
};

#endif // SPLITSCREEN_H
