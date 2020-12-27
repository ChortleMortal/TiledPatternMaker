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


const QString Tiling::defaultName = "The Unnamed";

int Tiling::refs = 0;

Tiling::Tiling()
{
    name       = defaultName;
    settings   = make_shared<ModelSettings>();
    settings->setSize(QSize(1500,1100));
    version    = -1;
    state      = TILING_EMPTY;
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
        name   = defaultName;
    }

    settings   = make_shared<ModelSettings>();
    settings->setSize(QSize(1500,1100));
    version     = -1;
    state       = TILING_EMPTY;
    refs++;
}

Tiling::Tiling(Tiling * other)
{
    for (auto it = other->placed_features.begin(); it != other->placed_features.end(); it++)
    {
        PlacedFeaturePtr pf = make_shared<PlacedFeature>(*it);
        placed_features.push_back(pf);
    }

    t1          = other->t1;
    t2          = other->t2;
    name        = other->name;
    desc        = other->desc;
    author      = other->author;
    settings    = other->settings;
    canvasXform = other->canvasXform;
    version     = -1;
    state       = TILING_LOADED;
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
    {
        return true;
    }
    else
    {
        return false;
    }
}


// Feature management.
// Added feature are embedded into a PlacedFeature.
void Tiling::setPlacedFeatures(QVector<PlacedFeaturePtr> & features)
{
    placed_features = features;
    setState(TILING_MODIFED);
}

void Tiling::add(const PlacedFeaturePtr pf )
{
    placed_features.push_back(pf);
    setState(TILING_MODIFED);
}

void Tiling::add(FeaturePtr f, QTransform  T)
{
    add(make_shared<PlacedFeature>(f, T));
    setState(TILING_MODIFED);
}

void Tiling::remove(PlacedFeaturePtr pf)
{
    placed_features.removeOne(pf);
    setState(TILING_MODIFED);
}

int Tiling::getVersion()
{
    return version;
}

void Tiling::setVersion(int ver)
{
    version = ver;
}

eTilingState Tiling::getState()
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

    deb << "name=" << name  << "t1=" << t1 << "t2=" << t2 << "num features=" << placed_features.size() << endl;
    for (int i=0; i < placed_features.size(); i++)
    {
        PlacedFeaturePtr  pf = placed_features[i];
        deb << "poly" << i << "points=" << pf->getFeature()->numPoints()  << "transform= " << Transform::toInfoString(pf->getTransform()) << endl;
        deb << pf->getFeaturePolygon() << endl;
    }

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



