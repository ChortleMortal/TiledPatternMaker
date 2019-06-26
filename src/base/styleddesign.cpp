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

#include "base/styleddesign.h"
#include "style/Style.h"
#include <QDebug>

void StyledDesign::clear()
{
    styleSet.clear(); // can now delete these since scene uses a copy
    name.clear();
    designNotes.clear();
    canvasSettings.init();
}

void  StyledDesign::addStyle(StylePtr style)
{
    qDebug() << "adding style to workspace: old count=" << styleSet.size();
    styleSet.push_front(style);
}

void StyledDesign::replaceStyle(StylePtr oldStyle, StylePtr newStyle)
{
    for (auto it = styleSet.begin(); it != styleSet.end(); it++)
    {
        StylePtr existingStyle = *it;
        if (existingStyle == oldStyle)
        {
            *it = newStyle;
            return;
        }
    }
}

void StyledDesign::setName(QString name)
{
     this->name = name;
}

void StyledDesign::setNotes(QString notes)
{
     designNotes = notes;
}

CanvasSettings & StyledDesign::getCanvasSettings()
{
    return canvasSettings;
}

void StyledDesign::setCanvasSettings(CanvasSettings settings)
{
    canvasSettings = settings;
}

StylePtr  StyledDesign::getFirstStyle()
{
    StylePtr sp;
    if (styleSet.size())
    {
        sp =styleSet[0];
    }
    return sp;
}

// TODO - thid assumes a single tiling
TilingPtr StyledDesign::getTiling()
{
    TilingPtr tp;
    if (styleSet.size())
    {
        StylePtr sp = styleSet[0];
        if (sp)
        {
            PrototypePtr pp = sp->getPrototype();
            if (pp)
            {
                tp = pp->getTiling();
            }
        }
    }
    return tp;
}

QString StyledDesign::getName()
{
    return name;
}

QString StyledDesign::getNotes()
{
    return designNotes;
}

QVector<PrototypePtr> StyledDesign::getPrototypes()
{
    QVector<PrototypePtr> vec;

    for (auto it = styleSet.begin(); it != styleSet.end(); it++)
    {
        StylePtr sp = *it;
        PrototypePtr pp = sp->getPrototype();
        if (!vec.contains(pp))
        {
            vec.push_back(pp);
        }
    }
    return vec;
}

void StyledDesign::deleteStyle(StylePtr style)
{
    styleSet.removeAll(style);
}

int  StyledDesign::moveUp(StylePtr style)
{
    for (int i=0; i < styleSet.size(); i++)
    {
        if (styleSet[i] == style)
        {
            if (i == 0)
                return 0;
            styleSet.takeAt(i);
            styleSet.insert((i-1),style);
            return (i-1);
        }
    }
    return 0;
}

int StyledDesign::moveDown(StylePtr style)
{
    for (int i=0; i < styleSet.size(); i++)
    {
        if (styleSet[i] == style)
        {
            if (i == styleSet.size()-1)
                return i;
            styleSet.takeAt(i);
            styleSet.insert((i+1),style);
            return (i+1);
        }
    }
    return 0;
}
