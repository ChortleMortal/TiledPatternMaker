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

#include "style/Style.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "base/view.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"

int Style::refs = 0;

Style::Style(PrototypePtr proto, PolyPtr bounds ) : Layer("Style")
{
    Q_ASSERT(proto != nullptr);
    Q_ASSERT(bounds != nullptr);
    prototype = proto;
    boundary  = bounds;
    setupStyleMap();
    refs++;
}

Style::Style(const Style &other) : Layer(other)
{
    prototype = other.prototype;
    boundary  = other.boundary;

    if (other.styleMap != nullptr)
    {
        styleMap = other.styleMap;
    }

    refs++;
}

Style::~Style()
{
    refs--;
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "style destructor";
    prototype.reset();
    boundary.reset();
    styleMap.reset();
#endif
}

void Style::setPrototype(PrototypePtr pp)
{
    prototype = pp;
    setupStyleMap();
}

MapPtr Style::setupStyleMap()
{
    Q_ASSERT(prototype);
    styleMap = prototype->getProtoMap();
    qDebug() << "Style::setupStyleMap proto=" << Utils::addr(prototype.get()) << "map="  << Utils::addr(styleMap.get());
    return styleMap;
}

// Retrieve a name describing this style and map.
QString Style::getDescription() const
{
    // TODO: improve the description with the settings of the prototype.
    return prototype->getTiling()->getName() + " : " + getStyleDesc();
}

QString Style::getInfo() const
{
    QString astring;
    if (prototype)
    {
        astring += prototype->getInfo();
        astring += " : ";
    }
    if (styleMap)
    {
        astring += styleMap->getInfo();
    }
    return astring;
}

void Style::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qDebug() << "Style::paint" << getDescription() << (void*)this;
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr;   // identity
    GeoGraphics gg(painter,tr);

    draw(&gg);
}
