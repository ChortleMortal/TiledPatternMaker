#pragma once
#ifndef PANEL_H
#define PANEL_H

#include <QButtonGroup>
#include <QtWidgets>

#include "panels/panel_page_controller.h"
#include "panels/panel_status.h"
#include "enums/ekeyboardmode.h"
#include "enums/eviewtype.h"

class panel_page;
class PanelViewSelect;
class TiledPatternMaker;
class MouseModeWidget;
class ViewController;
class View;
class MosaicMaker;
class TilingMaker;

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    ControlPanel();
    ~ControlPanel() override;
    void    init();

    void    floatPages()                     { pageController->floatPages(); }
    void    closePages()                     { pageController->closePages(); }
    bool    isVisiblePage(panel_page * page) { return pageController->isVisiblePage(page); }
    bool    isVisiblePage(ePanelPage page)   { return pageController->isVisiblePage(page); }
    void    setCurrentPage(QString name)     { pageController->setCurrentPage(name); }
    panel_page* getCurrentPage()             { return pageController->getCurrentPage(); }
    void    completePageSelection();

    void    setStatus(QString & txt)         { panelStatus->setStatusText(txt); }
    void    setStatus(QString && txt)        { panelStatus->setStatusText(txt); }
    void    clearStatus()                    { panelStatus->clearStatusText(); }
    QString getStatus()                      { return panelStatus->text(); }

    void    delegateView(eViewType vtype);
    void    delegateKeyboardMouse();

    MouseModeWidget  *  getMouseModeWidget() { return mouseModeWidget;}
    eKbdMode            getValidKbdMode(eKbdMode mode);

signals:
    void    sig_refreshView();
    void    sig_render();
    void    sig_panelResized();
    void    sig_reload();
    void    sig_saveLog();
    void    sig_id_layers();    // for debug

public slots:
    void    slot_reload();
    void    slot_poll();
    void    slot_reattachPage(panel_page * page)  { pageController->reAttachPage(page); }

private slots:
    void    repeatChanged(int mode);
    void    slot_logEvent();
    void    slot_exit();
    void    slot_raise();
    void    updateView();
    void    slot_kbdModeChanged(int row);
    void    slot_kbdMode(eKbdMode mode);
    void    slot_scaleToView(bool enb);
    void    showTilingPressed();
    void    showMosaicPressed();

protected:

    void    paintEvent(QPaintEvent *event) override;
    void	closeEvent(QCloseEvent *event) override;
    void    resizeEvent(QResizeEvent *event) override;
#if 0
    void moveEvent(QMoveEvent *event) override;
#endif

    void    delegateView();

    static eKbdMode getValidMosaicMode(eKbdMode mode);
    static eKbdMode getValidDesignMode(eKbdMode mode);

    void    getPanelInfo();
    void    setWindowTitles();

private:
    void        setupGUI();
    QGroupBox * createTopGroup();

private:
    class Configuration *  config;
    View                *  view;
    ViewController      *  viewControl;
    MosaicMaker         *  mosaicMaker;
    TilingMaker         *  tilingMaker;
    MouseModeWidget     *  mouseModeWidget;

    PanelViewSelect     *  viewSelect;
    PanelPageController *  pageController;

    bool                   isShown;
    PanelStatus         *  panelStatus;
    QTimer              *  mpTimer;

    QString                panelInfo;

    QRadioButton        * radioDefined;
    QRadioButton        * radioPack;
    QRadioButton        * radioSingle;

    QComboBox           * kbdModeCombo;

    QButtonGroup          repeatRadioGroup;
};

#endif
