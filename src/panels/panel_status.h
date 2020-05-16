#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QtWidgets>
#include "panels/panel_misc.h"

class PanelStatus : public AQLabel
{
public:
    PanelStatus();

    void display(QString txt);
    void hide();

protected:

private:
    QStack<QString>  msgStack;
    QColor color;
};

#endif
