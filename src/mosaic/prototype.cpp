#include "mosaic/prototype.h"
#include "figures/figure.h"
#include "geometry/crop.h"
#include "geometry/fill_region.h"
#include "geometry/map.h"
#include "misc/timers.h"
#include "mosaic/design_element.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

int Prototype::refs = 0;

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from features to figures.  Prototype knows how to turn
// this information into a finished design, returned as a Map.

Prototype::Prototype(TilingPtr t)
{
    Q_ASSERT(t);
    tiling = t;
    protoMap = make_shared<Map>("proto map");

    panel     = ControlPanel::getInstance();
    vcontrol  = ViewControl::getInstance();

    refs++;
}

Prototype::Prototype(MapPtr map)
{
    // This is a special case used by the map editor where the map is already prepared
    protoMap = map;

    vcontrol  = ViewControl::getInstance();
    panel = ControlPanel::getInstance();

    refs++;
}

Prototype::~Prototype()
{
    //qDebug() << "Prototype destructor";
#ifdef EXPLICIT_DESTRUCTOR
    designElements.clear();
    resetProtoMap();
    protoMap.reset();
#endif
    refs--;
}

bool Prototype::operator==(const Prototype & other)
{
    if (tiling != other.tiling)
        return false;

    if (designElements.size() != other.designElements.size())
        return false;

    for (int i=0; i <  designElements.size(); i++)
    {
        DesignElementPtr ele  = designElements[i];
        DesignElementPtr eleo = other.designElements[i];

        if ( !(ele->getFeature()->equals(eleo->getFeature())))
            return  false;

        if ( !(ele->getFigure()->equals(eleo->getFigure())))
            return  false;
    }
    return  true;
}

void Prototype::setTiling(const TilingPtr & newTiling)
{
    // replace the features (keeping the figures) where possible
    // make sure every feature in the new tiling has a design element
    // delete redundant design element

    Q_ASSERT(newTiling);
    analyze(newTiling);

    tiling = newTiling;

    wipeoutProtoMap();

    QVector<FeaturePtr>       unusedFeatures;
    QVector<DesignElementPtr> usedElements;

    // match elements to features
    const QVector<FeaturePtr> uniqueFeatures = newTiling->getUniqueFeatures();
    for (auto & newFeature : qAsConst(uniqueFeatures))
    {
        bool used = false;
        for (auto element : qAsConst(designElements))
        {
            if (usedElements.contains(element))
            {
                continue;
            }
            if (newFeature->equals(element->getFeature()))
            {
                // replace
                element->replaceFeature(newFeature);
                usedElements.push_back(element);
                used = true;
                break;
            }
        }
        if (!used)
        {
            unusedFeatures.push_back(newFeature);
        }
    }

    // remove unused elements
    QVector<DesignElementPtr> unusedElements;
    for (auto& element : qAsConst(designElements))
    {
        if (!usedElements.contains(element))
        {
            unusedElements.push_back(element);
        }
    }

    for (auto& element : unusedElements)
    {
        removeElement(element);
    }

    // create new elementsN
    for (const auto & feature : qAsConst(unusedFeatures))
    {
        DesignElementPtr del = make_shared<DesignElement>(feature);
        addElement(del);
    }
}

void Prototype::analyze(TilingPtr newTiling)
{
    QVector<FeaturePtr> features = newTiling->getUniqueFeatures();
    QString line = "elements: ";
    for (const auto & designElement : qAsConst(designElements))
    {
        FeaturePtr feature = designElement->getFeature();
        line += feature->summary();
    }
    qDebug().noquote() <<  line;
    line = "features: ";
    for (const auto & feature : qAsConst(features))
    {
        line += feature->summary();
    }
    qDebug().noquote() <<  line;
}

void Prototype::addElement(const DesignElementPtr & element )
{
    designElements.push_front(element);
}

void Prototype::removeElement(const DesignElementPtr & element)
{
    designElements.removeAll(element);
}

QString Prototype::getInfo() const
{
    return QString("elements=%1 tiling=%2").arg(designElements.size()).arg(tiling->getName());
}

DesignElementPtr Prototype::getDesignElement(const FeaturePtr & feature)
{
    for (const auto & designElement : qAsConst(designElements))
    {
        if (designElement->getFeature() == feature)
        {
            return designElement;
        }
    }

    qWarning() << "DESIGN ELEMENT NOT FOUND";
    DesignElementPtr del;
    return del;
}

DesignElementPtr Prototype::getDesignElement(int index)
{
    if (index < designElements.size())
    {
        return designElements[index];
    }

    qWarning() << "DESIGN ELEMENT NOT FOUND";
    DesignElementPtr del;
    return del;
}

FeaturePtr  Prototype::getFeature(const FigurePtr & figure)
{
    for (const auto & designElement: qAsConst(designElements))
    {
        if (designElement->getFigure() == figure)
        {
            return designElement->getFeature();
        }
    }

    qWarning() << "FEATURE NOT FOUND";
    FeaturePtr f;
    return f;
}

FigurePtr Prototype::getFigure(const FeaturePtr & feature)
{
    for (const auto & designElement: qAsConst(designElements))
    {
        if (designElement->getFeature() == feature)
        {
            return designElement->getFigure();
        }
    }

    qWarning() << "FIGURE IS NOT FOUND";
    FigurePtr f;
    return f;
}

QList<FeaturePtr> Prototype::getFeatures()
{
    QList<FeaturePtr> ql;
    for (const auto & designElement: qAsConst(designElements))
    {
        ql.append(designElement->getFeature());
    }
    return ql;
}

void Prototype::walk()
{
    qDebug() << "num design elements=" << designElements.size();
    if (designElements.size() == 0)
    {
        qWarning() << "There are no design elements in the prototype for tiling" << tiling->getName();
    }

    qDebug() << "start Prototype walk.... num figures=" << designElements.size();
    for (const auto & element : qAsConst(designElements))
    {
        FeaturePtr feature = element->getFeature();
        FigurePtr  figure  = element->getFigure();
        qDebug().noquote() << "figure:" << figure->getFigureDesc() << " feature:" << feature->summary();
    }
    qDebug() << "....end Prototype walk";
}

void Prototype::dump()
{
    auto map = getExistingProtoMap();
    if (map)
    {
        qDebug().noquote() << "Prototype:" << map->summary2();
    }
    else
    {
        qDebug() << "Prototype:" << "no map";
    }
}

void Prototype::wipeoutProtoMap()
{
    if (protoMap)
    {
        protoMap->wipeout();
    }
}

MapPtr Prototype::getProtoMap()
{
    if (protoMap->isEmpty())
    {
        createProtoMap();
    }
    return protoMap;
}

void Prototype::createProtoMap()
{
    AQElapsedTimer timer;
    qDebug() << "PROTOTYPE CONSTRUCT MAP" << this;
    QString astring = QString("Constructing prototype map for tiling: %1").arg(tiling->getName());
    panel->pushPanelStatus(astring);
    panel->splashTiling(astring);
    qDebug().noquote() << astring;

    wipeoutProtoMap();

    // Use FillRegion to get a list of translations for this tiling.
    // Note that the fill data could be from the mosaic rather than the tiling
    // this is where the locations for the replicated styles are put
    // each figure point is placed with a relative transform from the tiling when the map is constructed
    FillRegion flood(tiling,vcontrol->getFillData());
    QVector<QTransform> translations = flood.getTransforms();

    if (designElements.size() == 1 && translations.size() == 1 && translations[0].isIdentity())
    {
        auto feature = designElements[0]->getFeature();
        auto placements = tiling->getPlacements(feature);
        if (placements.size() == 0)
        {
            auto figure = designElements[0]->getFigure();
            protoMap = figure->getFigureMap();
            qDebug() << "PROTOTYPE  is figure map";
            // This is a special edge case and seems a kludge, fighting the system.
            // With more thought a better implementation of this idea could be made
        }
    }

    if (protoMap->isEmpty())
    {
        for (const auto & designElement : designElements)
        {
            // Now, for each different feature, build a submap corresponding
            // to all translations of that feature.
            FeaturePtr feature        = designElement->getFeature();
            FigurePtr  figure         = designElement->getFigure();

            QVector<QTransform> subT  = tiling->getPlacements(feature);
            if (!subT.size())
                subT.push_back(QTransform());   // dummy tilings have no placements
            qDebug() << "transforms =" << subT.size();

            MapPtr     figmap         = figure->getFigureMap();
            qDebug().noquote() << "protomap figure :" << figure->getFigureDesc() << figmap->summary();

            // Within a single translational unit, assemble the different
            // transformed figures corresponding to the given feature into a map.
            MapPtr unitmap = make_shared<Map>("proto transmap");
            unitmap->mergeMany(figmap, subT);
            qDebug().noquote() << "unitmap" << unitmap->summary2();

            // Now put all the translations together into a single map for this feature.
            MapPtr featuremap = make_shared<Map>("proto featuremap");
            qDebug() << "translations =" << translations.size();
            featuremap->mergeMany(unitmap, translations);
            qDebug().noquote() << "featuremap" << featuremap->summary2();

            // And do a slow merge to add this map to the finished design.
            qDebug().noquote() << "protomap before:" << protoMap->summary2();
            protoMap->mergeMap(featuremap);

            qDebug().noquote() << "protomap end: figure - " << figure->getFigureDesc() << protoMap->summary2();
        }
        qDebug() << "PROTOTYPE merged";
    }

    if (_crop)
    {
        if (_crop->getCropType() == CROP_RECTANGLE)
        {
            QRectF rect = _crop->getRect();
            if (rect.isValid())
            {
                if (_crop->isEmbedded())
                {
                    protoMap->embedCrop(rect);
                }
                if (_crop->isApplied())
                {
                    protoMap->cropOutside(rect);
                }
                qDebug() << "Crop-rect merged";
            }
        }
        else if (_crop->getCropType() == CROP_CIRCLE)
        {
            auto circle = _crop->getCircle();
            if (_crop->isEmbedded())
            {
                protoMap->embedCrop(circle);
            }
            if (_crop->isApplied())
            {
                protoMap->cropOutside(circle);
            }
            qDebug() << "Crop-circle merged";
        }
    }

    protoMap->verifyAndFix(Configuration::getInstance()->verifyProtos);

    qDebug() << "PROTOTYPE COMPLETED MAP: vertices=" << protoMap->numVertices() << "edges=" << protoMap->numEdges();

    panel->popPanelStatus();
    panel->removeSplashTiling();

    qDebug().noquote() << "Prototype construction" << timer.getElapsed() << "seconds";
}

void Prototype::initCrop(CropPtr crop)
{
    _crop = crop;
}

void Prototype::resetCrop(CropPtr crop)
{
    _crop = crop;
    qDebug() << "Prototype::resetCrop" << crop.get();
    wipeoutProtoMap();
    // dont create the prototype here by calling getProtoMap() - it is too soon and creates problems on loading mosaics
}
