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

#ifndef DLG_COLOR_SET_H
#define DLG_COLOR_SET_H

#include <QtWidgets>
#include <QSignalMapper>
#include "tile/Feature.h"
#include "makers/style_maker/style_editors.h"

class DlgColorSet : public QDialog
{
    Q_OBJECT

public:
    DlgColorSet(ColorSet & cset, QWidget * parent = nullptr);

protected:
    virtual void displayTable();

    QTableWidget  * table;

signals:
    void sig_colorsChanged();

private slots:
    void add();
    void modify();
    void del();
    void slot_ok();
    void up();
    void down();
    void slot_colorVisibilityChanged(int row);

private:
    ColorSet &      colorSet;
    QSignalMapper   mapper;
    int             currentRow;
};

#endif
