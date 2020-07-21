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

#ifndef PAGE_CANVAS_SETTINGS_H
#define PAGE_CANVAS_SETTINGS_H

#include "panel_page.h"
#include "panels/layout_transform.h"


class page_canvasSettings : public panel_page
{
    Q_OBJECT

enum eSettingsGroup
{
    DESIGN_SETTINGS   = 0,
    CANVAS_SETTINGS   = 1,
    TILING_SETTINGS   = 2,
    VIEW_SETTINGS     = 0,
    VIEW_STATUS       = 1
};

#define NUM_SETTINGS 3
#define NUM_BORDER_COLORS 2

public:
    page_canvasSettings(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void display();

    void pickBorderColor();
    void pickBorderColor2();
    void borderChanged(int row);

    void slot_loadBackground();
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();

    void slot_setBkgdXform();
    void slot_setBkgd();
    void pickDesignBackgroundColor();

    void slot_setBkgdXformCanvas();
    void slot_setBkgdCanvas();
    void pickBackgroundColorCanvas();

    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

    void slot_matchDesign();
    void slot_matchTiling();

    void designSizeChanged(int);
    void canvasSizeChanged(int);
    void viewSizeChanged(int);

    void designBkgdColorChanged(const QString & str);

protected:
    QGroupBox * createTilingSettings();
    QGroupBox * createDesignSettings();
    QGroupBox * createCanvasSettings();

    QGroupBox * createCanvasBorderSettings();
    QGroupBox * createDesignBorderSettings();

    QGroupBox * createViewSettings();
    QGroupBox * createViewStatus();

    CanvasSettings & getDesignSettings();

    void displaySettings(CanvasSettings & cSettings, eSettingsGroup group);
    void displayBkgdImgSettings(CanvasSettings & cSettings, eSettingsGroup group);
    void displayBorder(BorderPtr bp, eSettingsGroup group);

    void setFromForm();
    void setBorderFromForm();

private:
    void removeChildren(QTreeWidgetItem * parent);

    class TilingMaker * tilingMaker;

    SpinSet         * sizeEditW[NUM_SETTINGS];
    SpinSet         * sizeEditH[NUM_SETTINGS];

    QLineEdit       * bkColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];

    DoubleSpinSet   * startEditX[NUM_SETTINGS];
    DoubleSpinSet   * startEditY[NUM_SETTINGS];

    SpinSet         * viewW[NUM_SETTINGS];
    SpinSet         * viewH[NUM_SETTINGS];
    QLabel          * viewLabel;

    QComboBox         borderType[NUM_SETTINGS];
    QLineEdit       * borderWidth[NUM_SETTINGS];
    QLabel          * borderColorLabel[NUM_SETTINGS][NUM_BORDER_COLORS];
    QLineEdit       * borderColor[NUM_SETTINGS][NUM_BORDER_COLORS];
    ClickableLabel  * borderColorPatch[NUM_SETTINGS][NUM_BORDER_COLORS];

    QPushButton     * bkgdImageBtn;
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    LayoutTransform * bkgdLayout[NUM_SETTINGS];

    QCheckBox       * chk_showBkgd[NUM_SETTINGS];
    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];
    QCheckBox       * chk_xformBkgd[NUM_SETTINGS];

    QLabel          * labelViewTransform;
    QLabel          * labelTilingTransform;
};

#endif
