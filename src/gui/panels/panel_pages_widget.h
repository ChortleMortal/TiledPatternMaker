#pragma once
#ifndef PANEL_PAGES_WIDGET_H
#define PANEL_PAGES_WIDGET_H

#include <QWidget>

class panel_page;

class PanelPagesWidget : public QWidget
{
public:
    PanelPagesWidget();
    ~PanelPagesWidget();

    void setCurrentPage(panel_page * pp);
};

#endif
