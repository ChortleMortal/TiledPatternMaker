#include <QDebug>

#include "mosaic/mosaic.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/border.h"
#include "misc/unique_qvector.h"
#include "mosaic/prototype.h"
#include "style/style.h"

const QString Mosaic::defaultName =  "The Formless";

Mosaic::Mosaic()
{
    name = defaultName;
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
    CropPtr cp = style->getPrototype()->getCrop();
    if (cp)
        resetCrop(cp);
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
                resetCrop(cp);
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
        if (tp)
        {
            tilings.push_back(tp);
        }
    }
    return static_cast<QVector<TilingPtr>>(tilings);
}

void Mosaic::initCrop(CropPtr crop)
{
    this->crop = crop;
    border.reset();

    auto vec = getPrototypes();
    for (auto & proto : vec)
    {
        proto->initCrop(crop);       // resets proto map
    }
}

void Mosaic::resetCrop(CropPtr crop)
{
    this->crop = crop;
    border.reset();

    auto vec = getPrototypes();
    for (auto & proto : vec)
    {
        proto->resetCrop(crop);       // resets proto map
    }

    resetProtoMaps();
}

void Mosaic::_eraseCrop()
{
    CropPtr nullCrop;
    this->crop = nullCrop;

    auto vec = getPrototypes();
    for (auto & proto : vec)
    {
        proto->resetCrop(nullCrop);       // resets proto map
    }
}

void Mosaic::setBorder(BorderPtr border)
{
    this->border = border;
    _eraseCrop();
    resetStyleMaps();
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
    for (auto & style : styleSet)
    {
        style->resetStyleRepresentation();
    }
}

void Mosaic::resetProtoMaps()
{
    PrototypePtr nullProto;
    for (auto & style : styleSet)
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
    for (auto style : styleSet)
    {
        MapPtr map;
        QString str;
        if (auto proto = style->getPrototype())
        {
              map = proto->getExistingProtoMap();
              if (map)
              {
                  str = map->summary2();
              }
        }
        qDebug().noquote() << "Style" << style->getStyleDesc() << str;
    }
}
