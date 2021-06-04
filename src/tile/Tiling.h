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
#include "base/shared.h"
#include "base/model_settings.h"
#include "geometry/xform.h"

#define MAX_UNIQUE_FEATURE_INDEX 7


class FeatureGroup : public  QVector<QPair<FeaturePtr,QVector<PlacedFeaturePtr>>>
{
public:
    bool containsFeature(FeaturePtr fp);
    QVector<PlacedFeaturePtr> & getPlacements(FeaturePtr fp);
};

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public std::enable_shared_from_this<Tiling>
{
public:

    enum eTilingState
    {
        EMPTY,
        LOADED,
        MODIFED
    };

    Tiling();
    Tiling(QString name, QPointF t1, QPointF t2);
    Tiling(Tiling * other);
    ~Tiling();

    bool        isEmpty();

    bool        hasIntrinsicOverlaps();
    bool        hasTiledOverlaps();

    QString     getName()        const { return name; }
    QString     getDescription() const { return desc; }
    QString     getAuthor()      const { return author; }

    void        setName(QString n)               { name = n; }
    void        setDescription(QString descrip ) { desc = descrip; }
    void        setAuthor(QString auth)          { author = auth; }

    // Feature management.
    // Added features are embedded into a PlacedFeature.
    void        add(const PlacedFeaturePtr pf );
    void        add(FeaturePtr f, QTransform T );
    void        remove(PlacedFeaturePtr pf);

    void                      setPlacedFeatures(QVector<PlacedFeaturePtr> & features);
    const PlacedFeaturePtr    getPlacedFeature(int idx)   { return placed_features[idx]; }
    const QVector<PlacedFeaturePtr> & getPlacedFeatures() { return placed_features; }
    int                       countPlacedFeatures() const { return placed_features.size(); }
    int                       numPlacements(FeaturePtr fp);
    QVector<FeaturePtr>       getUniqueFeatures();

    void        setTrans1(QPointF pt) { t1=pt; }
    void        setTrans2(QPointF pt) { t2=pt; }
    QPointF     getTrans1() const { return t1; }
    QPointF     getTrans2() const { return t2; }
    QVector<QTransform> getFillTranslations();

    FeatureGroup regroupFeatures();      // the map was deadly, it reordered

    QString     dump() const;

    void        setCanvasXform(Xform & xf) { canvasXform = xf; }
    Xform &     getCanvasXform()           { return canvasXform; }

    int         getVersion();
    void        setVersion(int ver);

    eTilingState getState();
    void         setState(eTilingState state);

    // settings
    void        setSettings(ModelSettingsPtr settings) { this->settings = settings; }
    void        setSize(QSize sz)             { settings->setSize(sz); }
    void        setBackground(BkgdImgPtr bip) { settings->setBkgdImage(bip); }
    void        setFillData(FillData & fdata) { settings->setFillData(fdata); }

    ModelSettingsPtr    getSettings()  { return settings; }
    QSize               getSize()      { return settings->getSize(); }
    BkgdImgPtr          getBackground(){ return settings->getBkgdImage(); }
    const FillData    & getFillData()  { return settings->getFillData(); }

    // map operations
    MapPtr  createMap();
    MapPtr  createFilledMap();
    MapPtr  createProtoMap();

    static const QString defaultName;
    static int  refs;

protected:

private:
    int             version;
    QString         name;
    QString         desc;
    QString         author;
    ModelSettingsPtr settings;
    eTilingState    state;
    Tristate        intrinsicOverlaps;
    Tristate        tiledOveraps;
    QPointF         t1;
    QPointF         t2;
    Xform           canvasXform;
    QVector<PlacedFeaturePtr> placed_features;
};

#endif

