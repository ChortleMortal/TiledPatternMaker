#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QString>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QTransform>

class GeoGraphics;

typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class TilingSelector>   TilingSelectorPtr;
typedef std::shared_ptr<class PlacedFeature>    PlacedFeaturePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;



enum eAddToTranslate
{
    ADTT_NOSTATE,
    ADTT_STARTED,
    ADTT_DRAGGING,
    ADTT_ENDED,
};

class TilingMaker;

class Measurement
{
public:
    Measurement();

    void    reset();

    void    setStart(QPointF spt);
    void    setEnd(QPointF spt);

    QPointF startW();
    QPointF endW();
    QPointF startS();
    QPointF endS();

    qreal   lenS();
    qreal   lenW();

    bool    active;

private:
    TilingMakerPtr tm;

    QPointF wStart;
    QPointF wEnd;
};

class TilingMouseAction
{
public:
    TilingMouseAction(TilingMaker * tm, TilingSelectorPtr sel, QPointF spt );
    virtual ~TilingMouseAction() {}
    virtual void updateDragging( QPointF spt );
    virtual void draw(GeoGraphics * g2d );
    virtual void endDragging( QPointF spt );

    QString            desc;

protected:
    QPointF            wLastDrag;   // screen point
    TilingSelectorPtr selection;
    QColor             drag_color;

    TilingMaker   * tm;        // DAC added

private:
};

typedef std::shared_ptr<TilingMouseAction> MouseActionPtr;

class MovePolygon : public TilingMouseAction
{
public:
    MovePolygon(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt);
    virtual void updateDragging( QPointF spt );
};

class CopyMovePolygon : public MovePolygon
{
public:
    CopyMovePolygon(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
    virtual void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class DrawTranslation : public TilingMouseAction
{
public:
    DrawTranslation(TilingMaker * tilingMaker,  TilingSelectorPtr sel, QPointF spt , QPen apen);
    void updateDragging( QPointF spt );
    void draw( GeoGraphics* g2d );
    void endDragging(QPointF spt);

    eAddToTranslate   state;
    QLineF vector;
    QPen apen;
};

class JoinEdge : public TilingMouseAction
{
public:
    JoinEdge(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
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
    JoinMidPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
};

class JoinPoint : public JoinEdge
{
public:
    JoinPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );

private:
    virtual bool snapTo(QPointF spt) override;
};

class CopyJoinEdge : public JoinEdge
{
public:
    CopyJoinEdge(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinMidPoint : public JoinMidPoint
{
public:
    CopyJoinMidPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinPoint : public JoinPoint
{
public:
    CopyJoinPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CreatePolygon : public TilingMouseAction
{
    typedef std::shared_ptr<class Grid> GridPtr;

public:
    CreatePolygon(TilingMaker * tilingMaker, QPointF spt );
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

private:
    void addVertex(QPointF wpt);
    QPointF underneath;

    GridPtr grid;
};

class Measure : public TilingMouseAction
{
public:
    Measure(TilingMaker * tilingMaker, QPointF spt, TilingSelectorPtr sel);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

protected:
    QLineF normalVectorA(QLineF line);
    QLineF normalVectorB(QLineF line);

private:
    Measurement  m;
    QLineF       sPerpLine; // perpendicular line
};

class Position : public TilingMouseAction
{
public:
    Position(TilingMaker * tilingMaker, QPointF spt);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );

private:
    QPointF spt;
};



class EditFeature : public TilingMouseAction
{
public:
    EditFeature(TilingMaker * tilingMaker,  TilingSelectorPtr sel, PlacedFeaturePtr pfp, QPointF spt );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    PlacedFeaturePtr pfp;
    int              vertexIndex;
};

class EditEdge : public TilingMouseAction
{
public:
    EditEdge(TilingMaker * tilingMaker,  TilingSelectorPtr sel, QPointF spt );
    void draw( GeoGraphics* g2d );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    QPointF start;
    EdgePtr edge;
    QLineF  perp;
    PlacedFeaturePtr pfp;
};

class TilingConstructionLine : public TilingMouseAction
{
public:
    TilingConstructionLine(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt);
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
