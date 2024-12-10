#include "gui/top/splash_screen.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map.h"
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
    _protoMap      = make_shared<Map>("ProtoMap ");
    viewController = Sys::viewController;
    cleanseLevel   = 0;
    cleanseSensitivity = 0;
    refs++;
}


 // for use only by PrototypeMaker
Prototype::Prototype(TilingPtr t)
{
    Q_ASSERT(t);
    _tiling        = t;
    _protoMap      = make_shared<Map>("ProtoMap ");
    viewController = Sys::viewController;
    cleanseLevel   = 0;
    cleanseSensitivity = 0;
    refs++;
}

Prototype::Prototype(MapPtr map)
{
    // This is a special case used by the map editor where the map is already prepared
    _protoMap      = map;
    viewController = Sys::viewController;
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

    if (_designElements.size() != other._designElements.size())
        return false;

    for (int i=0; i <  _designElements.size(); i++)
    {
        DesignElementPtr ele  = _designElements[i];
        DesignElementPtr eleo = other._designElements[i];

        if ( !(ele->getTile()->equals(eleo->getTile())))
            return  false;

        if ( !(ele->getMotif()->equals(eleo->getMotif())))
            return  false;
    }
    return  true;
}

void Prototype::wipeoutProtoMap()
{
    _dcel.reset();                  // dcel is subordinate so must be erased too.
    _protoMap->clear();
    Q_ASSERT(_protoMap->isEmpty());
}

MapPtr Prototype::getProtoMap(bool splash)
{
    Q_ASSERT(_protoMap);
    if (_protoMap->isEmpty())
    {
        createProtoMap(splash);
    }
    return _protoMap;
}

void Prototype::createProtoMap(bool splash)
{
    QMutexLocker locker(&protoMutex);

    wipeoutProtoMap();

    if (_designElements.size() == 0)
    {
        return;
    }

    if (splash && viewController->splashCanPaint())
    {
        Sys::splash->display("Creating Prototype Map...");
    }

    AQElapsedTimer timer;

    qDebug() << "PROTOTYPE CONSTRUCT MAP";
    QString astring = QString("Constructing prototype map for tiling: %1").arg(_tiling->getName().get());
    qDebug().noquote() << astring;

    _createMap();

    qDebug().noquote() << "PROTOTYPE COMPLETED MAP:" << _protoMap->info();
    qDebug().noquote() << "Prototype construction" << timer.getElapsed() << "seconds";

    if (splash && viewController->splashCanPaint())
    {
        Sys::splash->remove();
    }
}

void Prototype::_createMap()
{
    Q_ASSERT(_tiling);

    // Use FillRegion to get a list of translations for this tiling.
    // Note that the fill data could be from the mosaic rather than the tiling
    // this is where the locations for the replicated styles are put
    // each motif point is placed with a relative transform from the tiling when the map is constructed

    MosaicPtr mosaic = wMosaic.lock();
    FillData fillData;
    if (mosaic)
        fillData = mosaic->getCanvasSettings().getFillData();
    else
        fillData = _tiling->getCanvasSettings().getFillData();
    FillRegion flood(_tiling.get(),fillData);
    Placements fillPlacements = flood.getPlacements(Sys::config->repeatMode);

    if (_designElements.size() == 1 && fillPlacements.size() == 1 && fillPlacements[0].isIdentity())
    {
        auto tile = _designElements[0]->getTile();
        if (_tiling->getPlacements(tile).size() == 0)
        {
            // This is a special edge case and seems a kludge, fighting the system.
            // With more thought a better implementation of this idea could be made
            auto motif = _designElements[0]->getMotif();
            _protoMap  = motif->getMotifMap();
            qDebug() << "PROTOTYPE is motif map";
        }
    }

    if (_protoMap->isEmpty() && (_designElements.size() > 0))
    {
        buildMotifMaps();

        for (const auto & designElement : std::as_const(_designElements))
        {
            // Now, for each different tile, build a submap corresponding
            // to all translations of that tile.
            TilePtr tile              = designElement->getTile();
            Placements tilePlacements = _tiling->getPlacements(tile);
            if (!tilePlacements.size()) tilePlacements.push_back(QTransform());   // dummy tilings have no placements

            MotifPtr motif  = designElement->getMotif();
            motif->dump();
            MapPtr motifMap = motif->getMotifMap();
            if (!motifMap)
            {
                qWarning("empty motif map");
                motifMap = make_shared<Map>("Kludge map");
            }
            qDebug().noquote() << "MOTIF:" << motif->getMotifDesc() << motifMap->summary() << "Tile-sides:" << tile->numSides();

            // Within a single translational unit, assemble the different
            // transformed figures corresponding to the given feature into a map.
            MapPtr unitMap =  make_shared<Map>("proto unit map");
            unitMap->mergeMany(motifMap, tilePlacements);
            qDebug().noquote() << "unit map" << unitMap->summary();

            // Now put all the translations together into a single map for this feature.
            MapPtr tileMap = make_shared<Map>("proto tile map");
            tileMap->mergeMany(unitMap, fillPlacements);
            qDebug().noquote() << "tile map" << tileMap->summary();

            // And do a slow merge to add this map to the finished design.
            qDebug().noquote() << "proto map before:" << _protoMap->summary();
            _protoMap->mergeMap(tileMap);
            qDebug().noquote() << "proto map end: figure - " << motif->getMotifDesc() << _protoMap->summary();
        }
        if (!_protoMap->isEmpty()) qDebug() << "PROTOTYPE merged";
    }

    if (_crop)
    {
        if (_crop->getCropType() == CROP_RECTANGLE)
        {
            const QRectF & rect = _crop->getRect();
            if (rect.isValid())
            {
                if (_crop->getEmbed())
                {
                    _protoMap->embedCrop(rect);
                }
                if (_crop->getApply())
                {
                    _protoMap->cropOutside(rect);
                }
                qDebug() << "Crop-rect merged";
            }
        }
        else if (_crop->getCropType() == CROP_CIRCLE)
        {
            const Circle & circle = _crop->getCircle();
            if (_crop->getEmbed())
            {
                _protoMap->embedCrop(circle);
            }
            if (_crop->getApply())
            {
                _protoMap->cropOutside(circle);
            }
            qDebug() << "Crop-circle merged";
        }
        else if (_crop->getCropType() == CROP_POLYGON)
        {
            QPolygonF poly = _crop->getAPolygon().get();
            if (_crop->getEmbed())
            {
                _protoMap->embedCrop(poly);
            }
            if (_crop->getApply())
            {
                _protoMap->cropOutside(poly);
            }
            qDebug() << "Crop-poly merged";
        }
    }

    if (!_protoMap->isEmpty())
    {
        qDebug() << "cleanse level=" << cleanseLevel;
        if (cleanseLevel)
        {
            qDebug() << "cleanse level (hex)" << Qt::hex << cleanseLevel;
            qDebug().noquote() << "pre proto map cleanse:" << _protoMap->summary();
            _protoMap->cleanse(cleanseLevel,cleanseSensitivity);
            qDebug().noquote() << "post proto map cleanse:" << _protoMap->summary();
        }
    }

    _protoMap->verifyAndFix(Sys::config->verifyProtos);
}

void Prototype::resetMotifMaps()
{
    for (const auto & del : std::as_const(_designElements))
    {
        auto motif = del->getMotif();
        motif->resetMotifMap();
    }
}

void Prototype::buildMotifMaps()
{
    // The InferredMotif type is dependant on its surrounding motifs.
    // So these need to be prepared before the prototype map
    // can be built

    // pass 1 - build all motif maps
    int num_inferred = 0;
    for (const auto & del : std::as_const(_designElements))
    {
        auto motif = del->getMotif();
        if (motif->getMotifType() == MOTIF_TYPE_INFERRED) num_inferred++;
        motif->cleanExtenders();
        motif->dump();
        motif->buildMotifMap();
    }

    // pass 2 - rebuild InferredMotifs
    for (int i=0; i < num_inferred; i++)
    {
        for (const auto & del : std::as_const(_designElements))
        {
            auto motif = del->getMotif();
            if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
            {
                motif->resetMotifMap();
                motif->dump();
                motif->buildMotifMap();
            }
        }
    }
}

DCELPtr Prototype::getDCEL()
{
    if (!_dcel)
    {
        QMutexLocker locker(&dcelMutex);

        auto protomap = getProtoMap();
        _dcel = std::make_shared<DCEL>(protomap.get());
        protomap->setDerivedDCEL(_dcel);
    }
    return _dcel;
}

void Prototype::replaceTiling(const TilingPtr & newTiling)
{
    // replace the tiles (keeping the motifs) where possible
    // make sure every tile in the new tiling has a design element
    // delete redundant design element

    Q_ASSERT(newTiling);
    analyze(newTiling);

    _tiling = newTiling;

    _protoMap->clear();

    QVector<TilePtr>          unusedTiles;
    QVector<DesignElementPtr> usedElements;

    // match elements to tiles
    const QVector<TilePtr> uniqueTiles = newTiling->getUniqueTiles();
    for (auto & newTile : std::as_const(uniqueTiles))
    {
        bool used = false;
        for (auto & del : std::as_const(_designElements))
        {
            if (usedElements.contains(del))
            {
                continue;
            }
            if (newTile->equals(del->getTile()))
            {
                // replace
                del->replaceTile(newTile);
                usedElements.push_back(del);
                used = true;
                break;
            }
        }
        if (!used)
        {
            unusedTiles.push_back(newTile);
        }
    }

    // remove unused elements
    QVector<DesignElementPtr> unusedElements;
    for (const auto & element : std::as_const(_designElements))
    {
        if (!usedElements.contains(element))
        {
            unusedElements.push_back(element);
        }
    }

    for (const auto & element : std::as_const(unusedElements))
    {
        removeDesignElement(element);
    }

    // create new elements
    for (const auto & tile : std::as_const(unusedTiles))
    {
        DesignElementPtr del = make_shared<DesignElement>(tile);
        addDesignElement(del);
    }
}

void Prototype::analyze(TilingPtr newTiling)
{
    QVector<TilePtr> tiles = newTiling->getUniqueTiles();
    QString line = "elements: ";
    for (const auto & designElement : std::as_const(_designElements))
    {
        TilePtr tile = designElement->getTile();
        line += tile->summary();
    }
    qDebug().noquote() <<  line;
    line = "tiles: ";
    for (const auto & tile : std::as_const(tiles))
    {
        line += tile->summary();
    }
    qDebug().noquote() <<  line;
}

void Prototype::addDesignElement(const DesignElementPtr & element )
{
    _designElements.push_front(element);
}

void Prototype::removeDesignElement(const DesignElementPtr & element)
{
    _designElements.removeAll(element);
}

QString Prototype::info() const
{
    return QString("elements=%1 tiling=%2").arg(_designElements.size()).arg(_tiling->getName().get());
}

#if 0
DesignElementPtr Prototype::getDesignElement(const TilePtr & tile)
{
    for (const auto & designElement : std::as_const(_designElements))
    {
        if (designElement->getTile() == tile)
        {
            return designElement;
        }
    }

    //qDebug() << __FUNCTION__ "- not found";
    DesignElementPtr del;
    return del;
}
#endif

DesignElementPtr Prototype::getDesignElement(int index)
{
    if (index < _designElements.size())
    {
        return _designElements[index];
    }

    //qDebug() << __FUNCTION__ "- not found";
    DesignElementPtr del;
    return del;
}

bool  Prototype::containsDesignElement(DesignElementPtr del)
{
    if (!del)
        return false;

    for (auto & desel : std::as_const(_designElements))
    {
        if (desel == del)
        {
            return true;
        }
    }
    return false;
}

TilePtr  Prototype::getTile(const MotifPtr & motif)
{
    for (const auto & designElement: std::as_const(_designElements))
    {
        if (designElement->getMotif() == motif)
        {
            return designElement->getTile();
        }
    }

    qWarning() << "TILE NOT FOUND";
    TilePtr f;
    return f;
}

MotifPtr Prototype::getMotif(const TilePtr & tile)
{
    for (const auto & designElement: std::as_const(_designElements))
    {
        if (designElement->getTile() == tile)
        {
            return designElement->getMotif();
        }
    }

    qWarning() << "MOTIF IS NOT FOUND";
    MotifPtr f;
    return f;
}

QList<TilePtr> Prototype::getTiles()
{
    QList<TilePtr> ql;
    for (const auto & designElement: std::as_const(_designElements))
    {
        ql.append(designElement->getTile());
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
    qDebug() << "num design elements=" << _designElements.size();
    if (_designElements.size() == 0)
    {
        qWarning() << "There are no design elements in the prototype for tiling" << _tiling->getName().get();
    }

    qDebug() << "start Prototype walk.... num motifs=" << _designElements.size();
    for (const auto & element : std::as_const(_designElements))
    {
        TilePtr tile = element->getTile();
        MotifPtr  motif  = element->getMotif();
        qDebug().noquote() << "motif:" << motif->getMotifDesc() << " tile:" << tile->summary();
    }
    qDebug() << "....end Prototype walk";
}

void Prototype::dumpMotifs()
{
    for (const auto & element : std::as_const(_designElements))
    {
        MotifPtr  motif  = element->getMotif();
        motif->dump();
    }
}

void Prototype::dump()
{
    Q_ASSERT(_protoMap);
    qDebug().noquote() << "Prototype:" << _protoMap->info();
}
