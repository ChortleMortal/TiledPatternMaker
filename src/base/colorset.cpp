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

#include "base/colorset.h"

ColorSet::ColorSet()
{
    pos = colorset.end();
    hidden = false;
}

ColorSet::ColorSet(const ColorSet & other)
{
    setColors(other);
    hidden = other.hidden;
}

void ColorSet::clear()
{
    colorset.clear();
    pos = colorset.begin();
}

void ColorSet::setColor(int idx, QColor color, bool hide)
{
    colorset.replace(idx,TPColor(color,hide));
    pos = colorset.begin();
}

void ColorSet::setColor(int idx, TPColor tpcolor)
{
    colorset.replace(idx,tpcolor);
    pos = colorset.begin();
}

void ColorSet::addColor(QColor color, bool hidden)
{
    colorset.push_back(TPColor(color,hidden));
    pos = colorset.begin();
}

void ColorSet::addColor(TPColor color)
{
    colorset.push_back(color);
    pos = colorset.begin();
}

void ColorSet::setColors(QVector<TPColor> &cset)
{
    clear();
    for (auto it = cset.begin(); it != cset.end(); it++)
    {
        TPColor tp = *it;
        colorset.push_back(tp);
    }
    pos = colorset.begin();
}

void  ColorSet::setColors(const ColorSet & cset)
{
    clear();
    colorset = cset.colorset;
    pos = colorset.begin();
}

void ColorSet::setOpacity(qreal val)
{
    for (auto& tpcolor : colorset)
    {
        QColor c = tpcolor.color;
        c.setAlpha(int(val));
        tpcolor.color = c;
    }
}

TPColor ColorSet::getFirstColor()
{
    pos = colorset.begin();
    return getNextColor();
}

TPColor ColorSet::getNextColor()
{
    if (size() == 0)
    {
        colorset.push_back(TPColor(Qt::yellow,false));
        pos = colorset.begin();
    }
    Q_ASSERT(pos != colorset.end());

    TPColor color = *pos;
    if (++pos == colorset.end())
    {
        pos = colorset.begin();
    }
    return color;
}

void ColorSet::removeColor(int idx)
{
    colorset.removeAt(idx);
    pos = colorset.begin();
}

QString ColorSet::colorsString()
{
    QString str;
    for (auto it = colorset.begin(); it != colorset.end(); it++)
    {
        TPColor tpcolor = *it;
        str += tpcolor.color.name(QColor::HexArgb);
        str += " ";
    }
    return str;
}

AQWidget * ColorSet::createWidget()
{
    AQHBoxLayout * hbox = createLayout();
    AQWidget   * widget = new AQWidget;
    widget->setLayout(hbox);
    return widget;
}

AQHBoxLayout * ColorSet::createLayout()
{
    AQHBoxLayout * hbox = new AQHBoxLayout;
    for (auto it = colorset.begin(); it != colorset.end(); it++)
    {
        TPColor tpcolor  = *it;
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);
        QLabel * label   = new QLabel;
        label->setMinimumWidth(50);
        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        hbox->addWidget(label);
    }
    return hbox;
}

////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////

ColorGroup::ColorGroup()
{
    pos = colorgroup.begin();
}

ColorGroup::ColorGroup(const ColorGroup & other)
{
    colorgroup = other.colorgroup;
    pos = colorgroup.begin();
}

void ColorGroup::addColorSet(ColorSet & cset)
{
    colorgroup.push_back(cset);
    pos = colorgroup.begin();
}

void ColorGroup::setColorSet(int idx, ColorSet & cset)
{
    colorgroup.replace(idx,cset);
    pos = colorgroup.begin();
}

ColorSet & ColorGroup::getColorSet(int idx)
{
    return colorgroup[idx];
}

void ColorGroup::removeColorSet(int idx)
{
    colorgroup.removeAt(idx);
    pos = colorgroup.begin();
}

void  ColorGroup::resetIndex()
{
    pos = colorgroup.begin();
    for (auto fset : qAsConst(colorgroup))
    {
        fset.resetIndex();
    }
}

ColorSet & ColorGroup::getNextColorSet()
{
    if (size() == 0)
    {
        ColorSet colorSet;
        colorgroup.push_back(colorSet);
        pos = colorgroup.begin();
    }
    ColorSet & cset =  *pos;
    if (++pos == colorgroup.end())
    {
        pos =colorgroup.begin();
    }
    return cset;
}

AQWidget * ColorGroup::createWidget()
{
    AQHBoxLayout * hbox = createLayout();
    AQWidget   * widget = new AQWidget;
    widget->setLayout(hbox);
    return widget;
}

AQHBoxLayout * ColorGroup::createLayout()
{
    AQHBoxLayout * hbox = new AQHBoxLayout;
    for (auto it = colorgroup.begin(); it != colorgroup.end(); it++)
    {
        ColorSet & cset = *it;
        AQHBoxLayout * h2 = cset.createLayout();
        hbox->addLayout(h2);
        hbox->addSpacing(5);
    }
    return hbox;
}

void ColorGroup::hide(int idx, bool hide)
{
    if (idx >= 0 && idx < size())
    {
        ColorSet & cs =  colorgroup[idx];
        cs.hide(hide);
    }

}
bool  ColorGroup::isHidden(int idx)
{
    if (idx >= 0 && idx < size())
    {
        ColorSet & cs =  colorgroup[idx];
        return cs.isHidden();
    }
    return false;
}
