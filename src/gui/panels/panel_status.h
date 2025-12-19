#pragma once
#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QLabel>

#include "sys/enums/epanelpage.h"

class PanelStatus : public QLabel
{
public:
    PanelStatus();

    void    overridePanelStatus(const QString & txt);
    void    restorePanelStatus();

    void    setPageStatus(ePanelPage pp, const QString & str);
    void    clearPageStatus(ePanelPage pp);

    void    setBacgroundColor(QColor color);

private:
    QString preamble;
    QString postamble;
    QString pageStatus;
    int     page;
};

#endif
