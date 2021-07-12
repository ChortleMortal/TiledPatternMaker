/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tapp/prototype.h"
#include "tile/tiling.h"
#include "tile/feature.h"
#include "tapp/figure.h"
#include "tapp/design_element.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "panels/panel.h"
#include "tile/placed_feature.h"
#include "base/border.h"

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
    protoMap = std::make_shared<Map>("proto map");

    panel = ControlPanel::getInstance();

    refs++;
}

Prototype::~Prototype()
{
    //qDebug() << "Prototype destructor";
    designElements.clear();
    resetProtoMap();
    protoMap.reset();
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

void Prototype::setTiling(TilingPtr newTiling)
{
    // replace the features (keeping the figures) where possible
    // make sure every feature in the new tiling has a design element
    // delete redundant design element

    Q_ASSERT(newTiling);
    analyze(newTiling);

    tiling = newTiling;

    resetProtoMap();
    //protoMap->wipeout();

    QVector<FeaturePtr>       unusedFeatures;
    QVector<DesignElementPtr> usedElements;

    // match elements to features
    QVector<FeaturePtr> uniqueFeatures = newTiling->getUniqueFeatures();
    for (auto newFeature : uniqueFeatures)
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
    for (auto feature : unusedFeatures)
    {
        DesignElementPtr del = std::make_shared<DesignElement>(feature);
        addElement(del);
    }
}

void Prototype::analyze(TilingPtr newTiling)
{
    QVector<FeaturePtr> features = newTiling->getUniqueFeatures();
    QString line = "elements: ";
    for (auto designElement : qAsConst(designElements))
    {
        FeaturePtr feature = designElement->getFeature();
        line += feature->summary();
    }
    qDebug().noquote() <<  line;
    line = "features: ";
    for (auto feature : features)
    {
        line += feature->summary();
    }
    qDebug().noquote() <<  line;
}

void Prototype::addElement(DesignElementPtr element )
{
    designElements.push_front(element);
}

void Prototype::removeElement(DesignElementPtr element)
{
    designElements.removeAll(element);
}

QString Prototype::getInfo() const
{
    return QString("elements=%1 tiling=%2").arg(designElements.size()).arg(tiling->getName());
}

DesignElementPtr Prototype::getDesignElement(FeaturePtr feature )
{
    for (auto designElement : qAsConst(designElements))
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

QTransform Prototype::getTransform(int index)
{
    if (index < translations.size())
    {
        return translations[index];
    }

    QTransform t;
    return t;
}

FeaturePtr  Prototype::getFeature(FigurePtr figure)
{
    for (auto designElement: qAsConst(designElements))
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

FigurePtr Prototype::getFigure(FeaturePtr feature)
{
    for (auto designElement: qAsConst(designElements))
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
    for (auto designElement: qAsConst(designElements))
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
    qDebug() << "num translations      =" << translations.size();

    qDebug() << "start Prototype walk.... num figures=" << designElements.size();
    for (auto element : qAsConst(designElements))
    {
        FeaturePtr feature = element->getFeature();
        FigurePtr  figure  = element->getFigure();
        qDebug().noquote() << "figure:" << figure->getFigureDesc() << " feature:" << feature->summary();
    }
    qDebug() << "....end Prototype walk";
}

// this is where the locations for the replicated styles are put
// each figure point is placed with a relative transform from the tiling when the map is constructed
void Prototype::receive(GeoGraphics *gg, int h, int v )
{
    Q_UNUSED(gg)
    //qDebug() << "Prototype::receive:"  << h << v;
    QPointF pt   = (tiling->getTrans1() * static_cast<qreal>(h)) + (tiling->getTrans2() * static_cast<qreal>(v));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
    translations << T;
}

void Prototype::resetProtoMap()
{
    translations.clear();
    if (protoMap)
    {
        protoMap->wipeout();
    }
}

MapPtr Prototype::getProtoMap()
{
    if (translations.isEmpty() || protoMap->isEmpty())
    {
        createProtoMap();
    }
    return protoMap;
}


MapPtr Prototype::createProtoMap()
{
    qDebug() << "PROTOTYPE CONSTRUCT MAP" << this;
    QString astring = QString("Constructing prototype map for tiling: %1").arg(tiling->getName());
    panel->showPanelStatus(astring);
#ifdef TPMSPLASH
    panel->splashTiling(astring);
#endif

    resetProtoMap();

    // Use FillRegion to get a list of translations for this tiling.
    fill(nullptr);

    //walk();

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.

    for (auto dep : qAsConst(designElements))
    {
        FeaturePtr feature   = dep->getFeature();
        FigurePtr figure     = dep->getFigure();

        qDebug() << "protomap start: figure - " << figure->getFigureDesc();

        MapPtr figmap        = figure->getFigureMap();

        QVector<QTransform> subT;
        for (auto placedFeature : tiling->getPlacedFeatures())
        {
            FeaturePtr f  = placedFeature->getFeature();
            if (f == feature)
            {
                QTransform t = placedFeature->getTransform();
                subT.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given feature into a map.
        MapPtr transmap = std::make_shared<Map>("proto transmap");
        transmap->mergeSimpleMany(figmap, subT);

        // Now put all the translations together into a single map for this feature.
        MapPtr featuremap = std::make_shared<Map>("proto featuremap");
        featuremap->mergeSimpleMany(transmap, translations);

        // And do a slow merge to add this map to the finished design.
        protoMap->mergeMap(featuremap);

        qDebug() << "protomap end: figure - " << figure->getFigureDesc();
    }

    qDebug() << "PROTOTYPE merged";

    bool finished = false;
    if (_border)
    {
        InnerBorder * ib = dynamic_cast<InnerBorder*>(_border.get());
        if (ib)
        {
            CropPtr crop = ib->getInnerBoundary();
            if (crop && (crop->getState() == CROP_BORDER_DEFINED  || crop->getState() == CROP_DEFINED))
            {
                QRectF rect = crop->getRect();
                protoMap->addCropBorder(rect);
                protoMap->removeOutisde(rect);
                qDebug() << "BORDER merged";
                finished = true;
            }
        }
    }

    if (!finished)
    {
        protoMap->sortEdges();
        protoMap->sortVertices();
        protoMap->buildNeighbours();
    }

    protoMap->verify();

    qDebug() << "PROTOTYPE COMPLETED MAP: vertices=" << protoMap->numVertices() << "edges=" << protoMap->numEdges();

    panel->hidePanelStatus();
#ifdef TPMSPLASH
    panel->removeSplashTiling();
#endif

    return protoMap;
}

void Prototype::setBorder(BorderPtr border)
{
    _border = border;
    if (border)
        qDebug() << "Prototype::setBorder" << border.get() << border->getType();
    resetProtoMap();
}
