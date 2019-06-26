#ifndef TILING_MOUSEACTIONS_H
#define TILING_MOUSEACTIONS_H

#include <QtWidgets>
#include "base/shared.h"
#include "makers/TilingSelection.h"
#include "viewers/GeoGraphics.h"

enum eMouseMode
{
    NO_MODE,
    COPY_MODE,
    DELETE_MODE,
    TRANS_MODE,
    DRAW_POLY_MODE,
    INCLUSION_MODE,
    TRANSFORM_MODE,
    POSITION_MODE,
    MEASURE_MODE
};

class TilingDesigner;

class Measurement
{
public:
    Measurement();
    void reset();

    void setStart(QPointF spt);
    void setEnd(QPointF spt);

    QPointF start();
    QPointF end();
    QPointF startS();
    QPointF endS();

    qreal lenS();
    qreal len();

    bool    active;

private:
    TilingDesigner * td;
    QPointF _start;
    QPointF _end;
};


class MouseAction
{
public:
    MouseAction(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    virtual ~MouseAction() {}
    virtual void updateDragging( QPointF spt );
    virtual void draw(GeoGraphics * g2d );
    virtual void endDragging( QPointF spt );

    QString            desc;

protected:
    QPointF            last_drag;
    TilingSelectionPtr selection;
    QColor             drag_color;

    TilingDesigner   * td;        // DAC added

private:
};

typedef shared_ptr<MouseAction> MouseActionPtr;

class MovePolygon : public MouseAction
{
public:
    MovePolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    virtual void updateDragging( QPointF spt );
};

class CopyMovePolygon : public MovePolygon
{
public:
    CopyMovePolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    virtual void endDragging( QPointF spt );

private:
    Transform initial_transform;
};

class DrawTranslation : public MouseAction
{
public:
    DrawTranslation(TilingDesigner * td,  TilingSelectionPtr sel, QPointF spt );
    void updateDragging( QPointF spt );
    void draw( GeoGraphics* g2d );
    void endDragging(QPointF spt);
};

class JoinEdge : public MouseAction
{
public:
    JoinEdge(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    void updateDragging( QPointF spt );
    void endDragging( QPointF spt );

private:
    bool snapToEdge( QPointF spt );
};

class CopyJoinEdge : public JoinEdge
{

public:
    CopyJoinEdge(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    void endDragging( QPointF spt );

private:
    Transform initial_transform;
};

class DrawPolygon : public MouseAction
{
public:
    DrawPolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt );
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

private:
    void addVertex(TilingSelectionPtr sel);
};

class Measure : public MouseAction
{
public:
    Measure(TilingDesigner * td, QPointF spt, TilingSelectionPtr sel);
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
    Position(TilingDesigner * td, QPointF spt);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

private:
    QPointF spt;
};

#endif
