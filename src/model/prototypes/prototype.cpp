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
    viewController  = Sys::viewController;
    cleanseLevel    = 0;
    distort         = false;
    cleanseSensitivity = 0;
    refs++;
}

 // for use only by PrototypeMaker
Prototype::Prototype(TilingPtr t)
{
    Q_ASSERT(t);
    _tiling         = t;
    _protoMap       = make_shared<Map>("ProtoMap ");
    viewController  = Sys::viewController;
    cleanseLevel    = 0;
    distort         = false;
    cleanseSensitivity = 0;
    refs++;
}

Prototype::Prototype(MapPtr map)
{
    // This is a special case used by the map editor where the map is already prepared
    _protoMap      = map;
    viewController = Sys::viewController;
    cleanseLevel   = 0;
    distort        = false;
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
        DELPtr ele  = _designElements[i];
        DELPtr eleo = other._designElements[i];

        if (ele->getTile() != eleo->getTile())
            return  false;

        if (ele->getMotif() != eleo->getMotif())
            return  false;
    }
    return  true;
}

void Prototype::wipeoutProtoMap()
{
    _DCEL.reset();                  // dcel is subordinate so must be erased too.
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

    Q_ASSERT(_protoMap);
    Q_ASSERT(_protoMap->isEmpty());

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
    QString astring = QString("Constructing prototype map for tiling: %1").arg(_tiling->getVName().get());
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
    Q_ASSERT(_protoMap->isEmpty());

    // Use FillRegion to get a list of translations for this tiling.
    // Note that the fill data could be from the mosaic rather than the tiling
    // this is where the locations for the replicated styles are put
    // each motif point is placed with a relative transform from the tiling when the map is constructed

    MosaicPtr mosaic = wMosaic.lock();

    FillData fillData;
    if (mosaic)
        fillData = mosaic->getCanvasSettings().getFillData();
    else
        fillData = _tiling->hdr().getCanvasSettings().getFillData();
    FillRegion flood(_tiling.get(),fillData);
    Placements fillPlacements = flood.getPlacements(Sys::config->repeatMode);

    if (_designElements.size() == 1 && fillPlacements.size() == 1 && fillPlacements[0].isIdentity())
    {
        auto tile = _designElements[0]->getTile();
        if (_tiling->unit().getPlacements(tile).size() == 0)
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

        buildPrototypeMap(fillPlacements);

        if (!_protoMap->isEmpty())
            qDebug() << "PROTOTYPE merged";
        else
            qDebug() << "PROTOTYPE empty";
    }

    if (distort && !distortionTransform.isIdentity())
    {
        qDebug() << "Prototype using distortion x" << distortionTransform.m11() << "y" << distortionTransform.m22();
        _protoMap->transform(distortionTransform);
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

void Prototype::resetMotifMaps()
{
    for (auto & del : _designElements)
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
    for (auto & del : _designElements)
    {
        auto motif = del->getMotif();
        if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
            num_inferred++;
        motif->cleanExtenders();
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
                motif->buildMotifMap();
            }
        }
    }
}

void Prototype::buildPrototypeMap(Placements & fillPlacements)
{
    for (auto & designElement : _designElements)
    {
        TilePtr tile              = designElement->getTile();
        Placements tilePlacements = _tiling->unit().getPlacements(tile);
        if (!tilePlacements.size())
            tilePlacements.push_back(QTransform());   // dummy tilings have no placements

        MotifPtr motif  = designElement->getMotif();
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
    if (!_DCEL)
    {
        QMutexLocker locker(&dcelMutex);

        auto protomap = getProtoMap();
        auto dcel     = std::make_shared<DCEL>(protomap.get());
        if (dcel->build())
        {
            _DCEL = dcel;
            protomap->setDerivedDCEL(dcel);
        }
    }
    return _DCEL;
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
    QVector<DELPtr> usedElements;

    // match elements to tiles
    const QVector<TilePtr> uniqueTiles = newTiling->unit().getUniqueTiles();
    for (auto & newTile : std::as_const(uniqueTiles))
    {
        bool used = false;
        for (auto & del : std::as_const(_designElements))
        {
            if (usedElements.contains(del))
            {
                continue;
            }
            if (newTile == del->getTile())
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
    QVector<DELPtr> unusedElements;
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
        DELPtr del = make_shared<DesignElement>(newTiling,tile);
        addDesignElement(del);
    }
}

void Prototype::exactReplaceTiling(const TilingPtr & newTiling)
{
    // exact replacement of tiling where tiles match
    _tiling = newTiling;

    _protoMap->clear();

    QVector<DELPtr> usedElements;

    // match elements to tiles
    const QVector<TilePtr> uniqueTiles = newTiling->unit().getUniqueTiles();
    for (auto & newTile : std::as_const(uniqueTiles))
    {
        for (auto & del : std::as_const(_designElements))
        {
            if (usedElements.contains(del))
            {
                continue;
            }
            if (newTile == del->getTile())
            {
                // replace
                del->exactReplaceTile(newTile);
                usedElements.push_back(del);
                break;
            }
        }
    }
}

void Prototype::analyze(TilingPtr newTiling)
{
    QVector<TilePtr> tiles = newTiling->unit().getUniqueTiles();
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

void Prototype::addDesignElement(const DELPtr & element )
{
    _designElements.push_front(element);
}

void Prototype::removeDesignElement(const DELPtr & element)
{
    _designElements.removeAll(element);
}

QString Prototype::info() const
{
    return QString("elements=%1 tiling=%2").arg(_designElements.size()).arg(_tiling->getVName().get());
}


DELPtr Prototype::getDesignElement(int index)
{
    if (index < _designElements.size())
    {
        return _designElements[index];
    }

    //qDebug() << "Prototype::getDesignElement" << "- not found";
    DELPtr del;
    return del;
}

bool  Prototype::containsDesignElement(DELPtr del)
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
        qWarning() << "There are no design elements in the prototype for tiling" << _tiling->getVName().get();
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
