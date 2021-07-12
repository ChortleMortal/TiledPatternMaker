#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QtWidgets>
#include "enums/eviewtype.h"

class Configuration;

class PanelStatus : public QLabel
{
public:
    PanelStatus();

    void        setCurrentView(eViewType vtype);
    void        display(QString txt);
    void        hide();

protected:
    QString     getMsg(eViewType vtype);

private:
    QStack<QString>  msgStack;

    QString viewMsgs[VIEW_MAX+1];

    Configuration * config;
};

#endif
