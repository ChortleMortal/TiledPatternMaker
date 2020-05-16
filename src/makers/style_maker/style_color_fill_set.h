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
#include <QSignalMapper>
#include "tile/Feature.h"
#include "makers/style_maker/style_editors.h"

class StyleColorFillSet : public QObject
{
    Q_OBJECT

public:
    StyleColorFillSet(FilledEditor * editor, ColorSet & cset, QVBoxLayout * vbox);
    void displayColors(ColorSet & cset);

protected:
    void    createTable();
    QTableWidget  * table;

signals:
    void sig_colorsChanged();

private slots:
    void add();
    void modify();
    void del();
    void up();
    void down();
    void rptColor();
    void copyColor();
    void pasteColor();
    void slot_click(int row, int col);
    void slot_double_click(int row, int col);

    void slot_colorVisibilityChanged(int row);

private:
    FilledEditor  * ed;
    ColorSet &      colorSet;
    QSignalMapper   mapper;
    TPColor         copyPasteColor;
};
#endif
