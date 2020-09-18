#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QtWidgets>
#include "panels/panel_misc.h"
#include "base/configuration.h"

struct viewMsg
{
    eViewType view;
    QString   msg;
};



class PanelStatus : public AQLabel
{
public:
    PanelStatus();

    void        setCurrentView(eViewType vtype);
    eViewType   getCurrentView() { return  viewType; }
    void        display(QString txt);
    void        hide();

protected:
    QString     getMsg();

private:
    QStack<QString>  msgStack;
    eViewType        viewType;

    static const viewMsg viewMsgs[NUM_VIEW_TYPES];
};

#endif
