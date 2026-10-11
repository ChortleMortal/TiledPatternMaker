#pragma once

#include <QWidget>

class panel_page;

class PanelPagesWidget : public QWidget
{
public:
    PanelPagesWidget();
    ~PanelPagesWidget();

    void setCurrentPage(panel_page * pp);
};
