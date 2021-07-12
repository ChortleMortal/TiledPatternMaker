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
// Selection.java
//
// A helper struct that holds information about a selection (see FeatureView).
// Probably not used in the applet at all.

#ifndef FEATURE_SELECTION_H
#define FEATURE_SELECTION_H

#include <QPointF>
#include <QLineF>
#include <QString>
#include <QPolygonF>

typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class PlacedFeature>    PlacedFeaturePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;

#define E2STR(x) #x

enum eSelection
{
    INTERIOR,
    EDGE,
    VERTEX,
    MID_POINT,
    ARC_POINT,
    FEAT_CENTER,
    SCREEN_POINT
    };

static QString strTiliingSelection[] =
{
    E2STR(INTERIOR),
    E2STR(EDGE),
    E2STR(VERTEX),
    E2STR(MID_POINT),
    E2STR(ARC_POINT),
    E2STR(FEAT_CENTER),
    E2STR(SCREEN_POINT)
};

class TilingMakerView;

class TilingSelector
{
public:
    eSelection       getType()          { return type; }
    QString          getTypeString()    { return strTiliingSelection[type]; }

    //QTransform       getTransform()     { return pfp->getTransform(); }
    //FeaturePtr       getFeature()       { return pfp->getFeature(); }

    EdgePtr          getModelEdge()     { return edge; }
    QLineF           getModelLine();
    QPointF          getModelPoint()    { return pt; }
    QPolygonF        getModelPolygon();

    PlacedFeaturePtr getPlacedFeature() { return pfp; }
    QLineF           getPlacedLine();
    QPointF          getPlacedPoint();
    QPolygonF        getPlacedPolygon();
    EdgePtr          getPlacedEdge();

protected:
    TilingSelector(QPointF pt);
    TilingSelector(PlacedFeaturePtr pfp);
    TilingSelector(PlacedFeaturePtr pfp, QPointF pt);
    TilingSelector(PlacedFeaturePtr pfp, EdgePtr edge);
    TilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt);

    eSelection       type;

private:
    PlacedFeaturePtr pfp;
    QPointF          pt;
    EdgePtr          edge;
};


class InteriorTilingSelector : public TilingSelector
{
public:
    InteriorTilingSelector(PlacedFeaturePtr pfp);
};

class EdgeTilingSelector : public TilingSelector
{
public:
    EdgeTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge);
};

class VertexTilingSelector : public TilingSelector
{
public:
    VertexTilingSelector(PlacedFeaturePtr pfp, QPointF pt);
};

class MidPointTilingSelector : public TilingSelector
{
public:
    MidPointTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt);
};

class ArcPointTilingSelector : public TilingSelector
{
public:
    ArcPointTilingSelector(PlacedFeaturePtr pfp, EdgePtr edge, QPointF pt);
};

class CenterTilingSelector : public TilingSelector
{
public:
    CenterTilingSelector(PlacedFeaturePtr, QPointF pt);
};

class ScreenTilingSelector : public TilingSelector
{
public:
    ScreenTilingSelector(QPointF pt);
};


#endif

