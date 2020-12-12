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

#ifndef STYLE_COLOR_FILL_GROUP_H
#define STYLE_COLOR_FILL_GROUP_H

#include <QtWidgets>
#include <QSignalMapper>
#include "tile/feature.h"
#include "makers/decoration_maker/style_editors.h"


class StyleColorFillGroup : public QObject
{
    Q_OBJECT

    enum eFGCol
    {
        COL_INDEX = 0,
        COL_COUNT = 1,
        COL_SIDES = 2,
        COL_AREA  = 3,
        COL_HIDE  = 4,
        COL_SEL   = 5,
        COL_BTN   = 6,
        COL_COLORS= 7
    };

public:
    StyleColorFillGroup(FaceGroup &fGroup, ColorGroup &cGroup, QVBoxLayout * vbox);
    void display();

signals:
    void sig_colorsChanged();

private slots:
    void slot_edit();

    void modify();
    void up();
    void down();
    void rptSet();
    void copySet();
    void pasteSet();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

    void slot_colorSetVisibilityChanged(int row);


private:
    FaceGroup     & faceGroup;
    ColorGroup    & colorGroup;

    AQTableWidget * table;
    QSignalMapper   mapper;
    ColorSet        copyPasteSet;
};
#endif
