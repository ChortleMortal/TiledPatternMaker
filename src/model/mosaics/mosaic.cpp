#include <QDebug>
#include <QPainterPath>

#include "model/mosaics/mosaic.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/xform.h"
#include "sys/geometry/map.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/border.h"
#include "sys/qt/unique_qvector.h"
#include "model/prototypes/prototype.h"
#include "model/styles/style.h"

int Mosaic::refs = 0;

Mosaic::Mosaic()
{
    name.set(Sys::defaultMosaicName);
    _cleanseLevel       = 0;
    _cleanseSensitivity = 1e-4;
    refs++;
}

Mosaic::~Mosaic()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting mosaic:" << name;
#endif
    refs--;
}

void Mosaic::build()
{
    setupCleansing();

    for (auto & style : std::as_const(styleSet))
    {
        style->createStyleRepresentation();
    }
    if (_border)
    {
        _border->createStyleRepresentation();
    }
}

void Mosaic::build(ViewController * vc)
{
    setViewController(vc);
    build();
}

void Mosaic::setupCleansing()
{
    // cleanse information is copied into prototypes for use by BMPEngine
    // BMPEngine cannot access mosasic settings from prototype using Sys::
    auto protos = getPrototypes();
    for (auto & proto : protos)
    {
        proto->setCleanseLevel(_cleanseLevel);
        proto->setCleanseSensitivity(_cleanseSensitivity);
    }
}

void Mosaic::setViewController(ViewController * vc)
{
    for (const auto & style : std::as_const(styleSet))
    {
        ProtoPtr pp = style->getPrototype();
        pp->setViewController(vc);
        style->setViewController(vc);
    }
    if (_border)
    {
        _border->setViewController(vc);
    }
}

// This enginePaint is called only by MosaicBMPEngine
void Mosaic::enginePaint(QPainter * painter)
{
    QVector<Layer*> layers;
    for (auto const & style : std::as_const(styleSet))
    {
        layers.push_back(style.get());
    }

    auto firststyle  = styleSet.first();

    if (_border)
    {
        Xform xf = firststyle->getModelXform();
        _border->setModelXform(xf,false);

        if (!_border->getRequiresConstruction())
        {
            _border->createStyleRepresentation();       // construct now rather than later
        }

        layers.push_back(_border.get());
    }

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevelP);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto crop = getPainterCrop();
    if (crop)
    {
        auto transform = firststyle->getLayerTransform();
        switch (crop->getCropType())
        {
        case CROP_RECTANGLE:
        {
            auto rect = crop->getRect();
            rect = transform.mapRect(rect);
            qInfo() << "Painter Clip:" << rect;
            painter->setClipRect(rect);
        }   break;

        case CROP_POLYGON:
        {
            QPolygonF p = crop->getAPolygon().get();
            QPolygonF p2 = transform.map(p);
            //painter.setClipRegion(p2);
            QPainterPath pp;
            pp.addPolygon(p2);
            painter->setClipPath(pp);
        }   break;

        case CROP_CIRCLE:
        {
            Circle c = crop->getCircle();
            QRectF rect = c.boundingRect();
            rect = transform.mapRect(rect);
            QPainterPath p;
            p.addEllipse(rect.x(),rect.y(),rect.width(),rect.height());
            painter->setClipPath(p);
        }   break;

        case CROP_UNDEFINED:
            break;
        }
    }

    for (auto const & layer : std::as_const(layers))
    {
        painter->save();
        layer->forceLayerRecalc(false);
        layer->paint(painter);
        painter->restore();
    }
}

void  Mosaic::addStyle(StylePtr style)
{
    qDebug() << "Mosaic adding style: old count=" << styleSet.size();
    styleSet.push_front(style);
    CropPtr cp = style->getPrototype()->getCrop();
    if (cp)
        setCrop(cp);
}

void Mosaic::replaceStyle(StylePtr oldStyle, StylePtr newStyle)
{
    for (auto it = styleSet.begin(); it != styleSet.end(); it++)
    {
        StylePtr existingStyle = *it;
        if (existingStyle == oldStyle)
        {
            *it = newStyle;

            CropPtr cp = newStyle->getPrototype()->getCrop();
            if (cp)
                setCrop(cp);
            return;
        }
    }
}

void Mosaic::setName(VersionedName & vname)
{
    name = vname;
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
    for (const auto & proto : getPrototypes())
    {
        TilingPtr tp = proto->getTiling();
        if (tp)
        {
            tilings.push_back(tp);
        }
    }
    return static_cast<QVector<TilingPtr>>(tilings);
}

void Mosaic::setCrop(CropPtr crop)
{
    Q_ASSERT(crop);
    _crop = crop;

    for (const auto & proto : getPrototypes())
    {
        proto->setCrop(crop);       // resets proto map
    }
    resetStyleMaps();
}

void Mosaic::resetCrop()
{
    _crop.reset();

    auto vec = getPrototypes();
    for (const auto & proto : vec)
    {
        proto->resetCrop();       // resets proto map
    }

    resetProtoMaps();
    resetStyleMaps();
}

void Mosaic::setPainterCrop(CropPtr crop)
{
    Q_ASSERT(crop);
    _painterCrop = crop;
}

void Mosaic::resetPainterCrop()
{
    _painterCrop.reset();
}

void Mosaic::setBorder(BorderPtr border)
{
    _border = border;
}

VersionedName Mosaic::getName()
{
    return name;
}

QString Mosaic::getNotes()
{
    return designNotes;
}

QVector<ProtoPtr> Mosaic::getPrototypes()
{
    UniqueQVector<ProtoPtr> vec;
    for (const auto & style : std::as_const(styleSet))
    {
        ProtoPtr pp = style->getPrototype();
        vec.push_back(pp);
    }
    return static_cast<QVector<ProtoPtr>>(vec);
}

MapPtr Mosaic::getFirstExistingPrototypeMap()
{
    QVector<ProtoPtr> protos = getPrototypes();
    for (auto & proto : protos)
    {
        auto map = proto->getExistingProtoMap();
        if (!map->isEmpty())
        {
            return map;
        }
    }
    MapPtr map;
    return map;
}

void Mosaic::resetStyleMaps()
{
    for (const auto & style : std::as_const(styleSet))
    {
        style->resetStyleRepresentation();
    }
}

void Mosaic::resetProtoMaps()
{
    ProtoPtr nullProto;
    for (const auto & style : std::as_const(styleSet))
    {
        auto proto = style->getPrototype();
        proto->wipeoutProtoMap();
        style->resetStyleRepresentation();
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

void Mosaic::dump()
{
    qDebug() << "Mosaic" << name.get();
    for (const auto & style : styleSet)
    {
        MapPtr map;
        QString str;
        if (auto proto = style->getPrototype())
        {
              map = proto->getExistingProtoMap();
              if (map)
              {
                  str = map->summary();
              }
        }
        qDebug().noquote() << "Style" << style->getStyleDesc() << str;
    }
}

void Mosaic::dumpMotifs()
{
    qDebug() << "==== Mosaic Motifs" << name.get();
    auto protos = getPrototypes();
    for (const auto & proto : protos)
    {
        proto->dumpMotifs();
    }
    qDebug() << "=====";
}


void Mosaic::dumpStyles()
{
    qDebug() << "==== Mosaic Styles" << name.get();
    for (const auto & style : std::as_const(styleSet))
    {
        style->dump();
    }
    qDebug() << "=====";
}
