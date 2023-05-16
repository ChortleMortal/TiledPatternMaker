#pragma once
#ifndef MOSAIC_IO_BASE_H
#define MOSAIC_IO_BASE_H

#include <QMap>
#include "enums/emotiftype.h"

class MosaicIOBase
{
public:
    MosaicIOBase();

protected:
    QMap<eMotifType,QString> motifRepresentation;

};

#endif // MOSAICREADERBASE_H
