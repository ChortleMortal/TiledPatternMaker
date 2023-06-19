#pragma once
#ifndef MOSAIC_IO_BASE_H
#define MOSAIC_IO_BASE_H

#include <QMultiMap>
#include "enums/emotiftype.h"

class MosaicIOBase
{
public:
    MosaicIOBase();

    QString    currentMotifName(eMotifType type);
    eMotifType getMotifType(QString name);

protected:
    QMultiMap<eMotifType,QString> motifRepresentation;

};

#endif // MOSAICREADERBASE_H
