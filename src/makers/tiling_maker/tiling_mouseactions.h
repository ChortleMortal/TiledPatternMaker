#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QtWidgets>
#include "base/shared.h"
#include "makers/tiling_maker/tiling_selection.h"
#include "viewers/GeoGraphics.h"

enum eMouseMode
{
    NO_MOUSE_MODE,
    COPY_MODE,
    DELETE_MODE,
    TRANSLATION_VECTOR_MODE,
    DRAW_POLY_MODE,
    INCLUSION_MODE,
    POSITION_MODE,
    MEASURE_MODE,
    BKGD_SKEW_MODE,
    EDIT_FEATURE_MODE,
    CURVE_EDGE_MODE,
    FLATTEN_EDGE_MODE,
    MIRROR_X_MODE,
    MIRROR_Y_MODE
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
    TilingMaker * tm;

    QPointF wStart;
    QPointF wEnd;
};

class MouseAction
{
public:
    MouseAction(TilingMaker * tm, TilingSelectionPtr sel, QPointF spt );
    virtual ~MouseAction() {}
    virtual void updateDragging( QPointF spt );
    virtual void draw(GeoGraphics * g2d );
    virtual void endDragging( QPointF spt );

    QString            desc;

protected:
    QPointF            wLastDrag;   // screen point
    TilingSelectionPtr selection;
    QColor             drag_color;

    TilingMaker   * tm;        // DAC added

private:
};

typedef shared_ptr<MouseAction> MouseActionPtr;

class MovePolygon : public MouseAction
{
public:
    MovePolygon(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt);
    virtual void updateDragging( QPointF spt );
};

class CopyMovePolygon : public MovePolygon
{
public:
    CopyMovePolygon(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
    virtual void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class DrawTranslation : public MouseAction
{
public:
    DrawTranslation(TilingMaker * tilingMaker,  TilingSelectionPtr sel, QPointF spt );
    void updateDragging( QPointF spt );
    void draw( GeoGraphics* g2d );
    void endDragging(QPointF spt);
};

class JoinEdge : public MouseAction
{
public:
    JoinEdge(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
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
    JoinMidPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
};

class JoinPoint : public JoinEdge
{
public:
    JoinPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );

private:
    virtual bool snapTo(QPointF spt) override;
};

class CopyJoinEdge : public JoinEdge
{
public:
    CopyJoinEdge(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinMidPoint : public JoinMidPoint
{
public:
    CopyJoinMidPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CopyJoinPoint : public JoinPoint
{
public:
    CopyJoinPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    QTransform initial_transform;
};

class CreatePolygon : public MouseAction
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

class Measure : public MouseAction
{
public:
    Measure(TilingMaker * tilingMaker, QPointF spt, TilingSelectionPtr sel);
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

class Position : public MouseAction
{
public:
    Position(TilingMaker * tilingMaker, QPointF spt);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );

private:
    QPointF spt;
};

class Perspective : public MouseAction
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

class EditFeature : public MouseAction
{
public:
    EditFeature(TilingMaker * tilingMaker,  TilingSelectionPtr sel, PlacedFeaturePtr pfp, QPointF spt );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    PlacedFeaturePtr pfp;
    int              vertexIndex;
};

class EditEdge : public MouseAction
{
public:
    EditEdge(TilingMaker * tilingMaker,  TilingSelectionPtr sel, QPointF spt );
    void draw( GeoGraphics* g2d );
    void updateDragging( QPointF spt );
    void endDragging(QPointF spt);

private:
    QPointF start;
    EdgePtr edge;
    QLineF  perp;
    PlacedFeaturePtr pfp;
};

#endif
