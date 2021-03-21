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

#ifndef STYLE_COLOR_FILL_SET_H
#define STYLE_COLOR_FILL_SET_H

#include <QtWidgets>
#include "tile/feature.h"
#include "makers/decoration_maker/style_editors.h"

class StyleColorFillSet : public QObject
{
    Q_OBJECT

    enum eCol
    {
        COL_ROW         = 0,
        COL_FACES       = 1,
        COL_SIDES       = 2,
        COL_AREA        = 3,
        COL_HIDE        = 4,
        COL_SEL         = 5,
        COL_COLOR_TEXT  = 6,
        COL_COLOR_PATCH = 7
    };

public:
    StyleColorFillSet(FaceGroup & fgroup, ColorSet & cset, QVBoxLayout * vbox);
    void display();

signals:
    void sig_colorsChanged();

private slots:
    void modify();
    void up();
    void down();
    void rptColor();
    void copyColor();
    void pasteColor();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

protected:
    void colorChanged(int row);
    void colorVisibilityChanged(int row);

private:
    FaceGroup    & faceGroup;
    ColorSet     & colorSet;

    AQTableWidget  * table;
    TPColor         copyPasteColor;
};
#endif
