#pragma once
#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QString>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QTransform>
#include "geometry/measurement.h"

class GeoGraphics;
class TilingMaker;
class TilingMakerView;

typedef std::shared_ptr<class TileSelector>     TileSelectorPtr;
typedef std::shared_ptr<class PlacedTile>       PlacedTilePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;

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
    TilingMouseAction(TileSelectorPtr sel, QPointF spt);
    virtual ~TilingMouseAction() {}
    virtual void updateDragging( QPointF spt );
    virtual void draw(GeoGraphics * g2d );
    virtual void endDragging( QPointF spt );

    QString            desc;

protected:
    void               flash(QColor color);

    QPointF            wLastDrag;   // screen point
    TileSelectorPtr    selection;
    QColor             drag_color;

    TilingMakerView   * tilingMakerView;
    TilingMaker       * tilingMaker;

private:
};

typedef std::shared_ptr<TilingMouseAction> MouseActionPtr;

class MovePolygon : public TilingMouseAction
{
public:
    MovePolygon( TileSelectorPtr sel, QPointF spt);
    virtual void updateDragging( QPointF spt );
};

class CopyMovePolygon : public MovePolygon
{
public:
    CopyMovePolygon( TileSelectorPtr sel, QPointF spt );
    virtual void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class DrawTranslation : public TilingMouseAction
{
public:
    DrawTranslation(TileSelectorPtr sel, QPointF spt , QPen apen);
    void updateDragging( QPointF spt );
    void draw( GeoGraphics* g2d );
    void endDragging(QPointF spt);

    eAddToTranslate state;
    QLineF          vector;
    QPen            apen;
};

class JoinEdge : public TilingMouseAction
{
public:
    JoinEdge(TileSelectorPtr sel, QPointF spt);
    void updateDragging( QPointF spt );
    void endDragging( QPointF spt );

protected:
    virtual bool snapTo(QPointF spt);
    QTransform   matchLineSegment(QPointF p, QPointF q);
    QTransform   matchTwoSegments(QPointF p1, QPointF q1, QPointF p2, QPointF q2);

    bool snapped;
};

class JoinMidPoint : public JoinEdge
{
public:
    JoinMidPoint( TileSelectorPtr sel, QPointF spt );

protected:
    virtual bool snapTo(QPointF spt) override;
};

class JoinPoint : public JoinEdge
{
public:
    JoinPoint( TileSelectorPtr sel, QPointF spt );

protected:
    virtual bool snapTo(QPointF spt) override;
};

class CopyJoinEdge : public JoinEdge
{
public:
    CopyJoinEdge( TileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinMidPoint : public JoinMidPoint
{
public:
    CopyJoinMidPoint(TileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinPoint : public JoinPoint
{
public:
    CopyJoinPoint(TileSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CreatePolygon : public TilingMouseAction
{
public:
    CreatePolygon( QPointF spt );
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

private:
    void addVertex(QPointF wpt);
    QPointF underneath;

    class GridView * gridView;
};

class Measure : public TilingMouseAction
{
public:
    Measure(QPointF spt, TileSelectorPtr sel);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

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
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );

private:
    QPointF spt;
};



class EditTile : public TilingMouseAction
{
public:
    EditTile(TileSelectorPtr sel, PlacedTilePtr pfp, QPointF spt );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    PlacedTilePtr pfp;
    int           vertexIndex;
};

class EditEdge : public TilingMouseAction
{
public:
    EditEdge(TileSelectorPtr sel, QPointF spt );
    void draw( GeoGraphics* g2d );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    QPointF         start;
    EdgePtr         edge;
    QLineF          perp;
    PlacedTilePtr   pfp;
};

class TilingConstructionLine : public TilingMouseAction
{
public:
    TilingConstructionLine(TileSelectorPtr sel, QPointF spt);
    ~TilingConstructionLine() override;

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(GeoGraphics * painter) override;

protected:
    QPointF * start;
    QPointF * end;
    VertexPtr startv;
    VertexPtr endv;

    QVector<QPointF> intersectPoints;
};
#endif
