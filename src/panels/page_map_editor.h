﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef PAGE_FIGURE_EDITOR_H
#define PAGE_FIGURE_EDITOR_H

#include "panels/panel_page.h"
typedef std::shared_ptr<class MapEditor>              MapEditorPtr;

class page_map_editor : public panel_page
{
    Q_OBJECT

public:
    page_map_editor(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override;

public slots:
    void slot_reload();
    void slot_selected_dele_changed();
    void slot_mosaicLoaded(QString name);
    void slot_tilingLoaded (QString name);
    void slot_setModes(int mode, bool checked);
    void slot_mapEdMode_pressed(int id, bool checked);

private slots:
    void slot_convertToExplicit();
    void slot_verify();
    void slot_divideIntersectingEdges();
    void slot_joinColinearEdges();
    void slot_cleanNeighbours();
    void slot_rebuildNeighbours();
    void slot_sortVertices();
    void slot_sortEdges();
    void slot_removeUnconnectedVertices();
    void slot_removeSingleConnectVertices();
    void slot_removeZombieEdges();
    void slot_popstash();
    void slot_undoConstructionLines();
    void slot_redoConstructionLines();
    void slot_clearConstructionLines();
    void slot_clearMap();
    void slot_createMap();
    void slot_loadMap();
    void slot_saveMap();
    void slot_pushToTiling();
    void slot_dumpMap();
    void slot_applyCrop();
    void slot_applyMask();
    void slot_cleanseMap();
    void slot_createDCEL();
    void slot_saveTemplate();
    void slot_loadTemplate();
    void slot_hideCons(bool hide);
    void slot_hideMap(bool hide);
    void slot_hidePoints(bool hide);
    void slot_hideMidPoints(bool hide);
    void slot_debugChk(bool on);

    void slot_radiusChanged(qreal r);
    void slot_lineWidthChanged(qreal r);
    void slot_consWidthChanged(qreal r);

private:
    MapEditorPtr    maped;

    QGroupBox     * editorStatusBox;
    QTextEdit       statusBox;
    QButtonGroup    mapEdModeGroup;
    QButtonGroup    modeGroup;
    QCheckBox     * animateChk;
    QString         lastNamedTemplate;

    QToolButton * pbCropMap;
    QToolButton * pbMaskMap;
    QPalette pal;
    QPalette palPink;
    QPalette palYellow;
};

#endif
