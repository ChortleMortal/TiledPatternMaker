/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PANEL_H
#define PANEL_H

#include "panels/panel_page.h"
#include "panels/panel_list_widget.h"
#include "panels/panel_status.h"
#include "panels/panel_pagesWidget.h"
#include "base/configuration.h"
#include "base/tpmsplash.h"

class TiledPatternMaker;
class WorkspaceViewer;

class ControlPanel : public AQWidget
{
    Q_OBJECT

public:
    static ControlPanel * getInstance();
    static void releaseInstance();

    void    init(TiledPatternMaker * parent);

    void	closeEvent(QCloseEvent *event) override;
    void    resizeEvent(QResizeEvent *event) override;
    void    populatePages();
    void    floatPages();
    void    closePages();

    void    showPanelStatus(QString txt) { panelStatus->display(txt); }
    void    hidePanelStatus()            { panelStatus->hide(); }
    void    reflectCurrentView(eViewType vtype) { panelStatus->setCurrentView(vtype); }

    void    showSplash(QString txt)      { splash->display(txt); }
    void    hideSplash()                 { splash->remove(); }

    TiledPatternMaker * getMaker() { return maker; }
    panel_page * getCurrentPage()  { return currentPage; }

    void    selectViewer(int id);

    static eKbdMode getValidKbdMode(eKbdMode mode);

signals:
    void    sig_viewWS();
    void    sig_render();
    void    sig_panelResized();
    void    sig_reload();
    void    sig_view_synch(int id,  bool  enb);

public slots:
    void    slot_selectPanelPage(int index);
    void    slot_detachWidget(QString name);
    void    slot_attachWidget(QString name);
    void    slot_poll();

    void    slot_Viewer_pressed(int id, bool enb);
    void    slot_multiSelect(bool enb);

private slots:
    void    repeatChanged(int mode);
    void    slot_logEvent();
    void    slot_exit();

    void    slot_raise();
    void    updateClicked(bool enb);
    void    updateView(bool enb);
    void    slot_kbdModeChanged(int row);
    void    slot_kbdMode(eKbdMode mode);
    void    slot_scaleToView(bool enb);
    void    showTilingPressed();
    void    showMosaicPressed();
    void    slot_showBackChanged(bool state);
    void    slot_view_synch(int id, int enb);

protected:
    ControlPanel();
    ~ControlPanel() override;

    QGroupBox * createWorkspaceViewers();

    void    delegateView();
    void    delegateKeyboardMouse(eViewType viewType);


    static eKbdMode getValidMosaicMode(eKbdMode mode);
    static eKbdMode getValidDesignMode(eKbdMode mode);

private:
    void	setupGUI();
    void	refreshPage(panel_page * wp);

private:
    static ControlPanel * mpThis;

    Configuration           * config;
    TiledPatternMaker       * maker;
    WorkspaceViewer         * viewer;
    View                    * view;

    PanelStatus             * panelStatus;
    TPMSplash               * splash;

    QVector<panel_page*>    mAttachedPages;
    QVector<panel_page*>    mDetachedPages;
    QTimer *				mpTimer;
    QMutex					mUpdateMutex;

    bool                    closed;

    PanelListWidget      *  mpPanelPageList;
    PanelPagesWidget     *  panelPagesWidget;
    panel_page           *  currentPage;

    QCheckBox    * cbUpdate;
    QLabel       * statusLabel;
    QRadioButton * radioDefined;
    QRadioButton * radioPack;
    QRadioButton * radioSingle;

    QComboBox    * kbdModeCombo;

    QButtonGroup   repeatRadioGroup;

    ///
    QCheckBox   *cbLockView;
    QCheckBox   *cbRawDesignView;
    QCheckBox   *cbMosaicView;
    QCheckBox   *cbPrototypeView;
    QCheckBox   *cbTilingView;
    QCheckBox   *cbProtoMaker;
    QCheckBox   *cbDELView;
    QCheckBox   *cbTilingMakerView;
    QCheckBox   *cbFigMapView;
    QCheckBox   *cbFaceSetView;

    QButtonGroup  viewerGroup;
};

#endif
