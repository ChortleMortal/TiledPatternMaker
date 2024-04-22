#include <QDebug>
#include <QtMath>
#include "legacy/legacy_tile.h"

int LegacyTile::refs = 0;

LegacyTile::LegacyTile(int Row, int Col) : Layer("Master Tile",true)
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
    setModelXform(xf,true);
}

void LegacyTile::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    xf_model = xf;
    forceLayerRecalc(update);
}

const Xform & LegacyTile::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    return xf_model;
}
