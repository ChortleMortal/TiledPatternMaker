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
#include "panels/style_editors.h"

class page_style_maker : public panel_page
{
    Q_OBJECT

    enum eStyleCol
    {
        STYLE_COL_CHECK_SHOW = 0,
        STYLE_COL_TILING     = 1,
        STYLE_COL_DATA       = STYLE_COL_TILING,
        STYLE_COL_STYLE      = 2,
        STYLE_COL_PROTO_EDIT = 3,
        STYLE_COL_ADDR       = 4
    };

public:
    page_style_maker(ControlPanel *panel);

    void onEnter() override;
    void refreshPage() override;

public slots:
    void     slot_loadedXML(QString name);
    void     slot_loadedTiling (QString name);

private slots:
    void    slot_unload();
    void    slot_styleSelected();
    void    slot_styleChanged(int row);
    void    slot_styleVisibilityChanged(int row);
    void    slot_editProto(int row);        // DAC should be slot_setWSProto
    void    slot_deleteStyle();
    void    slot_moveStyleUp();
    void    slot_moveStyleDown();
    void    slot_duplicateStyle();
    void    slot_analyzeStyleMap();

protected:
    void     reEnter();
    void     setupStyleParms(StylePtr style, QTableWidget * table);
    StylePtr getStyleRow(int row);
    StylePtr getStyleIndex(int index);
    StylePtr copyStyle(const StylePtr style);

private:
    QItemSelectionModel * selectModel;

    StyleEditor  * styleParms;
    QTableWidget * styleTable;
    QTableWidget * parmsTable;
    QVBoxLayout  * parmsCtrl;

    QSignalMapper  styleMapper;
    QSignalMapper  styleVisMapper;
    QSignalMapper  editProtoMapper;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;
    QPushButton * analyzeBtn;
};

#endif
