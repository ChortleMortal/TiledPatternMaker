#include <QDebug>
#include <QtMath>
#include "legacy/legacy_tile.h"

int LegacyTile::refs = 0;

LegacyTile::LegacyTile(int Row, int Col) : Layer(VIEW_LEGACY,PRIMARY,"Master Tile")
{
    row = Row;
    col = Col;

    instance = refs++;
}

LegacyTile::~LegacyTile()
{
    refs--;
}

void LegacyTile::addLayer(LayerPtr layer, int zlevel)
{
    layer->setZValue(zlevel);
    addSubLayer(layer);
}

bool LegacyTile::doStep(int index)
{
    qDebug() << "Tile::step" << index;
    return false;
}

qreal LegacyTile::getSLinearPos(int step, int duration)
{
    if (step == 0)
    {
        return 0.0;
    }
    if (step == duration)
    {
        return 1.0;
    }
    // put position in range -6 to +6
    qreal x  = (qreal)step * (13.0/((qreal)duration)) - 6.0;
    qreal normalizedValue = 1.0 / (1 + exp(-x));
    return normalizedValue;
}

void LegacyTile::tile_rotate(int amount)
{
    Xform xf = getModelXform();
    xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
    setModelXform(xf,true,0);
}
