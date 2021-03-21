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

#ifndef PAGE_DECORATION_MAKER_H
#define PAGE_DECORATION_MAKER_H

#include "panels/panel_page.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "panels/layout_sliderset.h"

class page_decoration_maker : public panel_page
{
    Q_OBJECT

    enum eStyleCol
    {
        STYLE_COL_CHECK_SHOW,
        STYLE_COL_TILING,
        STYLE_COL_STYLE,
        STYLE_COL_ADDR,
        STYLE_COL_STYLE_DATA = STYLE_COL_ADDR,
        STYLE_COL_TRANS,
        STYLE_COL_NUM_COLS
    };

public:
    page_decoration_maker(ControlPanel * apanel);

    void    onEnter() override;
    void    onExit() override {}
    void    refreshPage() override;

private slots:
    void    slot_styleSelected(const QItemSelection &selected, const QItemSelection &deselected);
    void    slot_deleteStyle();
    void    slot_moveStyleUp();
    void    slot_moveStyleDown();
    void    slot_duplicateStyle();
    void    slot_analyzeStyleMap();
    void    slot_set_reps();

protected:
    void     styleChanged(int row);
    void     tilingChanged(int row);
    void     styleVisibilityChanged(int row);

    void     reEnter();
    void     displayStyleParams();
    StylePtr getStyleRow(int row);
    StylePtr getStyleIndex(int index);
    StylePtr copyStyle(const StylePtr style);

    QHBoxLayout *createFillDataRow();

private:
    DecorationMaker   * decorationMaker;

    QItemSelectionModel * selectModel;

    AQTableWidget * styleTable;
    AQTableWidget * parmsTable;
    QVBoxLayout   * parmsLayout;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;
    QPushButton * analyzeBtn;

    SpinSet    * xRepMin;
    SpinSet    * xRepMax;
    SpinSet    * yRepMin;
    SpinSet    * yRepMax;
};

#endif
