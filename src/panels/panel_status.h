#pragma once
#ifndef PANEL_STATUS_H
#define PANEL_STATUS_H

#include <QStack>
#include <QLabel>

class PanelStatus : public QLabel
{
public:
    PanelStatus();

    void        pushStack(QString &txt);
    void        popStack();

private:
    QStack<QString>  msgStack;
    QString          preamble;
    QString          postamble;
};

#endif
