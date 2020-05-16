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

//#define BORDER

enum eSettingsGroup
{
    SDESIGN_SETTINGS  = 0,
    CANVAS_SETTINGS   = 1,
    VIEW_SETTINGS     = 0,
    SCENE_STATUS      = 2,
    VIEW_STATUS       = 1
};

public:
    page_canvasSettings(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void display();

    void pickBorderColor();
    void pickBorderColor2();
    void settingsSelectionChanged(int);
    void borderChanged(int row);

    void slot_loadBackground();
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();

    void slot_setBkgdXform();
    void slot_setBkgd();
    void pickBackgroundColor();

    void slot_setBkgdXformCanvas();
    void slot_setBkgdCanvas();
    void pickBackgroundColorCanvas();

    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

    void designSizeChanged(qreal);
    void canvasSizeChanged(qreal);


protected:
    QGroupBox * createDesignSettings();
    QGroupBox * createCanvasSettings();
    QGroupBox * createSceneStatus();

    QGroupBox * createViewSettings();
    QGroupBox * createViewStatus();

    void displaySettings(CanvasSettings & cSettings, eSettingsGroup group);
    void displayBkgdImgSettings(CanvasSettings & cSettings, eSettingsGroup group);

    void setFromForm();
    void setBorderFromForm();

private:
    void removeChildren(QTreeWidgetItem * parent);

    class TilingMaker * tilingMaker;

    QButtonGroup   bgroup;

    DoubleSpinSet * sizeEditW[3];
    DoubleSpinSet * sizeEditH[3];

    QLineEdit * bkColorEdit[3];
    ClickableLabel * bkgdColorPatch[3];

    DoubleSpinSet * startEditX[3];
    DoubleSpinSet * startEditY[3];

    SpinSet * viewW[2];
    SpinSet * viewH[2];


#ifdef BORDER
    QComboBox   borderType;

    QLineEdit * borderWidth;
    QLabel    * borderColorLabel[2];
    QLineEdit * borderColor[2];
    ClickableLabel * borderColorPatch[2];
#endif

    QPushButton   * bkgdImageBtn;
    QLineEdit     * bkgdImage[2];

    LayoutTransform * bkgdLayout[2];

    QCheckBox     * chk_showBkgd[2];
    QCheckBox     * chk_adjustBkgd[2];
    QCheckBox     * chk_xformBkgd[2];
};

#endif
