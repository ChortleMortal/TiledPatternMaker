#include "mosaic/mosaic_io_base.h"

MosaicIOBase::MosaicIOBase()
{
    // Note: this table is ordered, so that the most recent (current) value is inserted later (after earlier)
    motifRepresentation.insert(MOTIF_TYPE_ROSETTE             , "app.Rosette");
    motifRepresentation.insert(MOTIF_TYPE_ROSETTE             , "Rosette");

    motifRepresentation.insert(MOTIF_TYPE_STAR                , "app.Star");
    motifRepresentation.insert(MOTIF_TYPE_STAR                , "Star");

    motifRepresentation.insert(MOTIF_TYPE_CONNECT_STAR        , "ConnectStar");

    motifRepresentation.insert(MOTIF_TYPE_CONNECT_ROSETTE     , "app.ConnectFigure");
    motifRepresentation.insert(MOTIF_TYPE_CONNECT_ROSETTE     , "ConnectRosette");

    motifRepresentation.insert(MOTIF_TYPE_EXTENDED_ROSETTE    , "ExtendedRosette");

    motifRepresentation.insert(MOTIF_TYPE_EXTENDED_STAR       , "ExtendedStar");

    motifRepresentation.insert(MOTIF_TYPE_EXPLICIT_MAP        , "app.ExplicitFigure");
    motifRepresentation.insert(MOTIF_TYPE_EXPLICIT_MAP        , "ExplicitMap");

    motifRepresentation.insert(MOTIF_TYPE_INFERRED            , "app.Infer");
    motifRepresentation.insert(MOTIF_TYPE_INFERRED            , "IrregularInfer");

    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_ROSETTE   , "app.ExplicitRosette");
    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_ROSETTE   , "IrregularRosette");

    motifRepresentation.insert(MOTIF_TYPE_HOURGLASS           , "app.ExplicitHourglass");
    motifRepresentation.insert(MOTIF_TYPE_HOURGLASS           , "IrregularHourglass");

    motifRepresentation.insert(MOTIF_TYPE_INTERSECT           , "app.ExplicitIntersect");
    motifRepresentation.insert(MOTIF_TYPE_INTERSECT           , "IrregularIntersect");

    motifRepresentation.insert(MOTIF_TYPE_GIRIH               , "app.ExplicitGirih");
    motifRepresentation.insert(MOTIF_TYPE_GIRIH               , "IrregularGirih");

    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_STAR      , "app.ExplicitStar");
    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_STAR      , "IrregularStar");

    motifRepresentation.insert(MOTIF_TYPE_EXPLCIT_TILE        , "app.ExplicitFeature");
    motifRepresentation.insert(MOTIF_TYPE_EXPLCIT_TILE        , "ExplicitTile");

    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_NO_MAP    , "IrregularNoMotif");
    motifRepresentation.insert(MOTIF_TYPE_IRREGULAR_NO_MAP    , "NoMotif");
}

QString MosaicIOBase::currentMotifName(eMotifType type)
{
    auto list = motifRepresentation.values(type);
    return list.first();
}


eMotifType MosaicIOBase::getMotifType(QString name)
{
    auto i = motifRepresentation.cbegin();
    while (i != motifRepresentation.cend())
    {
        if (i.value() == name)
            return i.key();
        ++i;
    }

    return MOTIF_TYPE_UNDEFINED;
}
