#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QtWidgets>
#include "base/shared.h"
#include "makers/tiling_maker/feature_selection.h"
#include "base/geo_graphics.h"

enum eTMMouseMode
{
    TM_NO_MOUSE_MODE,
    TM_COPY_MODE,
    TM_DELETE_MODE,
    TM_TRANSLATION_VECTOR_MODE,
    TM_DRAW_POLY_MODE,
    TM_INCLUSION_MODE,
    TM_POSITION_MODE,
    TM_MEASURE_MODE,
    TM_BKGD_SKEW_MODE,
    TM_EDIT_FEATURE_MODE,
    TM_EDGE_CURVE_MODE,
    TM_MIRROR_X_MODE,
    TM_MIRROR_Y_MODE,
    TM_CONSTRUCTION_LINES
};

static QString sTMMouseMode[] =
{
    E2STR(TM_NO_MOUSE_MODE),
    E2STR(TM_COPY_MODE),
    E2STR(TM_DELETE_MODE),
    E2STR(TM_TRANSLATION_VECTOR_MODE),
    E2STR(TM_DRAW_POLY_MODE),
    E2STR(TM_INCLUSION_MODE),
    E2STR(TM_POSITION_MODE),
    E2STR(TM_MEASURE_MODE),
    E2STR(TM_BKGD_SKEW_MODE),
    E2STR(TM_EDIT_FEATURE_MODE),
    E2STR(TM_EDGE_CURVE_MODE),
    E2STR(TM_MIRROR_X_MODE),
    E2STR(TM_MIRROR_Y_MODE),
    E2STR(TM_CONSTRUCTION_LINES)
};

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

typedef shared_ptr<TilingMouseAction> MouseActionPtr;

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
public:
    CreatePolygon(TilingMaker * tilingMaker, QPointF spt );
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

private:
    void addVertex(QPointF wpt);
    QPointF underneath;
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

class Perspective : public TilingMouseAction
{
public:
    Perspective(TilingMaker * tilingMaker, QPointF spt);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );
    void addPoint(QPointF spt);

private:
    QPointF spt;
    QPolygonF poly;
};

typedef shared_ptr<Perspective> PerspectivePtr;

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
