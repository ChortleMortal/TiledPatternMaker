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
#include "base/tpmsplash.h"
#include "base/utilities.h"
#include "geometry/fill_region.h"
#include "geometry/map_cleanser.h"
#include "panels/panel.h"
#include "panels/panel_status.h"
#include "tile/placed_feature.h"

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

    panel = ControlPanel::getInstance();

    refs++;
}

Prototype::~Prototype()
{
    refs--;
    designElements.clear();
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

    // create new elements
    for (auto feature : unusedFeatures)
    {
        DesignElementPtr del = make_shared<DesignElement>(feature);
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
    if (index < locations.size())
    {
        return locations[index];
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
    qDebug() << "num locations      =" << locations.size();

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
    locations << T;
}

void Prototype::resetProtoMap()
{
    locations.clear();
    if (protoMap)
    {
        protoMap->wipeout();
    }
}

MapPtr Prototype::getProtoMap()
{
    if (locations.isEmpty() || protoMap->isEmpty())
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
     panel->showSplash(astring);
#endif

    resetProtoMap();

    // Use FillRegion to get a list of translations for this tiling.
    fill(nullptr);
    qDebug() << "locations=" << locations.size();

    walk();

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.
    qDebug() << "designElements count=" << designElements.size();
    int count = 0;
    for (auto dep : qAsConst(designElements))
    {
        qDebug().noquote() << "merging design element:" << count << "into prototype:" << dep->toString();

        FeaturePtr feature   = dep->getFeature();
        FigurePtr figure     = dep->getFigure();
        qDebug().noquote() << "figure" << figure->getFigureDesc();
        MapPtr figmap        = figure->getFigureMap();

        QVector<QTransform> subT;
        for (auto it2 = tiling->getPlacedFeatures().begin(); it2 != tiling->getPlacedFeatures().end(); it2++)
        {
            PlacedFeaturePtr pf = *it2;
            FeaturePtr       f  = pf->getFeature();
            if (f == feature)
            {
                QTransform t = pf->getTransform();
                subT.push_back(t);
            }
        }

        int sz = subT.size();
        if (sz)
            qDebug() << "subT count =" << sz;
        else
            qWarning() << "subT count = 0";

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given feature into a map.
        MapPtr transmap = make_shared<Map>("proto transmap");
        transmap->mergeSimpleMany(figmap, subT);

#if 0
            transmap->buildNeighbours();
            MapCleanser cleant(transmap);
            cleant.verifyMap("transmap");
#endif

        // Now put all the translations together into a single map for this feature.
        MapPtr featuremap = make_shared<Map>("proto featuremap");
        featuremap->mergeSimpleMany(transmap, locations);

#if 0
            featuremap->buildNeighbours();
            MapCleanser cleanf(featuremap);
            cleanf.verifyMap("featuremap");
#endif

        // And do a slow merge to add this map to the finished design.
        protoMap->mergeMap(featuremap);

#if 0
            protoMap->buildNeighbours();
            MapCleanser cleanp(protoMap);
            cleanp.verifyMap("pmap");
#endif

        qDebug().noquote() << "merged design element:" << count++;
    }

    qDebug() << "PROTOTYPE merged";

    protoMap->sortEdges();
    protoMap->sortVertices();
    protoMap->buildNeighbours();

    protoMap->verifyMap("protoMap");

    qDebug() << "PROTOTYPE COMPLETED MAP: vertices=" << protoMap->numVertices() << "edges=" << protoMap->numEdges();

    panel->hidePanelStatus();
#ifdef TPMSPLASH
     panel->hideSplash();
#endif

    return protoMap;
}
