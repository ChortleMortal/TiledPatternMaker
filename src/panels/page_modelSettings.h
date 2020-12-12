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

#ifndef PAGE_MODEL_SETTINGS_H
#define PAGE_MODEL_SETTINGS_H

#include "panels/panel_page.h"
#include "panels/panel_misc.h"
#include "panels/layout_transform.h"
#include "base/model_settings.h"

class page_modelSettings : public panel_page
{
    Q_OBJECT

enum eSettingsGroup
{
    DESIGN_SETTINGS,    // propagates to CANVAS_SETTINGS
    TILING_SETTINGS,    // propagates to CANVAS_SETTINGS
    FRAME_SETTINGS,     // definitions per view type
    VIEW_STATUS,        // current status
    NUM_SETTINGS
};

#define NUM_BORDER_COLORS 2

public:
    page_modelSettings(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

public slots:
    void slot_viewUpated();

private slots:
    void display();

    void slot_matchDesign();
    void slot_matchTiling();
    void slot_set_repsDesign(int val);
    void slot_set_repsTiling(int val);

    void designSizeChanged(int);
    void frameSizeChanged(int);
    void viewSizeChanged(int);

    void backgroundColorDesignPick();
    void backgroundColorDesignChanged(const QString & str);

    void slot_setBkgdMosaicXform();
    void slot_setBkgdTilingXform();
    void slot_setBkgdMosaic();
    void slot_setBkgdTiling();

    void slot_loadMosaicBackground();
    void slot_loadTilingBackground();

    void slot_tilingSizeChanged(int val);

    void pickBorderColor();
    void pickBorderColor2();
    void borderWidthChanged(int width);
    void borderRowsChanged(int rows);
    void borderColsChanged(int cols);
    void borderChanged(int row);

    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
#if 0
    void slot_bkgd_moveX(int amount);
    void slot_bkgd_moveY(int amount);
    void slot_bkgd_rotate(int amount);
    void slot_bkgd_scale(int amount);
#endif
    void slot_showBkgdsChanged(bool checked);
    void slot_showBordersChanged(bool checked);
    void slot_showFsetChanged(bool checked);

protected:
    QGroupBox * createTilingSettings();
    QGroupBox * createDesignSettings();
    QGroupBox * createFrameSettings();
    QGroupBox * createDesignBorderBox();
    QGroupBox * createViewStatus();

    QGroupBox * createBackgroundImageGroup(eSettingsGroup group, QString title);
    QGridLayout * createFillDataRow(eSettingsGroup group);

    ModelSettingsPtr getMosaicOrDesignSettings();

    void displayBkgdImgSettings(BkgdImgPtr bi, eSettingsGroup group);
    void displayBorder(BorderPtr bp);

private:
    void removeChildren(QTreeWidgetItem * parent);

    QSpinBox        * xRepMin[NUM_SETTINGS];
    QSpinBox        * xRepMax[NUM_SETTINGS];
    QSpinBox        * yRepMin[NUM_SETTINGS];
    QSpinBox        * yRepMax[NUM_SETTINGS];

    SpinSet         * sizeW[NUM_SETTINGS];
    SpinSet         * sizeH[NUM_SETTINGS];

    SpinSet         * sizeW2;
    SpinSet         * sizeH2;
    QLabel          * frameXform;

    QLineEdit       * bkColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];

    DoubleSpinSet   * startEditX[NUM_SETTINGS];
    DoubleSpinSet   * startEditY[NUM_SETTINGS];

    QComboBox         borderType;
    SpinSet         * borderWidth;
    SpinSet         * borderRows;
    SpinSet         * borderCols;
    QLabel          * borderColorLabel[NUM_BORDER_COLORS];
    QLineEdit       * borderColor[NUM_BORDER_COLORS];
    ClickableLabel  * borderColorPatch[NUM_BORDER_COLORS];

    QPushButton     * bkgdImageBtn[NUM_SETTINGS];
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    LayoutTransform * bkgdLayout[NUM_SETTINGS];

    QCheckBox       * chk_showBkgd[NUM_SETTINGS];
    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];

    QGroupBox       * viewBox;
    QGroupBox       * mosaicBkgdBox;
    QGroupBox       * tilingBkgdBox;
    QGroupBox       * borderBox;

    AQTableWidget   * frameTable;
};

#endif