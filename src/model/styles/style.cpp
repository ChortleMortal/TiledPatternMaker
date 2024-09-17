#include <QSvgGenerator>
#include <QDebug>

#include "model/styles/style.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/geo_graphics.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tiling.h"
#include "gui/viewers/debug_view.h"

int Style::refs = 0;

Style::Style(const ProtoPtr & proto) : LayerController("Style",true)
{
    prototype = proto;
    paintSVG  = false;
    generator = nullptr;
    debugMap  = nullptr;
    refs++;
}

Style::Style(const StylePtr & other) : LayerController(other)
{
    prototype = other->prototype;

    if (other->debugMap)
    {
        debugMap = other->debugMap;
    }
    paintSVG  = false;
    generator = nullptr;
    debugMap  = nullptr;

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

MapPtr Style::getExistingProtoMap()
{
    return prototype->getExistingProtoMap();
}

// uses existing tmpIndices
void Style::annotateEdges(MapPtr map)
{
    if (!map)
        return;

    uint dbgVal = 0x00;
    debugMap    = nullptr;
    if (dbgVal > 0)
    {
        debugMap = Sys::debugView->getMap();
    }

    int i=0;
    for (auto & edge : std::as_const(map->getEdges()))
    {
        QPointF p = edge->getMidPoint();
        if (debugMap) debugMap->insertDebugMark(p, QString::number(i++));
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
        astring += prototype->info();
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

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();

    //qDebug().noquote() << "Style::paint" << getDescription() << Transform::toInfoString(tr);

    GeoGraphics gg(painter,tr);

    draw(&gg);

    drawAnnotation(painter,tr);
    drawLayerModelCenter(painter);
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
    if (debugMap == nullptr)
    {
        return;
    }

    QPen pen(Qt::white);
    painter->setPen(pen);

    for (auto & edge : std::as_const(debugMap->getEdges()))
    {
        QPointF p1 = T.map(edge->v1->pt);
        QPointF p2 = T.map(edge->v2->pt);
        painter->drawLine(p1,p2);
    }

    const QVector<QPair<QPointF,QString>> & texts = debugMap->getTexts();
    for (auto & pair : std::as_const(texts))
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

void Style::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    xf_model = xf;
    forceLayerRecalc(update);
}

const Xform & Style::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    return xf_model;
}
