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

#ifndef PAGE_BORDERS_H
#define PAGE_BORDERS_H

#include "panels/panel_page.h"
#include "base/shared.h"
#include "panels/panel_misc.h"
#include "panels/layout_qrectf.h"

class page_borders : public panel_page
{
    Q_OBJECT

#define NUM_BORDER_COLORS 2

public:
    page_borders(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

public slots:
    void slot_viewUpated();

private slots:
    void display();

    void pickBorderColor();
    void pickBorderColor2();
    void borderWidthChanged(int width);
    void borderRowsChanged(int rows);
    void borderColsChanged(int cols);
    void borderChanged(int row);

    void slot_defineBorder();
    void slot_editBorder();
    void slot_innerBoundryChanged();
    void slot_outerBoundryChanged();

    void slot_cropAspect(int id);
    void slot_verticalAspect(bool checked);

protected:
    QWidget     * createOuterBorderWidget();
    QWidget     * createInnerBorderWidget();
    QHBoxLayout * createAspectLayout();

    void displayBorder(BorderPtr bp);

    BorderPtr getMosaicOrDesignBorder();

private:
    QComboBox         borderType;

    QGroupBox       * borderbox;
    QStackedLayout  * stack;

    QWidget         * innerBorderWidget;
    QWidget         * outerBorderWidget;
    QWidget         * nullBorderWidget;

    SpinSet         * borderWidth;
    SpinSet         * borderRows;
    SpinSet         * borderCols;
    QLabel          * borderColorLabel[NUM_BORDER_COLORS];
    QLineEdit       * borderColor[NUM_BORDER_COLORS];
    ClickableLabel  * borderColorPatch[NUM_BORDER_COLORS];

    LayoutQRectF    * innerBoundaryLayout;
    LayoutQRectF    * outerBoundaryLayout;

    QButtonGroup      aspects;
    QCheckBox      *  chkVert;
};

#endif
