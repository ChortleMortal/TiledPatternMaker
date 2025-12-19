#pragma once
#ifndef PANEL_H
#define PANEL_H

#include "gui/panels/panel_page_controller.h"
#include "gui/panels/panel_page.h"
#include "gui/panels/panel_status.h"
#include "sys/enums/eviewtype.h"
#include "sys/enums/epanelpage.h"

class AQPushButton;
class AQHBoxLayout;
class MosaicMaker;
class MouseModeWidget;
class PanelViewSelect;
class QButtonGroup;
class QRadioButton;
class SystemViewController;
class TiledPatternMaker;
class TilingMaker;
class panel_page;

class ControlPanel : public QWidget
{
    Q_OBJECT

    friend void panel_page::setPageStatus();
    friend void panel_page::clearPageStatus();

public:
    ControlPanel();
    ~ControlPanel() override;
    void    init();

    void    closePages()                        { pageController->closePages(); }
    bool    isVisiblePage(panel_page * page)    { return pageController->isVisiblePage(page); }
    bool    isVisiblePage(ePanelPage page)      { return pageController->isVisiblePage(page); }

    void        setCurrentPage(QString name)    { pageController->setCurrentPage(name); }
    panel_page* getCurrentPage()                { return pageController->getCurrentPage(); }
    panel_page* getPage(QString name)           { return pageController->getPage(name); }
    void        completePageSelection();

    void    overridePagelStatus(QString str);
    void    restorePageStatus();

    void    delegateView(eViewType vtype, bool delegate);
    void    deselectGangedViewers();

    MouseModeWidget     *  getMouseModeWidget() { return mouseModeWidget;}
    PanelPageController *  getPageController()  { return pageController; }

    void changeEvent(QEvent *event) override;

signals:
    void    sig_close();
    void    sig_reconstructView();
    void    sig_panelResized();
    void    sig_reload();
    void    sig_saveLog();
    void    sig_id_layers();    // for debug
    void    sig_raiseDetached();

public slots:
    void    slot_reload();
    void    slot_poll();
    void    slot_reattachPage(panel_page * page)    { pageController->reAttachPage(page); }
    void    slot_messageBox(QString txt, QString txt2);
    void    slot_raisePanel();
    void    slot_delegateView(eViewType vtype, bool delegate);

private slots:
    void    repeatChanged(int mode);
    void    slot_logEvent();
    void    slot_exit();
    void    slot_scaleToView(bool enb);
    void    showTilingPressed();
    void    showMosaicPressed();

    void    slot_dbgViewClicked(bool checked);
    void    slot_dbgFlagsClicked(bool checked);

protected:
    void    paintEvent(QPaintEvent *event) override;
    void	closeEvent(QCloseEvent *event) override;
    void    resizeEvent(QResizeEvent *event) override;
#if 0
    void moveEvent(QMoveEvent *event) override;
#endif
    void    setPageStatus(ePanelPage page, const QString & txt)     { if (panelStatus) panelStatus->setPageStatus(page,txt); }
    void    clearPageStatus(ePanelPage page)                        { if (panelStatus) panelStatus->clearPageStatus(page); }

    void    delegateViewByPage();

    void    getPanelInfo();
    void    setWindowTitles();

    QVBoxLayout * getDesignerTop();
    QVBoxLayout * getInsightTop();

private:
    void        setupGUI();
    QGroupBox * createTopGroup();

private:
    MouseModeWidget     * mouseModeWidget;

    PanelViewSelect     * viewSelect;
    PanelPageController * pageController;

    PanelStatus         * panelStatus;
    QTimer              * mpTimer;

    QString               panelInfo;

    QRadioButton        * radioDefined;
    QRadioButton        * radioPack;
    QRadioButton        * radioSingle;


    AQHBoxLayout        * rad_hbox;
    QButtonGroup          repeatRadioGroup;

    AQPushButton        * pbEnbDbgView;
    AQPushButton        * pbEnbDbgFlags;

    QPushButton * pbLogEvent;
    QPushButton * pbReload;
    QPushButton * pbRefreshView;
    QPushButton * pbUpdateView;
    QPushButton * pbSaveLog;
    QPushButton * pbShowMosaic;
    QPushButton * pbShowTiling;
    QPushButton * pbClearAll;
    QPushButton * pbRaise;
    QPushButton * pbExit;
    QCheckBox   * chkScaleToView;
};

#endif
