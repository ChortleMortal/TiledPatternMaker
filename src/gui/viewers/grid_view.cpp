#include "gui/top/view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/grid_view.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/geo.h"

GridView::GridView() : LayerController(Type::Grid,Model::Derived,Sys::config->gridZLevel,"Grid")
{
    config  = Sys::config;
    setZLevel(config->gridZLevel);

    // runs after Layer::initLayer has cleared the lock and breakaway flags
    connect(viewControl(), &ViewController::sig_resetLayers, this, &GridView::restorePosition);
}

GridView::~GridView()
{}

void GridView::paint(QPainter * painter)
{
    draw(painter);
}

QTransform GridView::getCanvasTransform()
{
    // the grid is not warped by canvas distortion, unlike the design content it overlays
    return viewControl()->getCanvas()->getUndistortedTransform();
}

void GridView::setModelXform(const Xform & xf, bool update, uint sigid)
{
    LayerController::setModelXform(xf,update,sigid);
    storeXform();
}

// user selection for a units row; applied now if that row is active
void GridView::setPosition(eGridUnits units, eGridPosition pos)
{
    eGridPosition * rowPos = positionOf(units);
    if (!rowPos)
        return;

    *rowPos = pos;
    if (units != config->gridUnits)
        return;

    auto vc = viewControl();
    if (isSolo())
        emit vc->sig_solo(this,false);

    switch (pos)
    {
    case GRID_POS_CENTERED:
        if (units == GRID_UNITS_SCREEN)
        {
            emit vc->sig_solo(this,true);
            setModelXform(Xform(),false,Sys::nextSigid());
            lock(true);
            emit vc->sig_solo(this,false);
        }
        else
        {
            lock(false);
            breakaway(false);
        }
        break;

    case GRID_POS_OFFSET:
        lock(false);
        breakaway(true);
        break;

    case GRID_POS_FIXED:
        lock(true);     // also breaks away
        break;
    }

    storeXform();
    emit sig_updateView();
}

// solo so only the grid moves; when off the grid stays Offset
void GridView::setMoving(bool move)
{
    if (move)
    {
        lock(false);
        emit viewControl()->sig_solo(this,true);
    }
    else
    {
        emit viewControl()->sig_solo(this,false);
    }

    eGridPosition * rowPos = positionOf(config->gridUnits);
    if (rowPos)
        *rowPos = GRID_POS_OFFSET;

    storeXform();
    emit sig_updateView();
}

// re-applies the saved position and xform of the active units row
void GridView::restorePosition()
{
    lock(false);
    breakaway(false);

    eGridPosition * rowPos = positionOf(config->gridUnits);
    bool screen = (config->gridUnits == GRID_UNITS_SCREEN);
    if (rowPos && (screen || *rowPos != GRID_POS_CENTERED))
    {
        Xform xf = (*rowPos == GRID_POS_CENTERED) ? Xform() : *xformOf(config->gridUnits);
        breakaway(true);
        setModelXform(xf,false,Sys::nextSigid());
        if (*rowPos != GRID_POS_OFFSET)
            lock(true);
    }
    emit sig_updateView();
}

// reads back the position from the layer flags, which other pages may have changed
void GridView::syncPosition()
{
    eGridPosition * rowPos = positionOf(config->gridUnits);
    if (!rowPos)
        return;

    bool screen = (config->gridUnits == GRID_UNITS_SCREEN);
    if (isUserLocked())
        *rowPos = (screen && getModelXform() == Xform()) ? GRID_POS_CENTERED : GRID_POS_FIXED;
    else if (isBreakaway() || screen)
        *rowPos = GRID_POS_OFFSET;
    else
        *rowPos = GRID_POS_CENTERED;

    storeXform();
}

void GridView::storeXform()
{
    Xform * rowXf = xformOf(config->gridUnits);
    if (rowXf && isBreakaway())
        *rowXf = getModelXform();
}

eGridPosition * GridView::positionOf(eGridUnits units)
{
    if (units == GRID_UNITS_MODEL)  return &config->gridModelPos;
    if (units == GRID_UNITS_SCREEN) return &config->gridScreenPos;
    return nullptr;
}

Xform * GridView::xformOf(eGridUnits units)
{
    if (units == GRID_UNITS_MODEL)  return &config->gridModelXform;
    if (units == GRID_UNITS_SCREEN) return &config->gridScreenXform;
    return nullptr;
}

void GridView::draw(QPainter * painter)
{
    if (config->gridUnits == GRID_UNITS_TILE && config->gridTilingAlgo == REGION)
        drawFromTilingRegion(painter);
    else if (config->gridUnits != GRID_UNITS_OFF)
        drawLattice(painter);

    drawCenters(painter);
}

void GridView::drawCenters(QPainter * painter)
{
    if (config->showGridModelCenter)
    {
        GeoGraphics gg(painter,getDesignTransform());
        QPen pen(Qt::green,3);
        gg.drawLine(QPointF(-10,10),QPointF(10,-10),pen);
        gg.drawLine(QPointF(-10,-10),QPointF(10,10),pen);
    }

    if (config->showGridViewCenter)
    {
        QRectF r = Sys::systemViewController->getViewRect();
        painter->save();
        painter->setPen(QPen(Qt::blue,3));
        painter->drawLine(r.topLeft(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.topRight());
        painter->restore();
    }
}

// one repeat-unit cell (T1 x T2 parallelogram at the first tile's center) per fill placement
void GridView::drawFromTilingRegion(QPainter *painter)
{
    auto tiling = Sys::tilingMaker->getSelectedTiling();
    if (!tiling)
        return;

    QPointF origin = getTilingOrigin(tiling.get());
    QPointF t1     = tiling->hdr().getTrans1();
    QPointF t2     = tiling->hdr().getTrans2();
    if (t1.isNull() || t2.isNull())
        return;

    QPolygonF cell;
    cell << origin << origin + t1 << origin + t1 + t2 << origin + t2;

    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);
    GeoGraphics gg(painter,getLayerTransform());

    FillRegion flood(tiling.get(),tiling->hdr().getTilingCanvas()->getFillData());
    const Placements & placements = flood.getPlacements(config->repeatMode);
    for (const auto & t : std::as_const(placements))
    {
        gg.drawPolygon(t.map(cell),pen);
    }

    QColor fill = TilingMakerView::tile_color;
    fill.setAlphaF(0.4);
    gg.fillPolygon(cell,fill);
}

// Lattice points are a*u + b*v mapped by S; lines are drawn along u and v, and if third
// also along u-v. The view is mapped into (a,b) so lines cover it.
void GridView::drawLattice(QPainter * painter)
{
    QTransform S;
    bool       third;
    if (!getLattice(S,third))
        return;

    QPen pen;
    switch (config->gridUnits)
    {
    case GRID_UNITS_SCREEN: pen = QPen(QColor(config->gridColorScreen),config->gridScreenWidth); break;
    case GRID_UNITS_MODEL:  pen = QPen(QColor(config->gridColorModel), config->gridModelWidth);  break;
    default:                pen = QPen(QColor(config->gridColorTiling),config->gridTilingWidth); break;
    }

    QRectF lr = S.inverted().mapRect(QRectF(Sys::systemViewController->getViewRect()));
    int a0 = qFloor(lr.left());
    int a1 = qCeil(lr.right());
    int b0 = qFloor(lr.top());
    int b1 = qCeil(lr.bottom());

    painter->save();
    painter->setPen(pen);
    auto drawLine = [&](QPointF p1, QPointF p2)
    {
        painter->drawLine(S.map(p1),S.map(p2));
    };
    for (int a = a0; a <= a1; a++)
        drawLine(QPointF(a,b0),QPointF(a,b1));
    for (int b = b0; b <= b1; b++)
        drawLine(QPointF(a0,b),QPointF(a1,b));
    if (third)
    {
        // lines of constant a+b
        for (int k = a0 + b0; k <= a1 + b1; k++)
        {
            int bs = qMax(b0,k - a1);
            int be = qMin(b1,k - a0);
            drawLine(QPointF(k - bs,bs),QPointF(k - be,be));
        }
    }
    painter->restore();
}

// S maps lattice coords (a,b) to screen for the current units
//   screen: origin at view center, follows the grid layer's xform in pixels (see setPosition)
//   model:  origin at model (0,0), grid layer transform (see setPosition)
//   tile:   tiling's T1,T2 through the first tile's center
bool GridView::getLattice(QTransform & S, bool & third)
{
    QPointF    origin(0,0);
    QPointF    u, v;
    QTransform T;
    third = false;

    switch (config->gridUnits)
    {
    case GRID_UNITS_SCREEN:
        if (!getBasis(config->gridScreenSpacing,u,v,third))
            return false;
        origin = QRectF(Sys::systemViewController->getViewRect()).center();
        T      = getModelTransform();
        break;

    case GRID_UNITS_MODEL:
        if (!getBasis(config->gridModelSpacing,u,v,third))
            return false;
        T      = getLayerTransform();
        break;

    case GRID_UNITS_TILE:
    {
        auto tiling = Sys::tilingMaker->getSelectedTiling();
        if (!tiling)
            return false;
        origin = getTilingOrigin(tiling.get());
        u      = tiling->hdr().getTrans1();
        v      = tiling->hdr().getTrans2();
        T      = getLayerTransform();
        break;
    }

    case GRID_UNITS_OFF:
        return false;
    }

    S = QTransform(u.x(),u.y(),v.x(),v.y(),origin.x(),origin.y()) * T;
    if (!S.isInvertible())
        return false;

    QPointF s0 = S.map(QPointF(0,0));
    qreal pixelStep = qMin(QLineF(s0,S.map(QPointF(1,0))).length(),QLineF(s0,S.map(QPointF(0,1))).length());
    if (pixelStep < 4.0)
    {
        qDebug() << "grid step too small" << pixelStep;
        return false;
    }
    return true;
}

// lattice basis for the grid type; third adds lines along u-v
//   orthogonal: u,v are horizontal/vertical, spacing step
//   rhombic:    u,v at +/- angle, lines of each family step apart vertically
//   isometric:  rhombic at 30 degrees plus vertical lines, equilateral with side step
bool GridView::getBasis(qreal step, QPointF & u, QPointF & v, bool & third)
{
    u     = QPointF(step,0);
    v     = QPointF(0,step);
    third = false;

    eGridType type = config->gridType;
    if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal angle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;
        qreal rad   = qDegreesToRadians(angle);
        qreal sin   = qSin(rad);
        if (qAbs(sin) < 0.01)
            return false;
        qreal len = step / (2.0 * sin);
        u = QPointF(len * qCos(rad),  len * sin);
        v = QPointF(len * qCos(rad), -len * sin);
        third = (type == GRID_ISOMETRIC);
    }
    return true;
}

QPointF GridView::getTilingOrigin(Tiling * tiling)
{
    const PlacedTileSet tileSet = tiling->unit().getIncluded();
    if (tileSet.isEmpty())
        return QPointF(0,0);

    auto placedTile = tileSet.first();
    return placedTile->getPlacement().map(placedTile->getTile()->getCenter());
}

QTransform GridView::getDesignTransform()
{
    return getCanvasTransform() * modelTransformOf(viewControl()->getSystemModelXform());
}

// used by tiling maker; returns the nearest grid crossing in tiling maker model units
bool GridView::nearGridPoint(QPointF spt, QPointF & foundGridPoint)
{
    if (!viewControl()->isEnabled(Layer::Type::Grid) || !config->tm_snapToGrid)
    {
        qDebug() << "SNAP grid: off - grid enabled" << viewControl()->isEnabled(Layer::Type::Grid) << "snapToGrid" << config->tm_snapToGrid;
        return false;
    }

    QTransform S;
    bool       third;
    if (!getLattice(S,third))
    {
        qDebug() << "SNAP grid: no lattice, units" << config->gridUnits;
        return false;
    }

    // a crossing near spt is a corner of the lattice cell containing spt
    QPointF lpt = S.inverted().map(spt);
    int a = qFloor(lpt.x());
    int b = qFloor(lpt.y());

    QPointF best;
    qreal   bestDist = std::numeric_limits<qreal>::max();
    for (int i = 0; i <= 1; i++)
    {
        for (int j = 0; j <= 1; j++)
        {
            QPointF pt   = S.map(QPointF(a + i,b + j));
            qreal   dist = QLineF(spt,pt).length();
            if (dist < bestDist)
            {
                bestDist = dist;
                best     = pt;
            }
        }
    }

    if (!Geo::isNear(spt,best))
        return false;

    foundGridPoint = Sys::tilingMakerView->screenToModel(best);
    qDebug() << "SNAP grid: hit" << spt << "->" << best << "model" << foundGridPoint;
    return true;
}

void GridView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void GridView::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}
