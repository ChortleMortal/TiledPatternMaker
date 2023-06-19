#pragma once
#ifndef PANELPAGE_H
#define PANELPAGE_H

#include <memory>
#include <QString>
#include <QtWidgets>
#include "widgets/panel_misc.h"

//class QVBoxLayout;

extern class TiledPatternMaker * theApp;

class panel_page : public QWidget
{
    Q_OBJECT

public:
    panel_page(class ControlPanel * panel, QString name);

    virtual void	onRefresh() = 0;
    virtual void    onEnter()     = 0;
    virtual void    onExit()      = 0;
    virtual bool    canExit() { return true; }

    void            leaveEvent(QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    void            enterEvent(QEnterEvent *event) override;
#else
    void            enterEvent(QEvent *event) override;
#endif
    virtual void	closeEvent(QCloseEvent * event) override;

    QString getName()                    { return pageName; }
    void    setNewlySelected(bool state) { newlySelected = state; }
    bool    isNewlySelected()            { return newlySelected; }
    bool    isFloated()                  { return floated;}
    void    setFloated(bool isFloated)   { floated = isFloated; }
    void    closePage();
    void    floatMe();
    bool    wasFloated();

    QString addr(const void * address);
    QString addr(void * address);

    bool    pageBlocked();

    void    updateView();

signals:
    void    sig_render();
    void	sig_attachMe(QString title);
    void    sig_refreshView();

protected:
    virtual void    mouseEnter() { refresh = false; }
    virtual void    mouseLeave() { refresh = true; }

    void    blockPage(bool block);

    QVBoxLayout             * vbox;
    QString                   pageName;

    class ControlPanel      * panel;
    class Configuration     * config;
    class ViewControl       * view;
    class TilingMaker       * tilingMaker;
    class PrototypeMaker    * prototypeMaker;
    class MosaicMaker       * mosaicMaker;
    class DesignMaker       * designMaker;

    bool                      refresh;

private:
    bool                      newlySelected;
    bool                      floated;
    int                       blockCount;
};

#endif
