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

#include "base/mosaic.h"
#include "style/style.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "tapp/prototype.h"
#include "base/misc.h"

const QString Mosaic::defaultName =  "The Formless";

Mosaic::Mosaic()
{
    name     = defaultName;
}

Mosaic::~Mosaic()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting mosaic:" << name;
#endif
}

void  Mosaic::addStyle(StylePtr style)
{
    qDebug() << "Mosaic adding style: old count=" << styleSet.size();
    styleSet.push_front(style);
    BorderPtr bp = style->getPrototype()->getBorder();
    if (bp)
        setBorder(bp);
}

void Mosaic::replaceStyle(StylePtr oldStyle, StylePtr newStyle)
{
    for (auto it = styleSet.begin(); it != styleSet.end(); it++)
    {
        StylePtr existingStyle = *it;
        if (existingStyle == oldStyle)
        {
            *it = newStyle;

            BorderPtr bp = newStyle->getPrototype()->getBorder();
            if (bp)
                setBorder(bp);
            return;
        }
    }
}

void Mosaic::setName(QString name)
{
     this->name = name;
}

void Mosaic::setNotes(QString notes)
{
     designNotes = notes;
}

StylePtr  Mosaic::getFirstStyle()
{
    if (!styleSet.isEmpty())
    {
        return styleSet.first();
    }
    StylePtr sp;
    return sp;
}

QVector<TilingPtr> Mosaic::getTilings()
{
    UniqueQVector<TilingPtr> tilings;
    for (auto& proto : getPrototypes())
    {
        TilingPtr tp = proto->getTiling();
        tilings.push_back(tp);
    }
    return static_cast<QVector<TilingPtr>>(tilings);
}

void Mosaic::setPrototype(StylePtr style, PrototypePtr pp)
{
    pp->setBorder(border);
    pp->resetProtoMap();
    style->setPrototype(pp);
}

void Mosaic::setBorder(BorderPtr bp)
{
    border = bp;

    QVector<PrototypePtr>vec = getPrototypes();
    for (auto& proto : getPrototypes())
    {
        proto->setBorder(bp);       // resets proto map
    }
    DecorationMaker * dm = DecorationMaker::getInstance();
    dm->sm_takeUp(vec,SM_RENDER);   // resets style representstions
}

QString Mosaic::getName()
{
    return name;
}

QString Mosaic::getNotes()
{
    return designNotes;
}

QVector<PrototypePtr> Mosaic::getPrototypes()
{
    UniqueQVector<PrototypePtr> vec;
    for (auto& style : styleSet)
    {
        PrototypePtr pp = style->getPrototype();
        vec.push_back(pp);
    }
    return static_cast<QVector<PrototypePtr>>(vec);
}

void Mosaic::resetPrototypeMaps()
{
    for (auto& style : styleSet)
    {
        PrototypePtr pp = style->getPrototype();
        pp->resetProtoMap();
    }
}

void Mosaic::deleteStyle(StylePtr style)
{
    styleSet.removeAll(style);
}

int  Mosaic::moveUp(StylePtr style)
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

int Mosaic::moveDown(StylePtr style)
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
