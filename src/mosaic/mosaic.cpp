#include <QDebug>

#include "mosaic/mosaic.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/border.h"
#include "misc/unique_qvector.h"
#include "makers/prototype_maker/prototype.h"
#include "style/style.h"

const QString Mosaic::defaultName =  "The Formless";

int Mosaic::refs = 0;

Mosaic::Mosaic()
{
    name         = defaultName;
    cleanseLevel = 0;
    refs++;
}

Mosaic::~Mosaic()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting mosaic:" << name;
#endif
    refs--;
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

void Mosaic::setBorder(BorderPtr border)
{
    _border = border;
}

QString Mosaic::getName()
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
    for (const auto & style : styleSet)
    {
        ProtoPtr pp = style->getPrototype();
        vec.push_back(pp);
    }
    return static_cast<QVector<ProtoPtr>>(vec);
}

MapPtr Mosaic::getPrototypeMap()
{
    // this assumes all styles have the same prototype map

    MapPtr map;

    if (styleSet.size() == 0)
        return map;

    auto style = styleSet.first();
    auto proto = style->getPrototype();
    map  = proto->getProtoMap();
    return map;
}

void Mosaic::resetStyleMaps()
{
    for (const auto & style : styleSet)
    {
        style->resetStyleRepresentation();
    }
}

void Mosaic::resetProtoMaps()
{
    ProtoPtr nullProto;
    for (const auto & style : styleSet)
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
    qDebug() << "Mosaic" << name;
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

void Mosaic::reportMotifs()
{
    qDebug() << "==== Mosaic Motifs" << name;
    auto protos = getPrototypes();
    for (const auto & proto : protos)
    {
        proto->reportMotifs();
    }
    qDebug() << "=====";
}


void Mosaic::reportStyles()
{
    qDebug() << "==== Mosaic Styles" << name;
    for (const auto & style : styleSet)
    {
        style->report();
    }
    qDebug() << "=====";
}
