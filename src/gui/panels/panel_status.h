#pragma once
#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QLabel>

class PanelStatus : public QLabel
{
public:
    PanelStatus();

    void    setStatusText(QString & txt);
    void    clearStatusText();

private:
    QString preamble;
    QString postamble;
};

#endif
