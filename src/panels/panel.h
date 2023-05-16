#pragma once
#ifndef PANEL_H
#define PANEL_H

#include <QButtonGroup>

#include "widgets/panel_misc.h"
#include "widgets/panel_list_widget.h"
#include "widgets/panel_status.h"
#include "misc/tpmsplash.h"
#include "enums/ekeyboardmode.h"
#include "enums/eviewtype.h"

class QCheckBox;
class QRadioButton;
class QGroupBox;
class panel_page;
class PanelPagesWidget;
class TiledPatternMaker;
class MouseModeWidget;

typedef std::shared_ptr<class TilingMaker> TilingMakerPtr;

class ControlPanel : public AQWidget
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

    void	closeEvent(QCloseEvent *event) override;
    void    resizeEvent(QResizeEvent *event) override;
    void    populatePages();
    void    floatPages();
    void    closePages();

    void    pushPanelStatus(QString & txt) { panelStatus->pushStack(txt); }
    void    pushPanelStatus(QString && txt){ panelStatus->pushStack(txt); }
    void    popPanelStatus()               { panelStatus->popStack(); }

    void    splashMosiac(QString & txt)  { splash->displayMosaic(txt); }
    void    splashMosiac(QString && txt) { splash->displayMosaic(txt); }
    void    splashTiling(QString & txt)  { splash->displayTiling(txt); }
    void    splashTiling(QString && txt) { splash->displayTiling(txt); }
    void    removeSplashMosaic()         { splash->removeMosaic(); }
    void    removeSplashTiling()         { splash->removeTiling(); }

    void    selectViewer(int id);

    void                setCurrentPage(QString name);
    panel_page *        getCurrentPage();
    MouseModeWidget  *  getMouseModeWidget() { return mouseModeWidget;}
    bool                isVisiblePage(panel_page *);
    static eKbdMode     getValidKbdMode(eKbdMode mode);
    void                setLoadState(eLoadState state, QString name=QString());

    QString gitBranch;  // (if there is one)

signals:
    void    sig_refreshView();
    void    sig_render();
    void    sig_panelResized();
    void    sig_reload();
    void    sig_saveLog();

public slots:
    void    slot_itemPanelPage(QListWidgetItem * item);
    void    slot_selectPanelPage(int index);
    void    slot_detachWidget(QString name);
    void    slot_attachWidget(QString name);
    void    slot_poll();
    void    slot_Viewer_pressed(int id, bool enable);
    void    slot_lockStatusChanged();

private slots:
    void    repeatChanged(int mode);
    void    slot_logEvent();
    void    slot_exit();
    void    slot_raise();
    void    updateView(bool enb);
    void    slot_kbdModeChanged(int row);
    void    slot_kbdMode(eKbdMode mode);
    void    slot_scaleToView(bool enb);
    void    showTilingPressed();
    void    showMosaicPressed();
    void    slot_showBackChanged(bool state);
    void    slot_showGridChanged(bool state);
    void    slot_showMeasureChanged(bool state);
    void    slot_showCenterChanged(bool state);
    void    slot_lockViewClicked(bool enb);
    void    slot_multiSelect(bool enb);

protected:
    ControlPanel();
    ~ControlPanel() override;

#if 0
    void moveEvent(QMoveEvent *event) override;
#endif

    QGroupBox * createViewersBox();

    void    delegateView();
    void    delegateKeyboardMouse(eViewType viewType);

    static eKbdMode getValidMosaicMode(eKbdMode mode);
    static eKbdMode getValidDesignMode(eKbdMode mode);

    void    getPanelInfo();
    void    setWindowTitles();

private:
    void	setupGUI();
    void	refreshPage(panel_page * wp);

private:
    static ControlPanel * mpThis;

    class Configuration  *  config;
    TiledPatternMaker    *  maker;
    class ViewControl    *  view;
    class MosaicMaker    * mosaicMaker;
    TilingMakerPtr         tilingMaker;
    MouseModeWidget      *  mouseModeWidget;

    PanelStatus          *  panelStatus;
    TPMSplash            *  splash;
    QTimer               *  mpTimer;
    volatile bool           updateLocked;

    QString                 panelInfo;
    bool                    closed;
    bool                    exclusiveViews;
    eLoadState              loadState;
    QString                 loadName;

    QVector<panel_page*>    mPages;
    PanelListWidget      *  panelPageList;
    PanelPagesWidget     *  panelPages;

    QRadioButton * radioDefined;
    QRadioButton * radioPack;
    QRadioButton * radioSingle;

    QComboBox    * kbdModeCombo;

    QButtonGroup   repeatRadioGroup;

    QCheckBox   *cbRawDesignView;
    QCheckBox   *cbMosaicView;
    QCheckBox   *cbPrototypeView;
    QCheckBox   *cbTilingView;
    QCheckBox   *cbProtoMaker;
    QCheckBox   *cbTilingMakerView;
    QCheckBox   *cbMapEditor;
    QCheckBox   *cbBackgroundImage;
    QCheckBox   *cbGrid;
    QCheckBox   *cbMeasure;
    QCheckBox   *cbCenter;

    QCheckBox   *cbLockView;
    QCheckBox   *cbMultiSelect;

    QButtonGroup  viewerGroup;
};

#endif
