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

#ifndef COLORSET_H
#define COLORSET_H

#include <QtCore>
#include <QColor>
#include "panels/panel_misc.h"

class ColorSet;

class TPColor
{
public:
    TPColor() { hidden = false; }
    TPColor(QColor color) { this->color = color; hidden = false; }
    TPColor(QColor color, bool hide) { this->color = color; this->hidden = hide; }

    QColor color;
    bool   hidden;
};

class  ColorGroup
{
public:
    ColorGroup();
    ColorGroup(const ColorGroup & other);

    void          addColorSet(ColorSet & cset);
    void          setColorSet(int idx, ColorSet & cset);
    ColorSet &    getNextColorSet();
    ColorSet &    getColorSet(int idx);
    void          removeColorSet(int idx);

    void          hide(int idx, bool hide);
    bool          isHidden(int idx);

    void          resetIndex() { pos = colorgroup.begin(); }

    int           size() { return colorgroup.size(); }
    void          resize(int num) { colorgroup.resize(num); }

    AQHBoxLayout * createLayout();
    AQWidget     * createWidget();

private:
    QVector<ColorSet>           colorgroup;
    QVector<ColorSet>::iterator pos;
};

class ColorSet
{
public:
    ColorSet();
    ColorSet(const ColorSet & other);

    void            addColor(QColor color, bool hide=false);
    void            addColor(TPColor color);
    void            setColor(int idx, QColor color, bool hide=false);
    void            setColor(int idx, TPColor tpcolor);

    void            setColors(QVector<TPColor> &cset);
    void            setColors(const ColorSet &cset);

    void            setOpacity(qreal val);

    void            removeColor(int idx);

    TPColor         getFirstColor();
    TPColor         getNextColor();
    TPColor         getColor(int index) { return colorset[index % colorset.size()]; }
    QString         colorsString();

    void            resetIndex() { pos = colorset.begin(); }
    void            clear();
    int             size() { return colorset.size(); }
    void            resize(int num) { colorset.resize(num); }

    void            hide(bool hide) { hidden = hide; }
    bool            isHidden() { return hidden; }

    AQHBoxLayout *  createLayout();
    AQWidget     *  createWidget();

protected:

private:
    QVector<TPColor> colorset;
    bool             hidden;
    QVector<TPColor>::iterator pos;
};

#endif // COLORSET_H
