#include <QtWidgets>
#include "tile/tiling.h"
#include "tile/tiling_data.h"
#include "tile/placed_tile.h"

using std::make_shared;

TilingData::TilingData()
{
}

TilingData::~TilingData()
{
    placed_tiles.clear();
}


void TilingData::init(Tiling * parent,QSize size, QPointF t1, QPointF t2)
{
    this->parent = parent;
    this->t1     = t1;
    this->t2     = t2;
    settings.setSize(size);
}


TilingData TilingData::copy()
{
    TilingData td;
    td.parent   = parent;
    td.settings = settings;
    td.t1       = t1;
    td.t2       = t2;
    for (auto & pf : placed_tiles)
    {
        auto pf2 = pf->copy();
        td.placed_tiles.push_back(pf2);
    }
    return td;
}

bool TilingData::isEmpty()
{
    if (placed_tiles.isEmpty() && t1.isNull() && t2.isNull())
        return true;
    else
        return false;
}

QVector<QTransform> TilingData::getFillTranslations()
{
    QVector<QTransform> translations;
    const FillData & fd = settings.getFillData();
    int minX,minY,maxX,maxY;
    bool singleton;
    fd.get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                QPointF pt   = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
                QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
                translations << T;
            }
        }
    }
    else
    {
        translations << QTransform();
    }
    return translations;
}


QString TilingData::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "t1=" << t1 << "t2=" << t2 << "num tiles=" << placed_tiles.size();
    return astring;
}

