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

#include <QSvgGenerator>
#include "style/style.h"

#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "base/geo_graphics.h"

int Style::refs = 0;

Style::Style(PrototypePtr proto) : Layer("Style")
{
    prototype = proto;
    debugMap = std::make_shared<Map>("Style debug map");
    paintSVG = false;
    generator = nullptr;
    refs++;
}

Style::Style(StylePtr other) : Layer(other)
{
    prototype = other->prototype;

    if (other->debugMap)
    {
        debugMap = other->debugMap;
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

void Style::eraseStyleMap()
{
    if (styleMap)
    {
        styleMap->wipeout();
        styleMap.reset();
    }
}

void Style::eraseProtoMap()
{
    prototype->resetProtoMap();
}

MapPtr Style::getMap()
{
    if (!styleMap)
    {
        styleMap = prototype->getProtoMap();
    }
    return styleMap;
}

MapPtr Style::getExistingMap()
{
    return prototype->getExistingProtoMap();
}

// uses existing tmpIndices
void Style::annotateEdges(MapPtr map)
{
    if (!map)
        return;

    debugMap->wipeout();

    int i=0;
    for (auto edge : map->getEdges())
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(i++));
    }
    //debugMap->dumpMap(false);
}

// Retrieve a name describing this style and map.
QString Style::getDescription() const
{
    //return prototype->getTiling()->getName() + " : " + getStyleDesc();
    return getStyleDesc();
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

    qDebug() << "Style::paint" << getDescription() << this;
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

    qDebug() << "Style::paintToSVG" << getDescription() << this;
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
        QPointF p1 = T.map(edge->v1->pt);
        QPointF p2 = T.map(edge->v2->pt);
        painter->drawLine(p1,p2);
    }

    const QVector<sText> & texts = debugMap->getTexts();
    for (auto t = texts.begin(); t != texts.end(); t++)
    {
        sText stxt = *t;
        QPointF pt = T.map(stxt.pt);
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
    }
}

void Style::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        qDebug() << getName() << config->kbdMode;
        if (view->getMouseMode() == MOUSE_MODE_CENTER && btn == Qt::LeftButton)
        {
            setCenterScreenUnits(spt);
            forceLayerRecalc();
            emit sig_refreshView();
        }
    }
}

void Style::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void Style::slot_mouseTranslate(QPointF pt)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + pt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + pt.y());
        forceLayerRecalc();
    }
}

void Style::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void Style::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void Style::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void Style::slot_wheel_scale(qreal delta)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceLayerRecalc();
    }
}

void Style::slot_wheel_rotate(qreal delta)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceLayerRecalc();
    }
}

void Style::slot_scale(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceLayerRecalc();
    }
}

void Style::slot_rotate(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceLayerRecalc();
    }
}

void Style:: slot_moveX(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceLayerRecalc();
    }
}

void Style::slot_moveY(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceLayerRecalc();
    }
}
