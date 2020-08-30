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

#ifndef PAGE_STYLE_MAKER_H
#define PAGE_STYLE_MAKER_H

#include "panels/panel_page.h"
#include "makers/style_maker/style_editors.h"

class page_style_maker : public panel_page
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
    page_style_maker(ControlPanel * apanel);

    void onEnter() override;
    void onExit() override {}
    void refreshPage() override;

public slots:
    void     slot_loadedXML(QString name);
    void     slot_loadedTiling (QString name);

private slots:
    void    slot_unload();
    void    slot_styleSelected();
    void    slot_styleChanged(int row);
    void    slot_tilingChanged(int row);
    void    slot_styleVisibilityChanged(int row);
    void    slot_deleteStyle();
    void    slot_moveStyleUp();
    void    slot_moveStyleDown();
    void    slot_duplicateStyle();
    void    slot_analyzeStyleMap();

protected:
    void     reEnter();
    void     setupStyleParms(StylePtr style, AQTableWidget *table);
    StylePtr getStyleRow(int row);
    StylePtr getStyleIndex(int index);
    StylePtr copyStyle(const StylePtr style);

    TilingPtr loadNewTiling(QString name);

private:
    QItemSelectionModel * selectModel;

    StyleEditor   * styleParms;
    AQTableWidget * styleTable;
    AQTableWidget * parmsTable;
    QVBoxLayout   * parmsCtrl;

    QSignalMapper  styleMapper;
    QSignalMapper  tilingMapper;
    QSignalMapper  styleVisMapper;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;
    QPushButton * analyzeBtn;
};

#endif
