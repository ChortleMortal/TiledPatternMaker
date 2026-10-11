#include <algorithm>

#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/qt/timers.h"
#include "sys/sys.h"

using std::make_shared;

int Prototype::refs = 0;

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from tiles to motifs.  Prototype knows how to turn
// this information into a finished design, returned as a Map.
//
// In other words the purpose of a prototype is to produce a Map
// This map can then be used to make a DCEL which has faces that
// can be colored (filled)

Prototype::Prototype(TilingPtr t, MosaicPtr m)
{
    Q_ASSERT(t);
    Q_ASSERT(m);
    _tiling         = t;
    wMosaic         = m;
    _protoMap       = make_shared<Map>("ProtoMap ");
    cleanseLevel    = 0;
    cleanseSensitivity = 0;
    refs++;
}

 // for use only by PrototypeMaker
Prototype::Prototype(TilingPtr t)
{
    Q_ASSERT(t);
    _tiling         = t;
    _protoMap       = make_shared<Map>("ProtoMap ");
    cleanseLevel    = 0;
    cleanseSensitivity = 0;
    refs++;
}

Prototype::Prototype(MapPtr map)
{
    // This is a special case used by the map editor where the map is already prepared
    _protoMap      = map;
    cleanseLevel   = 0;
    cleanseSensitivity = 0;
    refs++;
}

Prototype::~Prototype()
{
    //qDebug() << "Prototype destructor";
#ifdef EXPLICIT_DESTRUCTOR
    _protoMap.reset();
    _crop.reset();
#endif
    refs--;
}

bool Prototype::operator==(const Prototype & other)
{
    if (_tiling != other._tiling)
        return false;

    if (_tileMotifSet.size() != other._tileMotifSet.size())
        return false;

    for (int i=0; i <  _tileMotifSet.size(); i++)
    {
        TileMotifPtr ele  = _tileMotifSet[i];
        TileMotifPtr eleo = other._tileMotifSet[i];

        if (ele->getTile() != eleo->getTile())
            return  false;

        if (ele->getMotif() != eleo->getMotif())
            return  false;
    }
    return  true;
}

bool Prototype::hasContent()
{
    return (_tileMotifSet.size() > 0) || !_protoMap->isEmpty();
}

void Prototype::wipeoutProtoMap()
{
    if (!_tiling)
        return;     // tiling-less prototype: the map IS the prototype, nothing to rebuild it from

    _DCEL.reset();                  // dcel is subordinate so must be erased too.
    _dcelFailed = false;
    _protoMap->clear();
    Q_ASSERT(_protoMap->isEmpty());
}

void Prototype::setProtoMap(MapPtr map)
{
    // map may be the same object, edited in place, so the dcel is always stale
    _protoMap   = map;
    _DCEL.reset();
    _dcelFailed = false;
}

MapPtr Prototype::getProtoMap()
{
    Q_ASSERT(_protoMap);
    if (_protoMap->isEmpty())
    {
        splashCreateProtoMap();    // build on demnd
    }
    return _protoMap;
}

void Prototype::splashCreateProtoMap()
{
    QMutexLocker locker(&protoMutex);

    Q_ASSERT(_protoMap);
    Q_ASSERT(_protoMap->isEmpty());

    if (_tileMotifSet.size() == 0)
    {
        return;
    }

    if (!Sys::imgGeneratorInUse)
        Sys::systemViewController->splash_display("Creating Prototype Map...");

    AQElapsedTimer timer;

    qDebug() << "PROTOTYPE CONSTRUCT MAP";
    QString astring = QString("Constructing prototype map for tiling: %1").arg(_tiling->getVName().get());
    qDebug().noquote() << astring;

    createProtoMap();

    qDebug().noquote() << "PROTOTYPE COMPLETED MAP:" << _protoMap->info();
    qDebug().noquote() << "Prototype construction" << timer.getElapsed() << "seconds";

    if (!Sys::imgGeneratorInUse)
        Sys::systemViewController->splash_remove();
}

Canvas * Prototype::getCanvas()
{
    MosaicPtr mosaic = wMosaic.lock();
    Q_ASSERT(mosaic);
    return mosaic->getMosaicCanvas();
}

void Prototype::createProtoMap()
{
    Q_ASSERT(_tiling);
    Q_ASSERT(_protoMap->isEmpty());

    // Use FillRegion to get a list of translations for this tiling.
    // Note that the fill data could be from the mosaic rather than the tiling
    // this is where the locations for the replicated styles are put
    // each motif point is placed with a relative transform from the tiling when the map is constructed

    FillRegion flood(_tiling.get(),getCanvas()->getFillData());
    Placements fillPlacements = flood.getPlacements(Sys::config->repeatMode);

    if (_tileMotifSet.size() == 1 && fillPlacements.size() == 1 && fillPlacements[0].isIdentity())
    {
        auto tile = _tileMotifSet[0]->getTile();
        if (_tiling->unit().getPlacements(tile).size() == 0)
        {
            // This is a special edge case and seems a kludge, fighting the system.
            // With more thought a better implementation of this idea could be made
            auto motif = _tileMotifSet[0]->getMotif();
            _protoMap  = motif->getMotifMap();
            qDebug() << "PROTOTYPE is motif map";
        }
    }

    if (_protoMap->isEmpty() && (_tileMotifSet.size() > 0))
    {
        buildMotifMaps();

        //AQElapsedTimer timer2;
        _buildPrototypeMap(fillPlacements);
        //qInfo().noquote() << "_buildPrototypeMap" << timer2.getElapsed() << "seconds";

        if (!_protoMap->isEmpty())
            qDebug() << "PROTOTYPE merged";
        else
            qDebug() << "PROTOTYPE empty";
    }

    if (_crop)
    {
        // cropOutside() requires the map to already be split at the crop boundary (see
        // cropOutside(QRectF) in map.cpp), so when both embed and apply are requested (the
        // default for pre-embed/apply-attribute XML, see MosaicReader), embedCrop() only needs
        // to be called once - calling it a second time on top of that isn't a no-op (mergeMap
        // can't detect that a boundary segment is already represented by a chain of colinear
        // fragments from the first pass) and would insert redundant overlapping edges.
        if (_crop->getCropType() == CROP_RECTANGLE)
        {
            const QRectF & rect = _crop->getRect();
            if (rect.isValid())
            {
                if (_crop->getApply())
                {
                    _protoMap->embedCrop(rect);
                    _protoMap->cropOutside(rect);
                }
                else if (_crop->getEmbed())
                {
                    _protoMap->embedCrop(rect);
                }
                qDebug() << "Crop-rect merged";
            }
        }
        else if (_crop->getCropType() == CROP_CIRCLE)
        {
            const Circle & circle = _crop->getCircle();
            if (_crop->getApply())
            {
                _protoMap->embedCrop(circle);
                _protoMap->cropOutside(circle);
            }
            else if (_crop->getEmbed())
            {
                _protoMap->embedCrop(circle);
            }
            qDebug() << "Crop-circle merged";
        }
        else if (_crop->getCropType() == CROP_POLYGON)
        {
            QPolygonF poly = _crop->getAPolygon().get();
            if (_crop->getApply())
            {
                _protoMap->embedCrop(poly);
                _protoMap->cropOutside(poly);
            }
            else if (_crop->getEmbed())
            {
                _protoMap->embedCrop(poly);
            }
            qDebug() << "Crop-poly merged";
        }
    }

    if (!_protoMap->isEmpty())
    {
        if (cleanseLevel)
        {
            qDebug() << "cleanse level (hex)" << Qt::hex << cleanseLevel;
            qDebug().noquote() << "pre proto map cleanse:" << _protoMap->summary();
            MapCleanser mc(_protoMap);
            mc.cleanse(cleanseLevel,cleanseSensitivity);
            qDebug().noquote() << "post proto map cleanse:" << _protoMap->summary();
        }
    }

    MapVerifier mv(_protoMap);
    mv.verifyAndFix(Sys::config->forceVerifyProtos);
}

void Prototype::buildMotifMaps()
{
    // The InferredMotif type is dependant on its surrounding motifs.
    // So these need to be prepared before the prototype map
    // can be built

    // pass 1 - build all motif maps
    int num_inferred = 0;
    for (TileMotifPtr & timo : _tileMotifSet)
    {
        MotifPtr motif = timo->getMotif();
        if (motif->getMotifType() == Motif::Type::Inferred)
            num_inferred++;
        motif->build();
    }

    // pass 2 - rebuild InferredMotifs
    for (int i=0; i < num_inferred; i++)
    {
        for (const TileMotifPtr & timo : std::as_const(_tileMotifSet))
        {
            MotifPtr motif = timo->getMotif();
            if (motif->getMotifType() == Motif::Type::Inferred)
            {
                motif->build();
            }
        }
    }
}

void Prototype::_buildPrototypeMap(Placements & fillPlacements)
{
    for (TileMotifPtr & timo : _tileMotifSet)
    {
        TilePtr tile              = timo->getTile();
        Placements tilePlacements = _tiling->unit().getPlacements(tile,getCanvas()->getDistorionTransform());
        if (!tilePlacements.size())
            tilePlacements.push_back(QTransform());   // dummy tilings have no placements

        MotifPtr motif  = timo->getMotif();
        MapPtr motifMap = motif->getMotifMap();
        if (!motifMap)
        {
            qWarning("empty motif map");
            motifMap = make_shared<Map>("Kludge map");
        }

        MapPtr unitMap =  make_shared<Map>("proto unit map");
        unitMap->mergeMany(motifMap, tilePlacements);

        MapPtr tileMap = make_shared<Map>("proto tile map");
        tileMap->mergeMany(unitMap, fillPlacements);

        _protoMap->mergeMap(tileMap);
    }
}

const DCELPtr & Prototype::getDCEL()
{
    if (!_DCEL && !_dcelFailed)
    {
        QMutexLocker locker(&dcelMutex);

        auto protomap = getProtoMap();
        auto dcel     = std::make_shared<DCEL>(protomap.get());
        if (dcel->build())
        {
            _DCEL = dcel;
            protomap->setDerivedDCEL(dcel);
        }
        else
        {
            _dcelFailed = true;     // not retried until the proto map is wiped
        }
    }
    return _DCEL;
}

void Prototype::replaceTiling(const TilingPtr & newTiling)
{
    // replace the tiles (keeping the motifs) where possible:
    //  1) exact geometric matches keep their existing TileMotif untouched
    //  2) leftover old/new tiles are handed to pairModifiedTiles() - tiles that were
    //     probably edited (moved/rescaled/rotated) rather than added or deleted outright
    //  3) whatever's still unpaired after that is a real deletion or a real addition

    Q_ASSERT(newTiling);
    analyze(newTiling);

    _tiling = newTiling;

    _protoMap->clear();

    const QVector<TilePtr> newTiles = newTiling->unit().getUniqueTiles();

    // pass 1: exact matches
    TileMotifSet     matchedTileMotifs;
    QVector<TilePtr> unmatchedNewTiles;

    for (const TilePtr & newTile : std::as_const(newTiles))
    {
        bool matched = false;
        for (const TileMotifPtr & timo : std::as_const(_tileMotifSet))
        {
            if (matchedTileMotifs.contains(timo))
            {
                continue;
            }
            if (newTile->isBasicallyEqual(timo->getTile()))
            {
                timo->exactReplaceTile(newTile);
                matchedTileMotifs.push_back(timo);
                matched = true;
                break;
            }
        }
        if (!matched)
        {
            unmatchedNewTiles.push_back(newTile);
        }
    }

    QVector<TileMotifPtr> unmatchedOldTileMotifs;
    for (const TileMotifPtr & timo : std::as_const(_tileMotifSet))
    {
        if (!matchedTileMotifs.contains(timo))
        {
            unmatchedOldTileMotifs.push_back(timo);
        }
    }

    // pass 2: pair up likely-edited tiles among what's left
    const QVector<QPair<TileMotifPtr,TilePtr>> modifiedPairs = pairModifiedTiles(unmatchedOldTileMotifs, unmatchedNewTiles);

    QVector<TileMotifPtr> pairedOld;
    QVector<TilePtr>      pairedNew;
    for (const auto & pr : std::as_const(modifiedPairs))
    {
        pr.first->replaceModifiedTile(pr.second);
        pairedOld.push_back(pr.first);
        pairedNew.push_back(pr.second);
    }

    // pass 3: whatever's left is a real deletion or a real addition
    for (const TileMotifPtr & timo : std::as_const(unmatchedOldTileMotifs))
    {
        if (!pairedOld.contains(timo))
        {
            removeTileMotif(timo);
        }
    }

    for (const TilePtr & tile : std::as_const(unmatchedNewTiles))
    {
        if (!pairedNew.contains(tile))
        {
            TileMotifPtr timo = make_shared<TileMotif>(newTiling,tile);
            addTileMotif(timo);
        }
    }
}

QVector<QPair<TileMotifPtr,TilePtr>> Prototype::pairModifiedTiles(const QVector<TileMotifPtr> & unmatchedOld, const QVector<TilePtr> & unmatchedNew)
{
    // greedy nearest-neighbor pairing by shape closeness. Same side count and
    // regularity are required for a pair to even be a candidate - a triangle should
    // never pair with a hexagon. Among candidates, the closest by scale/rotation wins.
    // Whatever's left over (a genuine count mismatch, or no eligible candidate at all)
    // falls through to replaceTiling()'s pass 3 as a real deletion/addition.
    //
    // This is greedy, not a true optimal assignment (Hungarian algorithm territory) -
    // a bad early pick could in principle starve a better one later - but the tile
    // counts involved here are always small enough (single digits) that greedy and
    // optimal are indistinguishable in practice.

    struct Candidate
    {
        int   oldIndex;
        int   newIndex;
        qreal distance;
    };

    QVector<Candidate> candidates;
    for (int oi = 0; oi < unmatchedOld.size(); oi++)
    {
        TilePtr oldTile = unmatchedOld[oi]->getTile();
        for (int ni = 0; ni < unmatchedNew.size(); ni++)
        {
            TilePtr newTile = unmatchedNew[ni];
            if (oldTile->numEdges() != newTile->numEdges())
                continue;
            if (oldTile->isRegular() != newTile->isRegular())
                continue;

            qreal distance = qAbs(oldTile->getScale() - newTile->getScale()) + qAbs(oldTile->getRotation() - newTile->getRotation());
            candidates.push_back({oi, ni, distance});
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate & a, const Candidate & b)
    {
        return a.distance < b.distance;
    });

    QVector<bool> oldUsed(unmatchedOld.size(), false);
    QVector<bool> newUsed(unmatchedNew.size(), false);
    QVector<QPair<TileMotifPtr,TilePtr>> pairs;

    for (const Candidate & c : std::as_const(candidates))
    {
        if (oldUsed[c.oldIndex] || newUsed[c.newIndex])
            continue;

        oldUsed[c.oldIndex] = true;
        newUsed[c.newIndex] = true;
        pairs.push_back(qMakePair(unmatchedOld[c.oldIndex], unmatchedNew[c.newIndex]));
    }

    return pairs;
}

void Prototype::exactReplaceTiling(const TilingPtr & newTiling)
{
    // exact replacement of tiling where tiles match
    _tiling = newTiling;

    _protoMap->clear();

    TileMotifSet usedElements;

    // match elements to tiles
    const QVector<TilePtr> uniqueTiles = newTiling->unit().getUniqueTiles();
    for (const TilePtr & newTile : std::as_const(uniqueTiles))
    {
        for (const TileMotifPtr & timo : std::as_const(_tileMotifSet))
        {
            if (usedElements.contains(timo))
            {
                continue;
            }
            if (*newTile == *timo->getTile())
            {
                // replace
                timo->exactReplaceTile(newTile);
                usedElements.push_back(timo);
                break;
            }
        }
    }
}

void Prototype::recreateFromTiling()
{
    // this handles additions and deletions of TileMotifs due to changes in TilingUnit

    int count = 0;
    QVector<TilePtr> uniqueTiles = _tiling->unit().getUniqueTiles();
    for (const TilePtr & tile : std::as_const(uniqueTiles))
    {
        if (++count > MAX_UNIQUE_TILE_INDEX)
            qWarning() << "Large number of unique tiles in tiling count:" << count;

        bool found = false;
        for (const TileMotifPtr & timo : std::as_const(getTileMotifs()))
        {
            if (timo->getTile() == tile)
            {
                found = true;
                if (!timo->validMotifRegularity())
                {
                    // radial/adaptive family no longer matches the tile's regularity
                    timo->recreateMotifWhenRgularityChanged();
                }
                else if (timo->getMotif()->getN() != tile->numEdges())
                {
                    // same family still fits, but the side count changed underneath it
                    // (e.g. 8 sides -> 16, still regular) - keep the motif type, adapt N
                    timo->recreateMotifFromChangedTile();
                }
                break;
            }
        }
        if (!found)
        {
            // creates a default TileMotif
            TileMotifPtr dep = make_shared<TileMotif>(_tiling,tile);
            addTileMotif(dep);
        }
    }

    TileMotifSet forDeletion;
    PlacedTileSet tileSet = _tiling->unit().getIncluded();

    for (const TileMotifPtr & timo : std::as_const(getTileMotifs()))
    {
        TilePtr tile = timo->getTile();
        if (!uniqueTiles.contains(tile))
        {
            forDeletion.push_back(timo);
        }
    }

    for (const TileMotifPtr & timo : forDeletion)
    {
        removeTileMotif(timo);
    }

    wipeoutProtoMap();
}

void Prototype::analyze(TilingPtr newTiling)
{
    QVector<TilePtr> tiles = newTiling->unit().getUniqueTiles();
    QString line = "TileMotifs: ";
    for (const TileMotifPtr & tm : std::as_const(_tileMotifSet))
    {
        TilePtr tile = tm->getTile();
        line += tile->summary();
    }
    qDebug().noquote() <<  line;
    line = "Tiles: ";
    for (const TilePtr & tile : std::as_const(tiles))
    {
        line += tile->summary();
    }
    qDebug().noquote() <<  line;
}

void Prototype::addTileMotif(const TileMotifPtr & tileMotif)
{
    _tileMotifSet.push_front(tileMotif);
}

void Prototype::removeTileMotif(const TileMotifPtr & tileMotif)
{
    _tileMotifSet.removeAll(tileMotif);
}

QString Prototype::info() const
{
    return QString("elements=%1 tiling=%2").arg(_tileMotifSet.size()).arg(_tiling->getVName().get());
}

TileMotifPtr Prototype::getTileMotif(int index)
{
    if (index < _tileMotifSet.size())
    {
        return _tileMotifSet[index];
    }

    //qDebug() << "Prototype::getTileMotif" << "- not found";
    TileMotifPtr del;
    return del;
}

bool  Prototype::containsTileMotif(TileMotifPtr tileMotif)
{
    if (!tileMotif)
        return false;

    for (const TileMotifPtr & timo : std::as_const(_tileMotifSet))
    {
        if (timo == tileMotif)
        {
            return true;
        }
    }
    return false;
}

TilePtr  Prototype::getTile(const MotifPtr & motif)
{
    for (const TileMotifPtr  & timo: std::as_const(_tileMotifSet))
    {
        if (timo->getMotif() == motif)
        {
            return timo->getTile();
        }
    }

    qWarning() << "TILE NOT FOUND";
    TilePtr f;
    return f;
}

MotifPtr Prototype::getMotif(const TilePtr & tile)
{
    for (const TileMotifPtr  & timo: std::as_const(_tileMotifSet))
    {
        if (timo->getTile() == tile)
        {
            return timo->getMotif();
        }
    }

    qWarning() << "MOTIF IS NOT FOUND";
    MotifPtr f;
    return f;
}

QList<TilePtr> Prototype::getTiles()
{
    QList<TilePtr> ql;
    for (const TileMotifPtr & timo: std::as_const(_tileMotifSet))
    {
        ql.append(timo->getTile());
    }
    return ql;
}

void Prototype::setCrop(CropPtr crop)
{
    _crop = crop;
    wipeoutProtoMap();
}

void Prototype::resetCrop()
{
    _crop.reset();
    wipeoutProtoMap();
}

void Prototype::walk()
{
    qDebug() << "num TileMotifs =" << _tileMotifSet.size();
    if (_tileMotifSet.size() == 0)
    {
        qWarning() << "There are no tile motifs in the prototype for tiling" << _tiling->getVName().get();
    }

    qDebug() << "start Prototype walk.... num motifs=" << _tileMotifSet.size();
    for (const TileMotifPtr  & timo : std::as_const(_tileMotifSet))
    {
        TilePtr tile   = timo->getTile();
        MotifPtr motif = timo->getMotif();
        qDebug().noquote() << "motif:" << motif->getMotifDesc() << " tile:" << tile->summary();
    }
    qDebug() << "....end Prototype walk";
}

void Prototype::dumpMotifs()
{
    for (const TileMotifPtr  & timo : std::as_const(_tileMotifSet))
    {
        MotifPtr  motif  = timo->getMotif();
        motif->dump();
    }
}

void Prototype::dump()
{
    Q_ASSERT(_protoMap);
    qDebug().noquote() << "Prototype:" << _protoMap->info();
}
