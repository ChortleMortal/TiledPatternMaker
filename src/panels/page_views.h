/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kapla4n.      email: csk at cs.washington.edu
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

#ifndef PAGE_VIEWS_H
#define PAGE_VIEWS_H

#include "base/configuration.h"
#include "panels/layout_sliderset.h"
#include "panels/panel_page.h"

class page_views : public panel_page
{
    Q_OBJECT

public:
    page_views(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

signals:
    void    sig_regenerate();
    void    sig_loadTiling(QString);
    void    sig_mapEdSelection();

public slots:
    void    slot_loadedXML(QString name);
    void    slot_selectViewer(int id);

private slots:
    void    slot_Viewer_pressed(int id, bool enb);
    void    slot_lockViewClicked(bool enb);
    void    slot_multiSelect(bool enb);

protected:
    void  createViewControl();
    void  createWorkspaceViewers();
    void  refreshPanel();
    void  updateWsStatus();
    void  updateLoadedStatus();

private:
    QGroupBox   *workspaceViewersBox;
    QGroupBox   *workspaceMakersBox;
    QCheckBox   *cbLockView;
    QCheckBox   *cbRawDesignView;
    QCheckBox   *cbMosaicView;
    QCheckBox   *cbProtoView;
    QCheckBox   *cbProtoFeatureView;
    QCheckBox   *cbTilingView;
    QCheckBox   *cbFigureView;
    QCheckBox   *cbDELView;
    QCheckBox   *cbTilingMakerView;
    QCheckBox   *cbFigMapView;
    QCheckBox   *cbFaceSetView;

    QLabel  *  lab_activeDesigns;

    QLabel  *  lab_LoadedStyle;
    QLabel  *  lab_LoadedStylesTiling;
    QLabel  *  lab_LoadedStyleStyles;
    QLabel  *  lab_LoadedStylesProto;
    QLabel  *  lab_LoadedDEL;

    QButtonGroup  viewerGroup;

    QGridLayout * statusBox;
    QVBoxLayout * dummyStatusBox;
};

#endif
