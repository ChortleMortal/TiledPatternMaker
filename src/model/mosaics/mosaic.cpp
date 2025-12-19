#include <QDebug>
#include <QPainterPath>

#include "model/makers/mosaic_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "sys/qt/unique_qvector.h"

int Mosaic::refs = 0;

Mosaic::Mosaic()
{
    name.set(Sys::defaultMosaicName);
    _loadedXML_version     = MosaicWriter::currentXMLVersion; // default
    _legacyCenterConverted = false;
    refs++;
}

Mosaic::~Mosaic()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting mosaic:" << name;
#endif
    refs--;
}

bool Mosaic::isBuilt()
{
    for (auto & style : std::as_const(styleSet))
    {
        if (!style->isCreated())
        {
            return false;
        }
    }
    return true;
}

void Mosaic::build()
{
    for (auto & style : std::as_const(styleSet))
    {
        style->createStyleRepresentation();
    }
}

void Mosaic::setViewController(SystemViewController * vc)
{
    for (const auto & style : std::as_const(styleSet))
    {
        style->setViewController(vc);
        style->forceLayerRecalc(false);
        ProtoPtr pp = style->getPrototype();
        if (pp)
            pp->setViewController(vc);
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

    BkgdImagePtr bip = getBkgdImage();
    if (bip && Sys::config->includeBkgdGeneration)
    {
        bip->setViewController(getFirstRegularStyle()->viewControl());
        Sys::definedBkImage  = bip;
        Sys::currentBkgImage = BKGD_IMAGE_DEFINED;
        layers.push_front(bip.get());
    }

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevelP);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto crop = getPainterCrop();
    if (crop)
    {
        auto firststyle = getFirstRegularStyle();
        auto transform = firststyle->getLayerTransform();
        crop->setPainterClip(painter,transform);
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
    //qDebug() << "Mosaic adding style: old count=" << styleSet.size();
    styleSet.push_front(style);
    auto proto = style->getPrototype();
    if (proto)
    {
        // borders don't have protos
        CropPtr crop = proto->getCrop();
        if (crop)
            setCrop(crop);
    }
}

void Mosaic::replaceStyle(StylePtr oldStyle, StylePtr newStyle)
{
    for (auto it = styleSet.begin(); it != styleSet.end(); it++)
    {
        StylePtr existingStyle = *it;
        if (existingStyle == oldStyle)
        {
            *it = newStyle;

            ProtoPtr pp = newStyle->getPrototype();
            if (pp)
            {
                CropPtr cp = pp->getCrop();
                if (cp)
                {
                    setCrop(cp);
                }
            }
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

StylePtr  Mosaic::getFirstRegularStyle()
{
    for (auto & style: styleSet)
    {
        if (style->getStyleType() != STYLE_BORDER)
            return style;
    }
    StylePtr sp;
    return sp;
}

QVector<TilingPtr> Mosaic::getTilings()
{
    UniqueQVector<TilingPtr> tilings;
    for (auto & proto : getPrototypes())
    {
        TilingPtr tp = proto->getTiling();
        if (tp)
        {
            tilings.push_back(tp);
        }
    }
    return static_cast<QVector<TilingPtr>>(tilings);
}

void Mosaic::setBorder(BorderPtr bp)
{
    auto border = getBorder();
    if (border)
    {
        styleSet.removeOne(border);
    }
    addStyle(bp);
}

BorderPtr Mosaic::getBorder()
{
    for (auto & style : styleSet)
    {
        if (style->getStyleType() == STYLE_BORDER)
        {
            BorderPtr bp = std::dynamic_pointer_cast<Border>(style);
            return bp;
        }
    }
    BorderPtr bp;
    return bp;
}

void Mosaic::setCrop(CropPtr crop)
{
    Q_ASSERT(crop);
    _crop = crop;

    for (auto & proto : getPrototypes())
    {
        proto->setCrop(crop);       // resets proto map
    }
    resetStyleMaps();
}

void Mosaic::resetCrop()
{
    _crop.reset();

    auto vec = getPrototypes();
    for (auto & proto : vec)
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
        if (pp)
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
        if (proto)
        {
            proto->wipeoutProtoMap();
        }
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

void Mosaic::correctBorderAlignment(BorderPtr border)
{
    // correction for legacy XML files
    auto style           = getFirstRegularStyle();
    const Xform & xf_mos = style->getModelXform();
    QPointF mos_trans    = xf_mos.getTranslate();

    if (!mos_trans.isNull())
    {
        qInfo() << "Mosaic::correctBorderAlignment";
        Xform xf_bkgd       = border->getModelXform();
        QPointF bkgd_trans  = xf_bkgd.getTranslate();
        bkgd_trans         -= mos_trans;            // FIXME - should this be scaled
        xf_bkgd.setTranslate(bkgd_trans);

        border->setModelXform(xf_bkgd,false,Sys::nextSigid());
    }
}

void Mosaic::dump()
{
    qDebug() << "Mosaic" << name.get();
    for (auto & style : styleSet)
    {
        MapPtr map;
        QString str;
        auto proto = style->getPrototype();
        if (proto)
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
    for (auto & proto : protos)
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

void Mosaic::dumpTransforms()
{
    qDebug() << "==== Mosaic Styles" << name.get();
    for (const auto & style : std::as_const(styleSet))
    {
        auto t1 = style->getCanvasTransform();
        auto t2 = style->getModelTransform();
        auto t3 = style->getLayerTransform();
        qDebug() << Transform::info(t1) << Transform::info(t2) << Transform::info(t3);
    }
    qDebug() << "=====";
}
