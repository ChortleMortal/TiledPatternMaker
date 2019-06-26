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

#ifndef PAGE_DESIGNS_H
#define PAGE_DESIGNS_H

#include "panel_page.h"

class page_designs : public panel_page
{
    Q_OBJECT

    enum eDesignCol
    {
        DESIGN_COL_VISIBILITY = 0,
        DESIGN_COL_INFO       = 1
    };

public:
    page_designs(ControlPanel * panel);

    void onEnter() override;
    void refreshPage() override;

private slots:
    void    slot_designVisibilityChanged(int row);
    void    slot_designTableCell(int row, int col);

protected:

private:
    QTableWidget * designTable;
    QSignalMapper  designVisMapper;
};

#endif
