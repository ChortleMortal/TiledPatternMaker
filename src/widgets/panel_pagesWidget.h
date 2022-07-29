#ifndef PANEL_PAGES_WIDGET_H
#define PANEL_PAGES_WIDGET_H

#include "widgets/panel_misc.h"

class panel_page;

class PanelPagesWidget : public AQWidget
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
