﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#include <QSvgGenerator>
#include "style/style.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "base/view.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"

int Style::refs = 0;

Style::Style(PrototypePtr proto, PolyPtr bounds ) : Layer("Style",LTYPE_VIEW)
{
    Q_ASSERT(proto != nullptr);
    Q_ASSERT(bounds != nullptr);
    prototype = proto;
    boundary  = bounds;
    debugMap = make_shared<Map>("Style debug map");
    paintSVG = false;
    generator = nullptr;
    refs++;
}

Style::Style(const Style &other) : Layer(other)
{
    prototype = other.prototype;
    boundary  = other.boundary;

    if (other.debugMap)
    {
        debugMap = other.debugMap;
    }
    paintSVG  = false;
    generator = nullptr;
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
    debugMap.reset();
#endif
}

void Style::setPrototype(PrototypePtr pp)
{
    prototype = pp;
}

TilingPtr Style::getTiling()
{
    TilingPtr tp;
    if (prototype)
    {
        tp = prototype->getTiling();
    }
    return tp;
}

MapPtr Style::getMap()
{
    if (!styleMap)
    {
        styleMap = prototype->getProtoMap();
    }
    return styleMap;
}


// uses existing tmpIndices
void Style::annotateEdges(MapPtr map)
{
    if (!map)
        return;

    debugMap->wipeout();
    for (auto edge : map->getEdges())
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(edge->getTmpEdgeIndex()));
    }
    //debugMap->dumpMap(false);
}

// Retrieve a name describing this style and map.
QString Style::getDescription() const
{
    return prototype->getTiling()->getName() + " : " + getStyleDesc();
}

QString Style::getInfo() const
{
    QString astring;
    if (prototype)
    {
        astring += prototype->getInfo();
        //astring += " : ";
        //astring += prototype->getProtoMap()->getInfo();
    }
    return astring;
}

void Style::paint(QPainter *painter)
{
    if (paintSVG)
    {
        paintToSVG();
        paintSVG = false;
        return;
    }

    qDebug() << "Style::paint" << getDescription() << Utils::addr(this);
    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);

    draw(&gg);

    if (!debugMap)
    {
        qWarning() << "Style::paint - no debug map";
        return;
    }

    if (!debugMap->isEmpty())
    {
        debugMap->dumpMap(false);
        drawAnnotation(painter,tr);
    }

    drawCenter(painter);
}

void Style::paintToSVG()
{
    if (generator == nullptr)
    {
        qWarning() << "QSvgGenerator not found";
        return;
    }

    QPainter painter;

    painter.begin(generator);

    qDebug() << "Style::paintToSVG" << getDescription() << Utils::addr(this);
    painter.setRenderHint(QPainter::Antialiasing ,true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(&painter,tr);

    draw(&gg);

    painter.end();

    generator = nullptr;
}

void Style::drawAnnotation(QPainter * painter, QTransform T)
{
    QPen pen(Qt::white);
    painter->setPen(pen);

    for (auto edge : debugMap->getEdges())
    {
        QPointF p1 = T.map(edge->getV1()->getPosition());
        QPointF p2 = T.map(edge->getV2()->getPosition());
        painter->drawLine(p1,p2);
    }

    if (debugMap->texts.count())
    {
        for (auto t = debugMap->texts.begin(); t != debugMap->texts.end(); t++)
        {
            sText stxt = *t;
            QPointF pt = T.map(stxt.pt);
            painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
        }
    }
}

