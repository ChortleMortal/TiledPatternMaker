#pragma once
#ifndef PANELPAGECONTROLLER_H
#define PANELPAGECONTROLLER_H

#include "gui/panels/panel_page_list_widget.h"
#include "gui/panels/panel_pages_widget.h"

class ControlPanel;

class PanelPageController : public QObject
{
    Q_OBJECT

public:
    PanelPageController(PageListWidget * pageListWidget, PanelPagesWidget * PanelPages);
    virtual ~PanelPageController();

    void populatePages();
    void refreshPages();
    void closePages();
    bool isVisiblePage(ePanelPage page);
    bool isVisiblePage(panel_page * page);
    void reAttachPage(panel_page * page);

    void setCurrentPage(QString name);
    panel_page * getCurrentPage();
    panel_page * getPage(QString name);

public slots:
    void    slot_floatPages();
    void    slot_subAttachPage();
    void    slot_itemDetachPanelPage(QListWidgetItem * item);
    void    slot_selectPanelPage(QListWidgetItem * item);
    void    slot_detachWidget(QString name);

private:
    ControlPanel     * panel;
    Configuration    * config;

    PageListWidget   * pageList;
    PanelPagesWidget * pages;

    volatile bool      updateLocked;
};

#endif // PANELPAGECONTROLLER_H
