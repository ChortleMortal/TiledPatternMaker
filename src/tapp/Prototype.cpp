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
#include "geometry/fill_region.h"
#include "base/utilities.h"
#include "base/tpmsplash.h"
#include "panels/panel_status.h"
#include "panels/panel.h"

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
    refs++;
}

Prototype::~Prototype()
{
    refs--;
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "Prototype destructor";
    QMap<FeaturePtr, FigurePtr>::iterator it;
#if 1
    for (it = figures.begin(); it!= figures.end(); it++)
    {
        FigurePtr f = it.value();
        f.reset();
        FeaturePtr f2 = it.key();
        f2.reset();
    }
#endif
    figures.clear();
#endif
}

void Prototype::setTiling(TilingPtr t)
{
    tiling = t;
    resetProtoMap();
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
    for (int i=0; i < designElements.size(); i++)
    {
        if (designElements[i]->getFeature() == feature)
            return designElements[i];
    }

    qWarning() << "DESIGN ELEMENT NOT FOUND";
    return nullptr;
}

DesignElementPtr Prototype::getDesignElement(int index)
{
    if (index < designElements.size())
    {
        return designElements[index];
    }
    return nullptr;
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

FeaturePtr  Prototype::getFeature(FigurePtr figure )
{
    for (int i=0; i < designElements.size(); i++)
    {
        if (designElements[i]->getFigure() == figure)
            return designElements[i]->getFeature();
    }
    FeaturePtr f;
    qWarning() << "FEATURE IS NULL";
    return f;
}

FigurePtr Prototype::getFigure(FeaturePtr feature )
{
    for (int i=0; i < designElements.size(); i++)
    {
        if (designElements[i]->getFeature() == feature)
            return designElements[i]->getFigure();
    }
    FigurePtr f;
    qWarning() << "FIGURE IS NULL";
    return f;
}

QList<FeaturePtr> Prototype::getFeatures()
{
    QList<FeaturePtr> ql;
    for (int i=0; i < designElements.size(); i++)
    {
        ql.append(designElements[i]->getFeature());
    }
    return ql;
}

void Prototype::setFeaturesReversed(QVector<FeaturePtr> features)
{
    for (int i=0; i < designElements.size(); i++)
    {
        if (i < features.size())
        {
            designElements[i]->setFeature(features[features.size() - (i+1)]);
        }
    }
}

void Prototype::walk()
{
    qDebug() << "num design elements=" << designElements.size();
    if (designElements.size() == 0)
    {
        qWarning() << "There are no design elements in the prototype";
    }
    qDebug() << "num locations      =" << locations.size();

    qDebug() << "start Prototype walk.... num figures=" << designElements.size();
    for (auto it = designElements.begin(); it != designElements.end(); it++)
    {
        DesignElementPtr dep = *it;
        FeaturePtr feature = dep->getFeature();
        FigurePtr  figure  = dep->getFigure();
        qDebug().noquote() << "figure:" << figure->getFigureDesc() << " feature:" << feature->toString();
    }
    qDebug() << "....end Prototype walk";
}

// this is where the locations for the replicated styles are put
// each figure point is placed with a relative transform from the tiling when the map is constructed
void Prototype::receive(GeoGraphics *gg, int h, int v )
{
    Q_UNUSED(gg)
    //qDebug() << "fill qxy Prototype::receive:"  << h << v;
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


MapPtr Prototype::createProtoMap(bool showSplash)
{
    ControlPanel * panel = ControlPanel::getInstance();
    QString astring = QString("Constructing prototype map for tiling: %1").arg(tiling->getName());
    panel->showPanelStatus(astring);
#ifdef TPMSPLASH
    if (showSplash)
    {
        // showing splash causes the event queue to bw processed.  This can be quite unwanted.
        panel->showSplash(astring);
    }
#endif

    resetProtoMap();

    qDebug() << "PROTOTYPE::CONSTRUCT MAP" << Utils::addr(this);

    const bool debug = true;

    // Use FillRegion to get a list of translations for this tiling.
    fill(nullptr);
    qDebug() << "locations=" << locations.size();

    walk();

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.
    qDebug() << "designElements count=" << designElements.size();

    for (auto dep : designElements)
    {
        qDebug().noquote() << "design element:" << dep->toString();

        FeaturePtr feature   = dep->getFeature();
        FigurePtr figure     = dep->getFigure();
        MapPtr figmap        = figure->getFigureMap();

        qDebug().noquote() << figure->getFigureDesc();
        if (debug) figmap->verifyMap("figmap1");

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
        if (debug) transmap->verifyMap("transmap");

        // Now put all the translations together into a single map for this feature.
        MapPtr featuremap = make_shared<Map>("proto featuremap");
        featuremap->mergeSimpleMany(transmap, locations);
        if (debug) featuremap->verifyMap("featuremap");

        // And do a slow merge to add this map to the finished design.
        if (debug) protoMap->verifyMap("protoMap before Merge:");
        protoMap->mergeMap(featuremap);
        if (debug) protoMap->verifyMap("protoMap after Merge:");

        if (debug) qDebug() << "Constructed SUB map (ret): vertices=" << protoMap->numVertices()<< "edges=" << protoMap->numEdges();
        protoMap->analyzeVertices();
    }

    qDebug() << "Constructed complete map (ret): vertices=" << protoMap->numVertices() << "edges=" << protoMap->numEdges();

    panel->hidePanelStatus();
#ifdef TPMSPLASH
    if (showSplash)
    {
        panel->hideSplash();
    }
#endif

    return protoMap;
}
