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

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    enum eLoadState
    {
        LOADING_NONE,
        LOADING_TILING,
        LOADING_MOSAIC
    };

    static ControlPanel * getInstance();
    static void releaseInstance();

    void    init(TiledPatternMaker * parent);

    void    floatPages()                     { pageController->floatPages(); }
    void    closePages()                     { pageController->closePages(); }
    bool    isVisiblePage(panel_page * page) { return pageController->isVisiblePage(page); }
    void    setCurrentPage(QString name)     { pageController->setCurrentPage(name); }
    panel_page* getCurrentPage()             { return pageController->getCurrentPage(); }
    void    completePageSelection();

    void    pushPanelStatus(QString & txt)   { panelStatus->pushStack(txt); }
    void    pushPanelStatus(QString && txt)  { panelStatus->pushStack(txt); }
    void    popPanelStatus()                 { panelStatus->popStack(); }

    void    selectViewer(eViewType vtype);
    void    delegateKeyboardMouse();

    MouseModeWidget  *  getMouseModeWidget() { return mouseModeWidget;}
    eKbdMode            getValidKbdMode(eKbdMode mode);
    void                setLoadState(eLoadState state, QString name=QString());

    QString gitBranch;  // (if there is one)

signals:
    void    sig_refreshView();
    void    sig_render();
    void    sig_panelResized();
    void    sig_reload();
    void    sig_saveLog();

public slots:
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
    ControlPanel();
    ~ControlPanel() override;

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
    void	setupGUI();

private:
    static ControlPanel * mpThis;

    class Configuration *  config;
    TiledPatternMaker   *  maker;
    class ViewControl   *  view;
    class MosaicMaker   *  mosaicMaker;
    class TilingMaker   *  tilingMaker;
    MouseModeWidget     *  mouseModeWidget;

    PanelViewSelect     *  viewSelect;
    PanelPageController *  pageController;

    bool                   isShown;
    PanelStatus         *  panelStatus;
    QTimer              *  mpTimer;

    QString                panelInfo;
    eLoadState             loadState;
    QString                loadName;

    QRadioButton        * radioDefined;
    QRadioButton        * radioPack;
    QRadioButton        * radioSingle;

    QComboBox           * kbdModeCombo;

    QButtonGroup          repeatRadioGroup;
};

#endif
