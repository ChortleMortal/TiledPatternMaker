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
    getRWInTiling().clear();
}

TilingData TilingData::copy()
{
    TilingData td;
    td.settings = settings;
    td.t1       = t1;
    td.t2       = t2;
    for (const auto & pf : getInTiling())
    {
        auto pf2 = pf->copy();
        td.getRWInTiling().push_back(pf2);
    }
    return td;
}

bool TilingData::isEmpty()
{
    if (getInTiling().isEmpty() && t1.isNull() && t2.isNull())
        return true;
    else
        return false;
}

Placements TilingData::getFillPlacemenets()
{
    Placements placements;
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
                placements << T;
            }
        }
    }
    else
    {
        placements << QTransform();
    }
    return placements;
}


QString TilingData::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "t1=" << t1 << "t2=" << t2 << "num tiles=" << getInTiling().size();
    return astring;
}

