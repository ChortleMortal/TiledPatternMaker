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

    void addWidget(panel_page * page);

    panel_page * setCurrentPage(QString name);
    void         setCurrentPage(panel_page * pp);
    panel_page * getCurrentPage() {  return  currentPage; }

private:

    QMap<QString,panel_page *> pages;
    panel_page * currentPage;
};

#endif
