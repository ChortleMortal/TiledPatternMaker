﻿#include <QSvgGenerator>
#include <QDebug>

#include "style/style.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "misc/geo_graphics.h"
#include "makers/prototype_maker/prototype.h"
#include "tile/tiling.h"

int Style::refs = 0;

Style::Style(const ProtoPtr & proto) : LayerController("Style")
{
    prototype = proto;
    paintSVG = false;
    generator = nullptr;
    refs++;
}

Style::Style(const StylePtr & other) : LayerController(other)
{
    prototype = other->prototype;
    xf_canvas = other->xf_canvas;

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

void Style::setPrototype(ProtoPtr pp)
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

void Style::resetStyleMap()
{
    styleMap.reset();
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

#if 1
    debugMap = std::make_shared<DebugMap>("Style Debug Map");
#endif

    int i=0;
    for (auto & edge : qAsConst(map->getEdges()))
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(i++));
    }
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

    //qDebug() << "Style::paint" << getDescription() << this;
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);

    draw(&gg);

    if (debugMap)
    {
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
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

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

    for (auto & edge : qAsConst(debugMap->getEdges()))
    {
        QPointF p1 = T.map(edge->v1->pt);
        QPointF p2 = T.map(edge->v2->pt);
        painter->drawLine(p1,p2);
    }

    const QVector<QPair<QPointF,QString>> & texts = debugMap->getTexts();
    for (auto & pair : texts)
    {
        QPointF pt  = T.map(pair.first);
        QString txt = pair.second;
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),txt);
    }
}

void Style::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void Style::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
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

const Xform  & Style::getCanvasXform()
{
    return xf_canvas;
}

void Style::setCanvasXform(const Xform & xf)
{
    xf_canvas = xf;
}
