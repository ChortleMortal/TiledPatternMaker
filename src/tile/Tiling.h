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

#ifndef TILING
#define TILING

#include <QtCore>
#include "tile/PlacedFeature.h"
#include "base/shared.h"
#include "tilemaker.h"
#include "base/pugixml.hpp"

class FeatureGroup : public  QList<QPair<FeaturePtr,QList<PlacedFeaturePtr>>>
{
public:
    bool containsFeature(FeaturePtr fp);
    QList<PlacedFeaturePtr> & getPlacements(FeaturePtr fp);
};

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : protected TileMaker
{
public:
    Tiling();
    Tiling(QString name, QPointF t1, QPointF t2);
    Tiling(Tiling * other);
    ~Tiling();

    TilingPtr   readTiling(QString file);
    TilingPtr   readTiling(QTextStream & st);
    TilingPtr   readTilingXML(QString file);
    TilingPtr   readTilingXML(pugi::xml_node & tiling_node);

    bool        writeTilingXML();
    void        writeTilingXML(QTextStream & out);     // also called when writing styles

    QString     getName()        const { return name; }
    QString     getDescription() const { return desc; }
    QString     getAuthor()      const { return author; }

    void        setName(QString n)               { name = n; }
    void        setDescription(QString descrip ) { desc = descrip; }
    void        setAuthor(QString auth)          { author = auth; }

    // Feature management.
    // Added features are embedded into a PlacedFeature.
    void        add(const PlacedFeaturePtr pf );
    void        add(FeaturePtr f, Transform & T );
    void        remove(PlacedFeaturePtr pf);

    PlacedFeaturePtr          getPlacedFeature(int idx) { return placed_features[idx]; }
    QList<PlacedFeaturePtr> & getPlacedFeatures()       { return placed_features; }
    int                       countPlacedFeatures() const { return placed_features.size(); }

    QList<FeaturePtr> getUniqueFeatures();    // unique features

    void        setTrans1(QPointF pt) { t1=pt; }
    void        setTrans2(QPointF pt) { t2=pt; }
    QPointF     getTrans1() const { return t1; }
    QPointF     getTrans2() const { return t2; }

    FeatureGroup regroupFeatures();      // the map was deadly, it reordered

    FillData    getFillData() { return fillData;}
    void        setFillData(FillData & fdata) {fillData.set(fdata);}

    void        dump() const;

protected:
    QString     toString() { return name; }

    QString     readQuotedString(QTextStream &str);
    QPointF     getPoint(QString txt);
    Transform   getTransform(QString txt);

private:
    FillData fillData;

    QPointF	t1;
    QPointF	t2;

    QList<PlacedFeaturePtr>	placed_features;

    QString	name;
    QString	desc;
    QString author;
};

#endif

