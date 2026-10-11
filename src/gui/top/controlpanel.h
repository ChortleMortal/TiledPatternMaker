#pragma once

#include <QMessageBox>
#include <QButtonGroup>
#include <QGroupBox>
#include "gui/panels/panel_page.h"
#include "gui/panels/panel_status.h"
#include "gui/viewers/layer.h"
#include "sys/enums/epanelpage.h"

constexpr int PANEL_WIDTH     = 841;
constexpr int PANEL_LHS_WIDTH = 117;
constexpr int PANEL_RHS_WIDTH = PANEL_WIDTH - PANEL_LHS_WIDTH;
constexpr int PANEL_RHS_HEIGHT = 682;

class AQPushButton;
class AQHBoxLayout;
class MosaicMaker;
class MouseModeWidget;
class PanelPageController;
class PanelViewSelect;
class QButtonGroup;
class QRadioButton;
class ViewController;
class TiledPatternMaker;
class TilingMaker;
class panel_page;
class Layer;

class ControlPanel : public QWidget
{
    Q_OBJECT

    friend void panel_page::setPageStatus();
    friend void panel_page::clearPageStatus();

public:
    ControlPanel();
    ~ControlPanel() override;
    void    init();
    void    bringToPrimaryScreen();

    void    closePages();
    bool    isVisiblePage(panel_page * page);
    bool    isVisiblePage(ePanelPage page);

    void        setCurrentPage(QString name);
    panel_page* getCurrentPage();
    panel_page* getPage(QString name);
    void        completePageSelection();

    void    overridePagelStatus(QString str);
    void    restorePageStatus();

    void    delegateView(Layer::Type vtype, bool delegate);
    void    userDelegateView(Layer::Type vtype, bool delegate);
    void    deselectGangedViewers();
    void    reEnableViewers();

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
    void    slot_reattachPage(panel_page * page);
    void    slot_messageBox(QString txt, QString txt2, QMessageBox::Icon icon, bool modal);
    void    slot_interlaceUnwoven();
    void    slot_raisePanel();
    void    slot_delegateView(Layer::Type vtype, bool delegate);

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
    QMessageBox         * unwovenBox;
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
