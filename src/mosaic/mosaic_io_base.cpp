#include "mosaic/mosaic_io_base.h"

MosaicIOBase::MosaicIOBase()
{
    motifRepresentation[MOTIF_TYPE_ROSETTE]             = "app.Rosette";
    motifRepresentation[MOTIF_TYPE_STAR]                = "app.Star";
    motifRepresentation[MOTIF_TYPE_CONNECT_STAR]        = "app.ConnectFigure";
    motifRepresentation[MOTIF_TYPE_CONNECT_ROSETTE]     = "app.ConnectFigure";      // FIXME - one of these must be wrong
    motifRepresentation[MOTIF_TYPE_EXTENDED_ROSETTE]    = "ExtendedRosette";
    motifRepresentation[MOTIF_TYPE_EXTENDED_STAR]       = "ExtendedStar";
    motifRepresentation[MOTIF_TYPE_EXPLICIT_MAP]        = "app.ExplicitFigure";
    motifRepresentation[MOTIF_TYPE_INFERRED]            = "app.Infer";
    motifRepresentation[MOTIF_TYPE_IRREGULAR_ROSETTE]   = "app.ExplicitRosette";
    motifRepresentation[MOTIF_TYPE_HOURGLASS]           = "app.ExplicitHourglass";
    motifRepresentation[MOTIF_TYPE_INTERSECT]           = "app.ExplicitIntersect";
    motifRepresentation[MOTIF_TYPE_GIRIH]               = "app.ExplicitGirih";
    motifRepresentation[MOTIF_TYPE_IRREGULAR_STAR]      = "app.ExplicitStar";
    motifRepresentation[MOTIF_TYPE_EXPLCIT_TILE]        = "app.ExplicitFeature";
    motifRepresentation[MOTIF_TYPE_IRREGULAR_NO_MAP]    = "IrregularNoMotif";
}
