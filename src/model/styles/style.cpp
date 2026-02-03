#include <QSvgGenerator>
#include <QDebug>

#include "model/styles/style.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/map_editor/map_editor.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tiling.h"

int Style::refs = 0;

Style::Style(const ProtoPtr & proto) : LayerController(VIEW_MOSAIC,PRIMARY,"Style")
{
    prototype  = proto;
    paintSVG   = false;
    created    = false;
    generator  = nullptr;
    setClipable(true);
    refs++;

    connect(Sys::mapEditor, &MapEditor::sig_styleMapUpdated, this, &Style::slot_styleMapUpdated);
}

Style::Style(eModelType modelType, QString name) : LayerController(VIEW_MOSAIC,modelType,name)
{
    created    = false;
    setClipable(true);
    refs++;
    connect(Sys::mapEditor, &MapEditor::sig_styleMapUpdated, this, &Style::slot_styleMapUpdated);
}

Style::Style(const StylePtr & other) : LayerController(other)
{
    prototype  = other->prototype;
    paintSVG   = false;
    created    = false;
    generator  = nullptr;
    setClipable(true);
    connect(Sys::mapEditor, &MapEditor::sig_styleMapUpdated, this, &Style::slot_styleMapUpdated);
    refs++;
}

Style::~Style()
{
    refs--;
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "style destructor";
    prototype.reset();
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
    GeoGraphics gg(painter,tr);

    draw(&gg);

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

void Style::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
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
