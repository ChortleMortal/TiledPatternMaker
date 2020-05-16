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

#include "panel_page.h"

enum eProtoCol
{
    PROTO_COL_DEL,
    PROTO_COL_FEATURE,
    PROTO_COL_FIGURE,
    PROTO_COL_TRANSFORM
};

class page_protos : public panel_page
{
    Q_OBJECT

public:

    page_protos(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

public slots:

private slots:
    void slot_display(int id);

protected:

private:
    QTableWidget * protoTable;
    QLabel       * wsProtoLabel;

    QRadioButton * sourceStyle;
    QRadioButton * sourceWS;
    QButtonGroup   bgroup;
};

#endif
