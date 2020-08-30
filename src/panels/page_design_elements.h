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

#ifndef PAGE_DELS_H
#define PAGE_DELS_H

#include "panels/panel_page.h"

class page_design_elements : public panel_page
{
    Q_OBJECT

    enum eDelCol
    {
        DEL_COL_DEL        = 0,
        DEL_COL_FEATURE    = 1,
        DEL_COL_FIGURE     = 2,
        DEL_COL_DESC       = 3,
        DEL_COL_FIG_TYPE   = 4
    };

public:
    page_design_elements(ControlPanel * cpanel);

    void    refreshPage() override;
    void    onEnter() override;
    void    onExit() override {}

public slots:
    void    slot_loadedXML(QString name);
    void    slot_loadedTiling (QString name);
    void    slot_loadedDesign(eDesign design);

private slots:
    void    slot_rowSelected(int row, int col);
    void    slot_prototypeSelected(int);

protected:

private:
    AQTableWidget * delTable; // design element table

    QComboBox     * protoListBox;
};

#endif
