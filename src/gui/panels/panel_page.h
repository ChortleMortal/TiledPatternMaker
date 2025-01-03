#pragma once
#ifndef PANELPAGE_H
#define PANELPAGE_H

#include <QString>
#include <QtWidgets>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "sys/enums/epanelpage.h"

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
    virtual QString getPageStatus() { return QString(); }
    virtual bool    canExit()       { return true; }

    void            leaveEvent(QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    void            enterEvent(QEnterEvent *event) override;
#else
    void            enterEvent(QEvent *event) override;
#endif
    virtual void	closeEvent(QCloseEvent * event) override;

    QString    getName()                   { return pageName; }
    ePanelPage getPageType()             { return pageType; }

    void    setNewlySelected(bool state) { newlySelected = state; }
    bool    isNewlySelected()            { return newlySelected; }
    void    closePage(bool detached);
    void    floatMe();

    bool    wasFloated();

    QString addr(const void * address);
    QString addr(void * address);

    bool    pageBlocked();

signals:
    void    sig_render();
    void	sig_attachMe(panel_page * page);
    void    sig_reconstructView();
    void    sig_updateView();

protected:
    virtual void mouseEnter() { refresh = false; }
    virtual void mouseLeave() { refresh = true; }

    void    blockPage(bool block);

    QVBoxLayout             * vbox;
    QString                   pageName;
    ePanelPage                pageType;

    class ControlPanel      * panel;
    class Configuration     * config;
    class ViewController    * viewControl;
    class TilingMaker       * tilingMaker;
    class PrototypeMaker    * prototypeMaker;
    class MosaicMaker       * mosaicMaker;
    class DesignMaker       * designMaker;

    bool                      refresh;

private:
    bool                      closed;
    bool                      floating;
    bool                      newlySelected;
    int                       blockCount;
};

#endif
