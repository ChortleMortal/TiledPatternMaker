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

public:
    page_canvasSettings(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;

signals:

private slots:
    void display();

    void setInfo();

    void pickBorderColor();
    void pickBorderColor2();
    void pickBackgroundColor();
    void settingsSelectionChanged(int);
    void borderChanged(int row);

    void slot_loadBackground();
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
    void slot_setBkgdXform();
    void slot_setBkgd();

    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

protected:
    void displayBackgroundStatus();
    void setFromForm();
    void setBorderFromForm();

private:
    void removeChildren(QTreeWidgetItem * parent);

    class TilingMaker * tilingMaker;

    CanvasSettings  cSettings;      // local copy

    QButtonGroup   bgroup;

    ClickableLabel * bkgdColorPatch;
    QLineEdit * sizeEditW;
    QLineEdit * sizeEditH;
    QLineEdit * startEditX;
    QLineEdit * startEditY;
    QLineEdit * bkColorEdit;
    QLineEdit * imageEdit;

    QComboBox   borderType;

    QLineEdit * borderWidth;
    QLabel    * borderColorLabel[2];
    QLineEdit * borderColor[2];
    ClickableLabel * borderColorPatch[2];

    QLabel  * line1;
    QLabel  * line2;

    LayoutTransform bkgdLayout;

    QCheckBox     * chk_showBkgd;
    QCheckBox     * chk_adjustBkgd;
    QCheckBox     * chk_xformBkgd;
};

#endif
