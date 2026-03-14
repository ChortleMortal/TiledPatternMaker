#pragma once
#ifndef STYLED_EDITOR_H
#define STYLED_EDITOR_H

#include <QWidget>

#include "sys/enums/estyletype.h"

using std::shared_ptr;
using std::weak_ptr;

class AQTableWidget;

typedef shared_ptr<class Style>     StylePtr;
typedef weak_ptr<class Style>      wStylePtr;

class StyleEditor : public QWidget
{
    Q_OBJECT

public:
    StyleEditor(StylePtr style, eStyleType user);

    virtual void onEnter()   {};
    virtual void onExit()    {};
    virtual void onRefresh() {};

    void notify();

signals:
    void    sig_reconstructView();
    void    sig_updateView();

protected:
    wStylePtr       wStyle;
    eStyleType      user;
    AQTableWidget * setable;
};

#endif
