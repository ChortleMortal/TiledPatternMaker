#include <QApplication>

#include "motifs/motif.h"
#include "geometry/circle.h"
#include "geometry/arcdata.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "makers/motif_maker/motif_button.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/crop_view.h"
#include "viewers/map_editor_view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

const bool debugMouse = false;

MapedViewPtr MapEditorView::spThis;

MapedViewPtr MapEditorView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<MapEditorView>(1);
    }
    return spThis;
}

MapEditorView::MapEditorView(int ignore) : LayerController("MapEditorView")
{
    Q_UNUSED(ignore);

    config = Configuration::getInstance();

    mapLineWidth          = 3.0;
    constructionLineWidth = 1.0;
    selectionWidth        = 3.0;
}

void MapEditorView::init(MapEditorDb * maped_db, MapEditorSelection * selector)
{
    db     = maped_db;
    this->selector = selector;
}

void MapEditorView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // set up view transform
    viewT       = getLayerTransform();
    viewTinv    = viewT.inverted();
    //qDebug().noquote() << "MapEditorView::paint viewT" << Transform::toInfoString(viewT);

    // draw
    draw(painter);
    drawCenter(painter);
}

void MapEditorView::draw(QPainter *painter )
{
    selector->buildEditorDB();

    drawConstructionLines(painter);
    drawConstructionCircles(painter);

    // FIXME this assumes that composite and sources have the sanme source characteristics
    // A better scheme would be to put source design element references into composite data as an array
    for (auto & layer : db->getComposableLayers())
    {
        DesignElementPtr del = layer.wdel.lock();
        if (del)
        {
            Q_ASSERT(db->isMotif(layer.type));
            drawTile(painter,del);
            drawBoundaries(painter,del);
        }
    }

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        // draw the dcel
        drawDCEL(painter);
    }
    else
    {
        // draw the map
        Q_ASSERT(config->mapEditorMode == MAPED_MODE_MAP);
        if (db->isViewSelected(COMPOSITE))
        {
            drawMap(painter,COMPOSITE,Qt::green);
        }
        else
        {
            if (db->isViewSelected(LAYER_1))
            {
                drawMap(painter,LAYER_1,QColorConstants::Svg::forestgreen);
            }
            if (db->isViewSelected(LAYER_2))
            {
                drawMap(painter,LAYER_2,QColorConstants::Svg::orange);
            }
            if (db->isViewSelected(LAYER_3))
            {
                drawMap(painter,LAYER_3,QColorConstants::Svg::dodgerblue);
            }
        }
    }

    drawPoints(painter,selector->points);

    for (auto sel : selector->getCurrentSelections())
    {
        eMapSelection type = sel->getType();
        if (type == MAP_EDGE && db->showMap)
        {
            QLineF l1 = sel->getLine();
            painter->setPen(QPen(Qt::red,selectionWidth));
            painter->drawLine(viewT.map(l1));
            auto edge = sel->getEdge();
            if (edge && edge->isCurve() && db->showArcCentre)
            {
                QPointF pt = edge->getArcCenter();
                qreal radius = 8.0;
                painter->save();
                painter->setPen(QPen(Qt::blue,selectionWidth));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(viewT.map(pt), radius, radius);
                painter->restore();
            }
        }
        else if (type == MAP_LINE  && db->showConstructionLines)
        {
            QLineF l1 = sel->getLine();
            painter->setPen(QPen(Qt::red,selectionWidth));
            painter->drawLine(viewT.map(l1));
        }
        else if ((type == MAP_VERTEX || type == MAP_POINT) && db->showPoints)
        {
            qreal radius = 8.0;
            painter->setPen(QPen(Qt::blue,selectionWidth));
            painter->setBrush(Qt::red);
            painter->drawEllipse(viewT.map(sel->getPoint()), radius, radius);
        }
        else if (type == MAP_CIRCLE)
        {
            auto c = sel->getCircle();
            Q_ASSERT(c);
            if (!sel->isConstructionLine() || (sel->isConstructionLine() && db->showConstructionLines))
            {
                qreal radius   = c->radius;
                QPointF center = c->centre;
                radius = Transform::scalex(viewT) * radius;
                painter->setPen(QPen(Qt::red,selectionWidth));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(viewT.map(center), radius, radius);

                if (sel->hasCircleIntersect())
                {
                    QPointF pt = sel->getPoint();
                    radius = 8.0;
                    painter->setPen(QPen(Qt::red,selectionWidth));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawEllipse(viewT.map(pt), radius, radius);
                }
            }
        }
    }

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->draw(painter);
    }
}


void MapEditorView::drawMap(QPainter * painter, eLayer layer, QColor color)
{
    if (!db->showMap)
        return;

    auto map = db->getMap(layer);
    if (!map)
        return;

    auto type = db->getMapType(map);

    CropMaker * ced = CropMaker::getInstance();
    auto crop = ced->getActiveCrop();
    QTransform t;
    if (crop && type == MAPED_TYPE_CROP)
    {
        t = CropView::getSharedInstance()->getLayerTransform();
    }
    else
    {
        t = viewT;
    }

    QPen pen(color,mapLineWidth);
    painter->setPen(pen);

    for (auto & edge : qAsConst(map->getEdges()))
    {
        QPointF v1 = t.map(edge->v1->pt);
        QPointF v2 = t.map(edge->v2->pt);

        if (db->showDirnPoints)
        {
            GeoGraphics::drawLineArrowDirect(QLineF(v1,v2),pen,painter);
        }

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(v1,v2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF ac = viewT.map(edge->getArcCenter());

            ArcData ad(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawArc(ad.rect, start, span);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            QPointF ac = t.map(edge->getArcCenter());

            ArcData ad(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawChord(ad.rect, start, span);
        }
        if (edge->isCurve() && db->showArcCentre)
        {
            QPointF pt = edge->getArcCenter();
            qreal radius = 8.0;
            painter->save();
            painter->setPen(QPen(Qt::white,selectionWidth));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(viewT.map(pt), radius, radius);
            painter->restore();
        }
    }
}

void MapEditorView::drawDCEL(QPainter * painter)
{
    if (!db->showMap)
        return;

    auto dcel = db->getActiveDCEL();
    if (!dcel)
        return;

    QPen pen(Qt::green,mapLineWidth);
    painter->setPen(pen);

    for (auto edge : qAsConst(dcel->getEdges()))
    {
        QPointF v1 = viewT.map(edge->v1->pt);
        QPointF v2 = viewT.map(edge->v2->pt);

        if (db->showDirnPoints)
        {
            GeoGraphics::drawLineArrowDirect(QLineF(v1,v2),pen,painter);
        }

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(viewT.map(v1),viewT.map(v2));
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF ac = viewT.map(edge->getArcCenter());

            ArcData ad(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawArc(ad.rect, start, span);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            QPointF ac = viewT.map(edge->getArcCenter());

            ArcData ad(v1,v2,ac,edge->isConvex());

            int start = qRound(ad.start * 16.0);
            int span  = qRound(ad.span  * 16.0);

            painter->drawChord(ad.rect, start, span);
        }
    }
}

void MapEditorView::drawTile(QPainter * painter, DesignElementPtr del)
{
    if (!del)
        return;

    if (!db->showBoundaries)
        return;

    TilePtr tile = del->getTile();
    if (tile)
    {
        QTransform placement = getPlacement(tile);

        QPolygonF poly = tile->getPoints();
        poly = placement.map(poly);
        poly = viewT.map(poly);

        painter->setPen(QPen(Qt::magenta,1));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(poly); // not filled
    }
}

void MapEditorView::drawBoundaries(QPainter *painter,  DesignElementPtr del)
{
    if (!del)
        return;

    if (!db->showBoundaries)
        return;

    TilePtr tile = del->getTile();
    MotifPtr  motif = del->getMotif();
    if (!motif || !tile)
        return;

    const ExtendedBoundary & eb = motif->getExtendedBoundary();

    QTransform placement = getPlacement(tile);

    QPolygonF figBoundary = motif->getMotifBoundary();
    figBoundary  = placement.map(figBoundary);

    //figure->buildExtBoundary();
    QPolygonF extBoundary = eb.get();
    extBoundary           = placement.map(extBoundary);

    qreal  boundaryScale  = eb.scale;
    boundaryScale        *= Transform::scalex(placement);

    // paint figure boundary
    painter->setPen(QPen(QColor(255,127,0),3));   // orange
    painter->drawPolygon(viewT.map(figBoundary));

    // paint external boundary
    painter->setPen(QPen(Qt::yellow,1));
    if (!eb.isCircle())
    {
        painter->drawPolygon(viewT.map(extBoundary));
    }
    else
    {
        qreal scale = Transform::scalex(viewT);
        QPointF  pt = placement.map(QPointF(0,0));
        painter->drawEllipse(viewT.map(pt),boundaryScale*scale,boundaryScale*scale);
    }
}

void MapEditorView::drawPoints(QPainter * painter,  QVector<pointInfo> & points)
{
    if (!db->showPoints)
        return;

    qreal radius = 1.0;
    for (auto it = points.begin(); it != points.end(); it++)
    {
        pointInfo & pi = *it;
        switch (pi._type)
        {
        case PT_VERTEX:
            if (!db->showMap) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::blue,1));
            painter->setBrush(Qt::blue);
            break;
        case PT_VERTEX_MID:
            if (!db->showMap) continue;
            if (!db->showMidPoints) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::darkRed,1));
            painter->setBrush(Qt::darkRed);
            break;
        case PT_VERTEX_MID2:
            if (!db->showMap) continue;
            if (!db->showDirnPoints) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::darkGreen,1));
            painter->setBrush(Qt::darkGreen);
            break;
        case PT_LINE:
        case PT_CIRCLE:
            if (!db->showConstructionLines) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::yellow,1));
            painter->setBrush(Qt::yellow);
            break;
        case PT_CIRCLE_1:
            if (!db->showConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::magenta,1));
            painter->setBrush(Qt::magenta);
            break;
        case PT_CIRCLE_2:
            if (!db->showConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::cyan,1));
            painter->setBrush(Qt::cyan);
            break;
        case PT_LINE_MID:
            if (!db->showConstructionLines) continue;
            if (!db->showMidPoints) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::darkYellow,1));
            painter->setBrush(Qt::darkYellow);
            break;
        }
        QPointF pos = viewT.map(pi._pt);
        painter->drawEllipse(pos, radius, radius);
    }
}

void MapEditorView::drawConstructionLines(QPainter * painter)
{
    if (!db->showConstructionLines)
        return;

    painter->setPen(QPen(Qt::white,constructionLineWidth));
    for (auto line : qAsConst(db->constructionLines))
    {
        painter->drawLine(viewT.map(line));
    }
}

void MapEditorView::drawConstructionCircles(QPainter * painter)
{
    if (!db->showConstructionLines)
        return;

    for (const auto &circle : qAsConst(db->constructionCircles))
    {
        QPointF pt = viewT.map(circle->centre);
        painter->setPen(QPen(Qt::white,constructionLineWidth));
        painter->drawEllipse(pt, Transform::scalex(viewT) * circle->radius, Transform::scalex(viewT)  * circle->radius);

        if (db->showPoints)
        {
            // draw center X
            painter->setPen(QPen(Qt::red,constructionLineWidth));
            qreal len = 9.0;
            painter->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()));
            painter->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len));
        }
    }
}

QTransform MapEditorView::getPlacement(TilePtr tile)
{
    QTransform t;
    auto proto = db->getMotifPrototype();
    if (proto)
    {
        auto tiling = proto->getTiling();
        t = tiling->getPlacement(tile,0);
    }
    return t;
}

void MapEditorView::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    CropMaker * cm = CropMaker::getInstance();
    if (cm->getState() == CROPMAKER_STATE_ACTIVE)
    {
        return;     // it will be handled by the CropMaker
    }

    SelectionSet set = selector->findSelectionsUsingDB(spt);
    for (auto it = set.begin(); it != set.end(); it++)
    {
        auto sel = *it;
        switch (mouseButton)
        {
        case Qt::LeftButton:
            switch (sel->getType())
            {
            case MAP_VERTEX:
                db->setMouseInteraction(make_shared<MoveVertex>(sel->getVertex(), spt));
                return;
            case MAP_EDGE:
                db->setMouseInteraction(make_shared<MoveEdge>(sel->getEdge(), spt));
                return;
            default:
                break;
            }
            break;

        case Qt::MiddleButton:
        case Qt::RightButton:
        default:
            break;
        }
    }

    db->resetMouseInteraction();
}

void MapEditorView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void MapEditorView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void MapEditorView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void MapEditorView:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void MapEditorView::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
    }
}

//////////////////////////////////////////////////////////////////
///
/// slots and evzents
///
//////////////////////////////////////////////////////////////////

const Xform  & MapEditorView::getCanvasXform()
{
    switch (db->getMapType(db->getEditMap()))
    {
    case MAPED_TYPE_UNKNOWN:
    case MAPED_LOADED_FROM_FILE:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_CROP:
        return xf_canvas;

    case MAPED_LOADED_FROM_MOTIF:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_COMPOSITE:
    case MAPED_TYPE_COMPOSITE_MOTIF:

    case MAPED_LOADED_FROM_MOSAIC:
    case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
    case MAPED_LOADED_FROM_TILING_UNIT:
    case MAPED_LOADED_FROM_TILING_REPEATED:
        return Layer::getCanvasXform();
    }
    return xf_canvas;   // makes compiler happy
}

void MapEditorView::setCanvasXform(const Xform & xf)
{
    switch (db->getMapType(db->getEditMap()))
    {
    case MAPED_TYPE_UNKNOWN:
    case MAPED_LOADED_FROM_FILE:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_COMPOSITE:
    case MAPED_TYPE_CROP:
        xf_canvas = xf;
        break;

    case MAPED_LOADED_FROM_MOTIF:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_COMPOSITE_MOTIF:

    case MAPED_LOADED_FROM_MOSAIC:
    case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
    case MAPED_LOADED_FROM_TILING_UNIT:
    case MAPED_LOADED_FROM_TILING_REPEATED:
        Layer::setCanvasXform(xf);
        break;
    }
}

void MapEditorView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void MapEditorView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

    setMousePos(spt);

    if (debugMouse)
    {
        MapMouseActionPtr mma = db->getMouseInteraction();
        if (mma)
            qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << mma->desc;
        else
            qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << "no mouse interaction";
    }

    SelectionSet set;

    switch (db->getMouseMode())
    {
    case MAPED_MOUSE_NONE:
        startMouseInteraction(mousePos,btn);
        break;

    case MAPED_MOUSE_DRAW_LINE:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(make_shared<DrawLine>(set,mousePos));
        break;

    case MAPED_MOUSE_CREATE_LINE:
    {
        QVector<EdgePtr> qvep;
        SelectionSet set;
        for (auto & map : db->getMapLayerMaps())
        {
            selector->findEdges(map,mousePos, qvep, set);
        }
        if (set.size())
        {
            MapPtr map = db->getEditMap();
            if (map)
            {
                qreal len    = config->mapedLen;
                EdgePtr ep   = set.first()->getEdge();

                QLineF line  = Point::createLine(ep->getLine(),ep->getMidPoint(),qDegreesToRadians(config->mapedAngle),len);
                VertexPtr v1 = map->insertVertex(line.p1());
                VertexPtr v2 = map->insertVertex(line.p2());
                map->insertEdge(v1,v2);

                line  = Point::createLine(ep->getLine(),ep->getMidPoint(),qDegreesToRadians(-config->mapedAngle),len);
                v1 = map->insertVertex(line.p1());
                v2 = map->insertVertex(line.p2());
                map->insertEdge(v1,v2);
            }
        }
        else
            qDebug() << "no edge";
        break;
    }

    case MAPED_MOUSE_CONSTRUCTION_LINES:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(make_shared<ConstructionLine>(set,mousePos));
        break;

    case MAPED_MOUSE_DELETE:
        set = selector->findSelectionsUsingDB(mousePos);
        // only delete one line
        for (auto sel : set)
        {
            if (sel->getType() == MAP_EDGE)
            {
                MapPtr map = db->getEditMap();
                if (map)
                {
                    map->removeEdge(sel->getEdge());
                    map->cleanse(joinupColinearEdges);
                    map->verify();
                }
                break;
            }
            else if (sel->getType() == MAP_LINE && sel->isConstructionLine())
            {
                QLineF line = sel->getLine();
                db->constructionLines.removeAll(line);
                db->getStash()->stash();
                break;
            }
            else if (sel->getType() == MAP_CIRCLE)
            {
                auto c = sel->getCircle();
                db->constructionCircles.removeOne(c);
                break;
            }
        }
        forceRedraw();
        //setMapedMouseMode(MAPED_MOUSE_NONE);
        break;

    case MAPED_MOUSE_SPLIT_LINE:
    {
        QVector<EdgePtr> qvep;
        SelectionSet set;
        for (auto & map : db->getMapLayerMaps())
        {
            selector->findEdges(map, mousePos, qvep, set);
        }
        if (set.size())
        {
            MapPtr map = db->getEditMap();
            for (auto & sel : set)
            {
                map->splitEdge(sel->getEdge());
                map->verify();
            }
            forceRedraw();
            //setMapedMouseMode(MAPED_MOUSE_NONE);
        }
    }
        break;

    case MAPED_MOUSE_EXTEND_LINE_P1:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(make_shared<ExtendLine>(set,mousePos,true));
        break;

    case MAPED_MOUSE_EXTEND_LINE_P2:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(make_shared<ExtendLine>(set,mousePos,false));
        break;

    case MAPED_MOUSE_CONSTRUCTION_CIRCLES:
        if (btn == Qt::RightButton)
        {
            // add circle
            SelectionSet endset    = selector->findSelectionsUsingDB(mousePos);
            MapSelectionPtr endsel = selector->findAPoint(endset);
            QPointF center;
            if (endsel)
            {
                center = QPointF(endsel->getPoint());
            }
            else
            {
                center = viewTinv.map(mousePos);
            }
            db->constructionCircles.push_back(make_shared<Circle>(center, config->mapedRadius));
            db->getStash()->stash();
            forceRedraw();
        }
        else if (btn == Qt::LeftButton)
        {
            MapSelectionPtr sel = selector->findConstructionCircle(mousePos);
            if (sel)
            {
                auto c = sel->getCircle();
                db->setMouseInteraction(make_shared<EditConstructionCircle>(c,mousePos));
            }
        }
        break;
    }

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
        qDebug().noquote() << "press end:"  << mma->desc;
    else
        qDebug() << "press end: no mouse_interaction";

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getMouseMode(MOUSE_MODE_CENTER))
    {
        setCenterScreenUnits(mousePos);
    }
}


void MapEditorView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos)  << sMapEditorMouseMode[db->getMouseMode()];

    selector->setCurrentSelections(selector->findSelectionsUsingDB(mousePos));

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->updateDragging(mousePos);
    }

    forceRedraw();
}

void MapEditorView::slot_mouseTranslate(QPointF pt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setCanvasXform(xf);
    }
}

void MapEditorView::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    selector->setCurrentSelections(selector->findSelectionsUsingDB(mousePos));

    forceRedraw();
}

void MapEditorView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        db->resetMouseInteraction();
    }
}

void MapEditorView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void MapEditorView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() == VIEW_MAP_EDITOR && view->getKbdMode( KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void MapEditorView::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}


//////////////////////////////////////////////////////////////////////
///
///  MapEditor Layer
///
//////////////////////////////////////////////////////////////////////

MapEditorLayer::MapEditorLayer()
{
    reset();
}

MapEditorLayer::MapEditorLayer(MapPtr map, eMapEditorMapType type)
{
    qDebug() << "Inserting map" << map->summary();
    this->map = map;
    this->type = type;
}

MapEditorLayer::MapEditorLayer(MapPtr map, eMapEditorMapType type, WeakDELPtr wdel)
{
    qDebug() << "Inserting map" << map->summary();
    this->map = map;
    this->type = type;
    this->wdel = wdel;
}

void MapEditorLayer::reset()
{
    map.reset();
    type = MAPED_TYPE_UNKNOWN;
    wdel.reset();
}

