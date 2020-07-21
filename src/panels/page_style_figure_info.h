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

#ifndef PAGE_FIGURES_H
#define PAGE_FIGURES_H

#include "panel_page.h"
#include "base/shared.h"

Q_DECLARE_METATYPE(WeakTilingPtr)

class page_style_figure_info : public panel_page
{
    enum eCols
    {
        COL_STYLE_NAME,
        COL_FIGURE_TYPE
    };

public:
    page_style_figure_info(ControlPanel * panel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

protected:
    void showFiguresFromStyles();
    void showFiguresFromTiling();
    void showFiguresFromMaker();

private:
    AQTableWidget * figureTable;
};

#endif
