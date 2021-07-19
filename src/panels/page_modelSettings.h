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

#ifndef PAGE_MODEL_SETTINGS_H
#define PAGE_MODEL_SETTINGS_H

#include "panels/panel_page.h"
#include "settings/model_settings.h"

class AQSpinBox;
class SpinSet;
class DoubleSpinSet;

class ClickableLabel;
class AQTableWidget;

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

private slots:
    void slot_set_repsDesign(int val);
    void slot_set_repsTiling(int val);

    void designSizeChanged(int);
    void cropSizeChanged(int);
    void viewSizeChanged(int);

    void backgroundColorDesignPick();
    void backgroundColorDesignChanged(const QString & str);

    void slot_tilingSizeChanged(int val);
    void slot_showFrameInfoChanged(bool checked);
    void slot_boundsChanged();

protected:
    QGroupBox * createTilingSettings();
    QGroupBox * createDesignSettings();
    QGroupBox * createFrameSettings();
    QGroupBox * createViewStatus();

    QGridLayout * createFillDataRow(eSettingsGroup group);

    ModelSettings & getMosaicOrDesignSettings();

private:
    void removeChildren(QTreeWidgetItem * parent);

    AQSpinBox       * xRepMin[NUM_SETTINGS];
    AQSpinBox       * xRepMax[NUM_SETTINGS];
    AQSpinBox       * yRepMin[NUM_SETTINGS];
    AQSpinBox       * yRepMax[NUM_SETTINGS];

    SpinSet         * sizeW[NUM_SETTINGS];
    SpinSet         * sizeH[NUM_SETTINGS];

    SpinSet         * sizeW2;
    SpinSet         * sizeH2;

    DoubleSpinSet   * ds_left;
    DoubleSpinSet   * ds_top;
    DoubleSpinSet   * ds_width;

    QLabel          * l_xform;

    QLineEdit       * bkColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];

    DoubleSpinSet   * startEditX[NUM_SETTINGS];
    DoubleSpinSet   * startEditY[NUM_SETTINGS];

    QPushButton     * bkgdImageBtn[NUM_SETTINGS];
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];

    QGroupBox       * viewBox;
    QGroupBox       * frameBox;

    AQTableWidget   * frameTable;
};

#endif
