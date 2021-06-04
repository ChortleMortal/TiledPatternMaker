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

////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// PlacedFeatures that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#include <QtWidgets>
#include "tile/tiling.h"
#include "tile/backgroundimage.h"
#include "base/configuration.h"
#include "base/fileservices.h"
#include "base/misc.h"
#include "geometry/transform.h"
#include "panels/panel.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "base/mosaic_writer.h"
#include "tapp/explicit_figure.h"


const QString Tiling::defaultName = "The Unnamed";

int Tiling::refs = 0;

Tiling::Tiling()
{
    name        = defaultName;
    settings    = make_shared<ModelSettings>();
    settings->setSize(QSize(1500,1100));
    version     = -1;
    state       = EMPTY;
    refs++;
}

Tiling::Tiling(QString name, QPointF t1, QPointF t2)
{
    this->t1 = t1;
    this->t2 = t2;

    if (!name.isEmpty())
    {
        this->name = name;
    }
    else
    {
        name    = defaultName;
    }

    settings    = make_shared<ModelSettings>();
    settings->setSize(QSize(1500,1100));
    version      = -1;
    state        = EMPTY;
    refs++;
}

Tiling::Tiling(Tiling * other)
{
    for (auto it = other->placed_features.begin(); it != other->placed_features.end(); it++)
    {
        PlacedFeaturePtr pf = make_shared<PlacedFeature>(*it);
        placed_features.push_back(pf);
    }

    t1           = other->t1;
    t2           = other->t2;
    name         = other->name;
    desc         = other->desc;
    author       = other->author;
    settings     = other->settings;
    canvasXform  = other->canvasXform;
    version      = -1;
    state        = LOADED;
    refs++;
}

Tiling::~Tiling()
{
    placed_features.clear();
    refs--;
}

bool Tiling::isEmpty()
{
    if (name == "The Unnamed" && placed_features.isEmpty() && t1.isNull() && t2.isNull())
        return true;
    else
        return false;
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        for (auto pf : qAsConst(placed_features))
        {
            QPolygonF poly = pf->getPlacedPolygon();

            for (auto pf2 : qAsConst(placed_features))
            {
                if (pf2 == pf) continue;

                QPolygonF poly2 = pf2->getPlacedPolygon();
                if (poly2.intersects(poly))
                {
                    QPolygonF p3 = poly2.intersected(poly);
                    if (!p3.isEmpty())
                    {
                        //qDebug() << "overlapping";
                        intrinsicOverlaps.set(true);
                    }
                }
            }
        }
        intrinsicOverlaps.set(false);
    }
    return (intrinsicOverlaps.get() == Tristate::True);
}

bool Tiling::hasTiledOverlaps()
{
    if (tiledOveraps.get() == Tristate::Unknown)
    {
        MapPtr map = createProtoMap();
        tiledOveraps.set(map->hasIntersectingEdges());
    }
    return (tiledOveraps.get() == Tristate::True);
}

MapPtr Tiling::createMap()
{
    MapPtr map = make_shared<Map>("tiling map");
    for (auto pfp : placed_features)
    {
        EdgePoly poly = pfp->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("feature",poly);
        map->mergeMap(emap);
    }

    return map;
}

MapPtr Tiling::createFilledMap()
{
    MapPtr map = make_shared<Map>("tiling map");

    QVector<QTransform> translations = getFillTranslations();

    MapPtr tilingMap = createMap();

    for (auto transform : translations)
    {
        MapPtr m  = tilingMap->recreate();
        m->transformMap(transform);
        map->mergeMap(m);
    }
    return map;
}

MapPtr Tiling::createProtoMap()
{
    // Thios builds a prototype using explicit fewature figures and generates its map
    Prototype proto(shared_from_this());

    QVector<FeaturePtr> uniqueFeatures = getUniqueFeatures();

    for (auto feature :  uniqueFeatures)
    {
        EdgePoly & ep = feature->getEdgePoly();
        MapPtr     fm = make_shared<Map>("feature map",ep);
        FigurePtr fig = make_shared<ExplicitFigure>(fm,FIG_TYPE_EXPLICIT_FEATURE,feature->numSides());
        DesignElementPtr  dep = make_shared<DesignElement>(feature, fig);
        proto.addElement(dep);
    }

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.

    QVector<QTransform> translations = getFillTranslations();

    MapPtr testmap = make_shared<Map>("testmap");

    for (auto dep : qAsConst(proto.getDesignElements()))
    {
        FeaturePtr feature   = dep->getFeature();
        FigurePtr figure     = dep->getFigure();
        MapPtr figmap        = figure->getFigureMap();

        QVector<QTransform> subT;
        for (auto placed_feature : placed_features)
        {
            FeaturePtr       f  = placed_feature->getFeature();
            if (f == feature)
            {
                QTransform t = placed_feature->getTransform();
                subT.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given feature into a map.
        MapPtr transmap = make_shared<Map>("proto transmap");
        if (hasIntrinsicOverlaps())
            transmap->mergeMany(figmap, subT);
        else
            transmap->mergeSimpleMany(figmap, subT);

        // Now put all the translations together into a single map for this feature.
        MapPtr featuremap = make_shared<Map>("proto featuremap");
        featuremap->mergeSimpleMany(transmap, translations);

        // And do a slow merge to add this map to the finished design.
        testmap->mergeMap(featuremap);
    }
    return testmap;
}


QVector<QTransform> Tiling::getFillTranslations()
{
    QVector<QTransform> translations;
    FillData & fd = settings->getFillData();
    int minX,minY,maxX,maxY;
    fd.get(minX,maxX,minY,maxY);
    for (int h = minX; h <= maxX; h++)
    {
        for (int v = minY; v <= maxY; v++)
        {
            QPointF pt   = (t1 * static_cast<qreal>(h)) + (t2 * static_cast<qreal>(v));
            QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
            translations << T;
        }
    }
    return translations;
}

// Feature management.
// Added feature are embedded into a PlacedFeature.
void Tiling::setPlacedFeatures(QVector<PlacedFeaturePtr> & features)
{
    placed_features = features;
    setState(MODIFED);
}

void Tiling::add(const PlacedFeaturePtr pf )
{
    placed_features.push_back(pf);
    setState(MODIFED);
}

void Tiling::add(FeaturePtr f, QTransform  T)
{
    add(make_shared<PlacedFeature>(f, T));
    setState(MODIFED);
}

void Tiling::remove(PlacedFeaturePtr pf)
{
    placed_features.removeOne(pf);
    setState(MODIFED);
}

int Tiling::getVersion()
{
    return version;
}

void Tiling::setVersion(int ver)
{
    version = ver;
}

Tiling::eTilingState Tiling::getState()
{
    return state;
}

void Tiling::setState(eTilingState state)
{
    this->state = state;
}

QVector<FeaturePtr> Tiling::getUniqueFeatures()
{
    UniqueQVector<FeaturePtr> fs;

    for (auto pfp : qAsConst(placed_features))
    {
        FeaturePtr fp = pfp->getFeature();
        fs.push_back(fp);
    }

    return static_cast<QVector<FeaturePtr>>(fs);
}

int Tiling::numPlacements(FeaturePtr fp)
{
    int count = 0;
    for (auto pfp : qAsConst(placed_features))
    {
        FeaturePtr fp2 = pfp->getFeature();
        if (fp2 == fp)
        {
            count++;
        }
    }

    return count;
}

// Regroup features by their translation so that we write each feature only once.
FeatureGroup Tiling::regroupFeatures()
{
    FeatureGroup featureGroup;
    for(auto placedFeature : qAsConst(placed_features))
    {
        FeaturePtr feature = placedFeature->getFeature();
        if (featureGroup.containsFeature(feature))
        {
            QVector<PlacedFeaturePtr>  & v = featureGroup.getPlacements(feature);
            v.push_back(placedFeature);
        }
        else
        {
            QVector<PlacedFeaturePtr> v;
            v.push_back(placedFeature);
            featureGroup.push_back(qMakePair(feature,v));
        }
    }
    return featureGroup;
}

QString Tiling::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "tiling=" << name  << "t1=" << t1 << "t2=" << t2 << "num features=" << placed_features.size();
#if 0
    for (int i=0; i < placed_features.size(); i++)
    {
        PlacedFeaturePtr  pf = placed_features[i];
        deb << endl << "poly" << i << "points=" << pf->getFeature()->numPoints()  << "transform= " << Transform::toInfoString(pf->getTransform()) << endl;
        deb << pf->getFeaturePolygon();
    }
#endif
    return astring;
}

///
/// class Feature Group
///

bool FeatureGroup::containsFeature(FeaturePtr fp)
{
    for (auto& apair : *this)
    {
        FeaturePtr f = apair.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}


QVector<PlacedFeaturePtr> & FeatureGroup::getPlacements(FeaturePtr fp)
{

    Q_ASSERT(containsFeature(fp));

    for (auto& apair : *this)
    {
        FeaturePtr f = apair.first;
        if (f == fp)
        {
            return apair.second;
        }
    }

    qWarning("should never reach here");
    static QVector <PlacedFeaturePtr> v;
    return v;
}



