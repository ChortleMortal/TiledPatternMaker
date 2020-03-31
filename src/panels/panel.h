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

#include <QtGui>
#include <QtWidgets>

#include "panels/panel_page.h"
#include "panels/panel_misc.h"
#include "panels/panel_pagesWidget.h"
#include "base/configuration.h"

class TiledPatternMaker;
class WorkspaceViewer;

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    ControlPanel(TiledPatternMaker * parent);
    ~ControlPanel() override;

    void	closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void    resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void    populatePages();
    void    floatPages();
    void    closePages();

    QLabel  * getStatus()          { return status; }
    TiledPatternMaker * getMaker() { return maker; }
    panel_page * getCurrentPage()  { return currentPage; }

signals:
    void    sig_viewWS();
    void    sig_render(eRender type);
    void    sig_selectViewer(int id, int id2);
    void    sig_panelResized();
    void    sig_reload();

public slots:
    void    slot_selectWidget(int index);
    void    slot_detachWidget(QString name);
    void    slot_attachWidget(QString name);
    void    slot_poll();

private slots:
    void    repeatChanged(int mode);
    void    slot_logEvent();
    void    slot_exit();
    void    autoClearClicked(bool enb);
    void    autoLoadStylesClicked(bool enb);
    void    autoLoadTilingClicked(bool enb);
    void    autoLoadDesignsClicked(bool enb);
    void    slot_raise();
    void    updateClicked(bool enb);
    void    slot_xformModeChanged(int row);
    void    slot_kbdMode(eKbdMode mode);

protected:
    void    delegateView();

private:
    void	setupGUI();
    void	setupTools(QToolBar *tools);
    void	refreshPage(panel_page * wp);

private:
    Configuration           * config;
    Canvas                  * canvas;
    TiledPatternMaker       * maker;

    QVector<panel_page*>    mAttachedPages;
    QVector<panel_page*>    mDetachedPages;
    QTimer *				mpTimer;
    QMutex					mUpdateMutex;

    PanelListWidget      *  mpPanelPageList;
    PanelPagesWidget     *  panelPagesWidget;
    panel_page           *  currentPage;
    QLabel               *  status;

    QCheckBox    * cbUpdate;
    QLabel       * statusLabel;
    QRadioButton * radioDefined;
    QRadioButton * radioPack;
    QRadioButton * radioSingle;
    QCheckBox    * cbAutoClear;
    QCheckBox    * cbAutoLoadStyles;
    QCheckBox    * cbAutoLoadTiling;
    QCheckBox    * cbAutoLoadDesigns;
    QComboBox    * xformModeCombo;

    QButtonGroup   repeatRadioGroup;
};

#endif
