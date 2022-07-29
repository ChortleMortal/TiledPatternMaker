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

