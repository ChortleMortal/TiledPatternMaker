#pragma once
#ifndef PANELPAGE_H
#define PANELPAGE_H

#include <QString>
#include <QtWidgets>

#include "sys/enums/epanelpage.h"
#include "gui/panels/panel_page_list_widget.h"

#if defined(Q_OS_WINDOWS)
#define PANEL_RHS_WIDTH 740
#else
#define PANEL_RHS_WIDTH 760
#endif

extern class TiledPatternMaker * theApp;

class panel_page : public QWidget
{
    Q_OBJECT

public:
    panel_page(class ControlPanel *parent, ePanelPage page, QString name);

    virtual void	onRefresh()     = 0;
    virtual void    onEnter()       = 0;
    virtual void    onExit()        = 0;
    virtual bool    canExit()       { return true; }

    void            leaveEvent(QEvent *event) override;
    void            enterEvent(QEnterEvent *event) override;
    virtual void	closeEvent(QCloseEvent * event) override;

    QString         getName()                   { return pageName; }
    ePanelPage      getPageType()               { return pageType; }
    ePageState      getState()                  { return pageState; }
    QString &       getPageStatus()             { return pageStatusString; }

    void            setNewlySelected(bool state){ newlySelected = state; }
    void            setState(ePageState ps)     { pageState = ps; }
    bool            isNewlySelected()           { return newlySelected; }
    void            closePage();

    void            setPageStatus();
    void            clearPageStatus();

    void            detach(QString tname);
    bool            isFloating()                { return floating; }
    bool            wasFloated();
    bool            wasSubAttached();

    QString         addr(const void * address);
    QString         addr(void * address);

    bool            pageBlocked();

signals:
    void            sig_attachMe(panel_page * page);
    void            sig_reconstructView();
    void            sig_updateView();

public slots:
    void            slot_raiseDetached();

protected:
    virtual void    mouseEnter() { refresh = false; }
    virtual void    mouseLeave() { refresh = true; }

    void            blockPage(bool block);

    QVBoxLayout             * vbox;

    class ControlPanel      * panel;
    class Configuration     * config;
    class TilingMaker       * tilingMaker;
    class PrototypeMaker    * prototypeMaker;
    class MosaicMaker       * mosaicMaker;
    class DesignMaker       * designMaker;
    class SystemViewController  * viewControl;

    ePanelPage                pageType;
    bool                      refresh;
    QString                   pageName;
    QString                   pageStatusString;

private:
    bool                      closed;
    bool                      floating;
    ePageState                pageState;
    bool                      newlySelected;
    int                       blockCount;
};

#endif
