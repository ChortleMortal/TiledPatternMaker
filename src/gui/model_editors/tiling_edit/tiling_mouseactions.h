#pragma once
#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QString>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QTransform>
#include "sys/geometry/measurement.h"

class GeoGraphics;
class TilingMaker;
class TilingMakerView;

typedef std::shared_ptr<class PlacedTileSelector> PlacedTileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>         PlacedTilePtr;
typedef std::shared_ptr<class Edge>               EdgePtr;
typedef std::shared_ptr<class Vertex>             VertexPtr;
typedef std::weak_ptr<class PlacedTile>          wPlacedTilePtr;

enum eAddToTranslate
{
    ADTT_NOSTATE,
    ADTT_STARTED,
    ADTT_DRAGGING,
    ADTT_ENDED,
};

class TilingMouseAction
{
public:
    TilingMouseAction(PlacedTileSelectorPtr sel, QPointF spt);
    virtual ~TilingMouseAction() {}

    virtual void updateDragging( QPointF spt );
    virtual void draw(GeoGraphics * g2d );
    virtual void endDragging( QPointF spt );

    QString            desc;

protected:
    QPointF            wLastDrag;   // screen point
    PlacedTileSelectorPtr    selection;
    QColor             drag_color;

    TilingMakerView   * tilingMakerView;
    TilingMaker       * tilingMaker;

private:
};

typedef std::shared_ptr<TilingMouseAction> MouseActionPtr;

class MovePolygon : public TilingMouseAction
{
public:
    MovePolygon( PlacedTileSelectorPtr sel, QPointF spt);
    virtual void updateDragging( QPointF spt ) override;
};

class CopyMovePolygon : public MovePolygon
{
public:
    CopyMovePolygon( PlacedTileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt ) override;

private:
    QTransform initial_transform;
};

class DrawTranslation : public TilingMouseAction
{
public:
    DrawTranslation(PlacedTileSelectorPtr sel, QPointF spt , QPen apen);
    void updateDragging( QPointF spt ) override;
    void draw( GeoGraphics* g2d ) override;
    void endDragging(QPointF spt) override;

    eAddToTranslate state;
    QLineF          vector;
    QPen            apen;
    QPointF         mOrigin;
};

class JoinEdge : public TilingMouseAction
{
public:
    JoinEdge(PlacedTileSelectorPtr sel, QPointF spt);
    void updateDragging( QPointF spt ) override;
    void endDragging( QPointF spt ) override;

protected:
    virtual bool snapTo(QPointF spt);
    QTransform   matchLineSegment(QPointF p, QPointF q);
    QTransform   matchTwoSegments(QPointF p1, QPointF q1, QPointF p2, QPointF q2);

    bool snapped;
};

class JoinMidPoint : public JoinEdge
{
public:
    JoinMidPoint( PlacedTileSelectorPtr sel, QPointF spt );

protected:
    bool snapTo(QPointF spt) override;
};

class JoinPoint : public JoinEdge
{
public:
    JoinPoint( PlacedTileSelectorPtr sel, QPointF spt );

protected:
    bool snapTo(QPointF spt) override;
};

class CopyJoinEdge : public JoinEdge
{
public:
    CopyJoinEdge( PlacedTileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt ) override;

private:
    QTransform initial_transform;
};

class CopyJoinMidPoint : public JoinMidPoint
{
public:
    CopyJoinMidPoint(PlacedTileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt ) override;

private:
    QTransform initial_transform;
};

class CopyJoinPoint : public JoinPoint
{
public:
    CopyJoinPoint(PlacedTileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt ) override;

private:
    QTransform initial_transform;
};

class CreatePolygon : public TilingMouseAction
{
public:
    CreatePolygon( QPointF spt );
    void updateDragging(QPointF spt ) override;
    void draw( GeoGraphics * g2d ) override;
    void endDragging(QPointF spt ) override;

private:
    void addVertex(QPointF wpt);
    QPointF underneath;

    class GridView * gridView;
};

class Measure : public TilingMouseAction
{
public:
    Measure(QPointF spt, PlacedTileSelectorPtr sel);
    void updateDragging(QPointF spt ) override;
    void draw( GeoGraphics * g2d ) override;
    void endDragging(QPointF spt )override;

protected:
    QLineF normalVectorA(QLineF line);
    QLineF normalVectorB(QLineF line);

private:
    Measurement  * m;
    QLineF       sPerpLine; // perpendicular line
};

class Position : public TilingMouseAction
{
public:
    Position(QPointF spt);
    void updateDragging(QPointF spt ) override;
    void draw( GeoGraphics * g2d ) override;

private:
    QPointF spt;
};



class EditTile : public TilingMouseAction
{
public:
    EditTile(PlacedTileSelectorPtr sel, PlacedTilePtr pfp, QPointF spt );
    void updateDragging( QPointF spt ) override;
    void endDragging(QPointF spt) override;

private:
    PlacedTilePtr pfp;
    int           vertexIndex;
};

class EditEdge : public TilingMouseAction
{
public:
    EditEdge(PlacedTileSelectorPtr sel, QPointF spt );
    void draw( GeoGraphics* g2d ) override;
    void updateDragging( QPointF spt ) override;
    void endDragging(QPointF spt) override;

private:
    QPointF         start;
    EdgePtr         edge;
    QLineF          perp;
    PlacedTilePtr   pfp;
};

class TilingConstructionLine : public TilingMouseAction
{
public:
    TilingConstructionLine(PlacedTileSelectorPtr sel, QPointF spt);
    ~TilingConstructionLine() override;

    void updateDragging(QPointF spt) override;
    void endDragging( QPointF spt) override;
    void draw(GeoGraphics * painter) override;

protected:
    QPointF * start;
    QPointF * end;
    VertexPtr startv;
    VertexPtr endv;

    QVector<QPointF> intersectPoints;
};
#endif
