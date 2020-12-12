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

#ifndef PAGE_PROTOS_H
#define PAGE_PROTOS_H

#include "panels/panel_page.h"
#include "panels/panel_misc.h"

enum eProtoCol
{
    PROTO_ROW_PROTO,
    PROTO_ROW_TILING,
    PROTO_ROW_DEL,
    PROTO_ROW_FEATURE,
    PROTO_ROW_FIGURE,
    PROTO_ROW_SCALE,
    PROTO_ROW_ROT,
    PROTO_ROW_X,
    PROTO_ROW_Y
};

class page_prototype_info : public panel_page
{
    Q_OBJECT

public:

    page_prototype_info(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

public slots:

private slots:
    void    slot_prototypeSelected(int row, int col);
    void    drawMapClicked(bool enb);
    void    drawFigureClicked(bool enb);
    void    drawFeatureClicked(bool enb);

protected:

private:
    AQTableWidget * protoTable;
};

#endif
