#pragma once
#ifndef PANELLISTWIDGET_H
#define PANELLISTWIDGET_H

#include <QListWidget>
#include "gui/panels/panel_page.h"

class ControlPanel;

enum ePageState
{
    PAGE_ATTACHED,
    PAGE_SUB_ATTACHED,
    PAGE_DETACHED
};

class PageData
{
public:
    PageData(panel_page * ppage, QListWidgetItem * item);
    PageData(const PageData & other);

    PageData & operator=(const PageData & other);

    ePageState        pageState;
    panel_page      * page;
    QListWidgetItem * pageItem;
    QString           pageName;
    bool              selected;
};

class PageListWidget : public QListWidget
{
    Q_OBJECT

public:
    PageListWidget(ControlPanel *parent);

    void        refreshPages();
    void        closePages();
    void        deletePages();

    void        addPage(          panel_page * page);
    ePageState  getState(         panel_page * page);
    int         getIndex(         panel_page * page);
    bool        isVisiblyDetached(panel_page * page);
    bool        isVisiblyDetached(ePanelPage   page);

    panel_page* getPage(QString name);
    panel_page* getSelectedPage();

    void        setCurrentRow(QString name);

    void        setState(QString name, ePageState state);
    void        addSeparator();

    QStringList wereFloated();

    virtual void mousePressEvent(QMouseEvent * event) override;

    void        establishSize();

    PageData &  getPageData(QString name);
    PageData &  getPageData(panel_page * page);

signals:
    void        sig_detachWidget(QString name);

protected slots:
    void        slot_floatAction();

protected:
    void        refreshPage(panel_page * wp);

    QBrush      foregroundBrush;
    QBrush      backgroundBrush;

private:
    ControlPanel * controlPanel;

    QVector<PageData> pageData;

    int         separators;
    QString     pageToDetach;

    volatile bool localDrop;
};

#endif // PANELLISTWIDGET_H
