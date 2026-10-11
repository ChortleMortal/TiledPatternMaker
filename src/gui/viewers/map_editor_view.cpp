#include <QApplication>
#include <QMenu>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_selection.h"
#include "gui/model_editors/motif_edit/tile_motif_button.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/crop_maker_view.h"    // REQUIRED
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/map_editor_view.h"
#include "model/makers/crop_maker.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/dcel.h"              // REQUIRED
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"

MapEditorView::MapEditorView() : LayerController(Type::MapEditor,Model::Derived,ZLevel::Standard_Z0,"Map Editor")
{
    config      = Sys::config;
    debugMouse  = false;

    mapLineWidth          = 3.0;
    constructionLineWidth = 1.0;
    selectionWidth        = 3.0;
}

MapEditorView::~MapEditorView()
{}

void MapEditorView::init(MapEditor *maped, MapEditorSelection * selector)
{
    editor = maped;
    db     = maped->getDb();
    this->selector = selector;
}

void MapEditorView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // set up view transform
    viewT       = getLayerTransform();
    viewTinv    = viewT.inverted();
    //qDebug().noquote() << "MapEditorView::paint viewT" << Transform::info(viewT);

    // draw
    draw(painter);
    drawLayerModelCenter(painter);
}

void MapEditorView::draw(QPainter *painter )
{
    // TODO: MapEditor::createComposite() merges tile-motifs from multiple source
    // layers (L1/L2/L3) but keeps only a single shared ProtoPtr, overwritten on
    // each source in the loop - if the sources come from different prototypes/
    // tilings, the composite silently keeps only the last one's prototype.
    // A better scheme would track a prototype per source tile-motif rather than
    // one shared prototype for the whole composite.

    db->buildEditorDB();

    drawConstructionLines(painter);
    drawConstructionCircles(painter);

    if (db->showBoundaries)
    {
        QVector<TimoMap *> vec = editor->getDrawTimoMaps();
        for (TimoMap * tmap : std::as_const(vec))
        {
            TileMotifSet timos = tmap->getTileMotifs();
            for (auto & timo : timos)
            {
                drawTile(painter,timo);
                drawBoundaries(painter,timo);
            }
        }
    }

    if (config->mapEditorMode == MapEditor::EditorMode::DCEL)
    {
        // draw the dcel
        drawDCEL(painter);
    }
    else
    {
        // draw the map
        Q_ASSERT(config->mapEditorMode == MapEditor::EditorMode::Map);
        QVector<const MapEditorLayer *> layers = editor->getDrawLayers();
        for (const MapEditorLayer * layer : std::as_const(layers))
        {
            QColor color = layer->getColor();
            MapPtr map   = layer->getMapedLayerMap();

            switch (layer->getLayerMapType())
            {
            case MapType::None:
                break;
            case MapType::Style:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::Prototype:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::Motif:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::MotifUnit:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::TilingUnit:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::TilingFull:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::File:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::FileMotif:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::Created:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::Composite:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::CompositeMotif:
                drawMap(painter,viewT,map,color);
                break;
            case MapType::Crop:
                drawMap(painter,Sys::cropMakerView->getLayerTransform(),map,color);
                break;
            }
        }
    }

    drawPoints(painter,db->getPoints());

    for (auto & sel : selector->getCurrentSelections())
    {
        MapSelection::Type type = sel->getType();
        if (type == MapSelection::Type::MAP_EDGE && db->showMap)
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
        else if (type == MapSelection::Type::MAP_LINE  && db->showConstructionLines)
        {
            QLineF l1 = sel->getLine();
            painter->setPen(QPen(Qt::red,selectionWidth));
            painter->drawLine(viewT.map(l1));
        }
        else if ((type == MapSelection::Type::MAP_VERTEX || type == MapSelection::Type::MAP_POINT) && db->showPoints)
        {
            qreal radius = 8.0;
            painter->setPen(QPen(Qt::blue,selectionWidth));
            painter->setBrush(Qt::red);
            painter->drawEllipse(viewT.map(sel->getPoint()), radius, radius);
        }
        else if (type == MapSelection::Type::MAP_CIRCLE)
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

    drawSelectedConstructionLine(painter);

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->draw(painter);
    }
}

void MapEditorView::drawMap(QPainter * painter, QTransform t, MapPtr map , QColor color)
{
    QPen pen(color,mapLineWidth);
    painter->setPen(pen);

    map->paint(painter,t,db->showDirnPoints,db->showArcCentre,false,true);
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

    dcel->paint(painter,viewT,db->showDirnPoints,db->showArcCentre,false,true);
}

void MapEditorView::drawTile(QPainter * painter, TileMotifPtr timo)
{
    if (!timo)
        return;

    TilePtr tile = timo->getTile();
    if (!tile)
        return;

    TilingPtr tiling = timo->getTiling();
    Q_ASSERT(tiling);
    QTransform placement = tiling->getFirstPlacement(tile);

    QPolygonF poly = tile->getPoints();
    poly = placement.map(poly);
    poly = viewT.map(poly);

    painter->setPen(QPen(Qt::magenta,1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(poly); // not filled
}

void MapEditorView::drawBoundaries(QPainter *painter,  TileMotifPtr timo)
{
    if (!timo)
        return;

    TilingPtr tiling = timo->getTiling();
    TilePtr tile     = timo->getTile();
    MotifPtr  motif  = timo->getMotif();
    if (!tiling || !motif || !tile)
        return;

    QTransform placement = tiling->getFirstPlacement(tile);

    QPolygonF figBoundary = motif->getMotifBoundary();
    figBoundary  = placement.map(figBoundary);

    // paint figure boundary
    painter->setPen(QPen(QColor(255,127,0),3));   // orange
    painter->drawPolygon(viewT.map(figBoundary));

    for (ExtenderPtr ep : motif->getExtenders())
    {
        const ExtendedBoundary & eb = ep->getExtendedBoundary();

        QPolygonF extBoundary = eb.getPoly();
        extBoundary           = placement.map(extBoundary);

        qreal  boundaryScale  = eb.getRadius();
        boundaryScale        *= Transform::scalex(placement);

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
}

void MapEditorView::drawPoints(QPainter * painter,  QVector<PointInfo> & points)
{
    qreal radius = 1.0;
    for (auto & pi : std::as_const(points))
    {
        switch (pi.type)
        {
        case PointInfo::Type::Vertex:
            if (!db->showPoints) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::blue,1));
            painter->setBrush(Qt::blue);
            break;
        case PointInfo::Type::VertexMid:
            if (!db->showMidPoints) continue;
            radius = 4.0;
            painter->setPen(QPen(Qt::darkRed,1));
            painter->setBrush(Qt::darkRed);
            break;
        case PointInfo::Type::Line:
        case PointInfo::Type::Circle:
            if (!db->showConstructionLines) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::yellow,1));
            painter->setBrush(Qt::yellow);
            break;
        case PointInfo::Type::Circle1:
            if (!db->showConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::magenta,1));
            painter->setBrush(Qt::magenta);
            break;
        case PointInfo::Type::Circle2:
            if (!db->showConstructionLines) continue;
            radius = 7.0;
            painter->setPen(QPen(Qt::cyan,1));
            painter->setBrush(Qt::cyan);
            break;
        case PointInfo::Type::LineMid:
            if (!db->showConstructionLines) continue;
            if (!db->showMidPoints) continue;
            radius = 5.0;
            painter->setPen(QPen(Qt::darkYellow,1));
            painter->setBrush(Qt::darkYellow);
            break;
        case PointInfo::Type::Center:
            radius = 4.0;
            if (!db->showTileCentre) continue;
            painter->setPen(QPen(Qt::cyan,1));
            painter->setBrush(Qt::cyan);
            break;
        case PointInfo::Type::Center2:
            radius = 4.0;
            if (!db->showTileCentre) continue;
            painter->setPen(QPen(Qt::magenta,1));
            painter->setBrush(Qt::magenta);
            break;
        }
        QPointF pos = viewT.map(pi.pt);
        painter->drawEllipse(pos, radius, radius);
    }
}

void MapEditorView::drawConstructionLines(QPainter * painter)
{
    if (!db->showConstructionLines)
        return;

    QColor color = (Sys::systemViewController->getViewBackgroundColor() == Qt::white) ? Qt::black : Qt::white;

    painter->setPen(QPen(color,constructionLineWidth));
    for (int i = 0; i < db->constructionLines.size(); i++)
    {
        if (i == db->liftedCLine)
            continue;
        painter->drawLine(viewT.map(db->constructionLines[i]));
    }
}

// drawn last, on top of the map
void MapEditorView::drawSelectedConstructionLine(QPainter * painter)
{
    if (!db->showConstructionLines || db->getMouseMode() != MapEditor::MouseMode::ConstructionLines)
        return;
    if (!db->isCLine(db->selectedCLine) || db->selectedCLine == db->liftedCLine)
        return;

    const QLineF & line = db->constructionLines[db->selectedCLine];
    painter->setPen(QPen(Qt::yellow,constructionLineWidth + 2));
    painter->drawLine(viewT.map(line));

    // hovering in the grab-zone of an endpoint
    if (!db->getMouseInteraction())
    {
        int endpoint = ConstructionLinesView::findEndpointNear(line,viewT,mousePos);
        if (endpoint >= 0)
        {
            QPointF pt = (endpoint == 0) ? line.p1() : line.p2();
            painter->setPen(QPen(Qt::yellow,2));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(viewT.map(pt), 6.0, 6.0);
        }
    }
}

void MapEditorView::drawConstructionCircles(QPainter * painter)
{
    if (!db->showConstructionLines)
        return;

    QColor color = (Sys::systemViewController->getViewBackgroundColor() == Qt::white) ? Qt::black : Qt::white;

    for (const auto & circle :  std::as_const(db->constructionCircles))
    {
        QPointF pt = viewT.map(circle->centre);
        painter->setPen(QPen(color,constructionLineWidth));
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

void MapEditorView::startMouseInteraction(QPointF spt, Qt::MouseButton mouseButton)
{
    if (Sys::cropMakerView->getShowCrop(CM_MAPED))
    {
        return;     // it will be handled by the CropMaker - this is right
    }

    SelectionSet set = selector->findSelectionsUsingDB(spt);
    for (auto sel :  std::as_const(set))
    {
        switch (mouseButton)
        {
        case Qt::LeftButton:
            switch (sel->getType())
            {
            case MapSelection::Type::MAP_VERTEX:
            {
                auto map = editor->getEditMap();
                NeighbourMap  * nmap = new NeighbourMap(map);
                db->setMouseInteraction(std::make_shared<MoveVertex>(nmap,sel->getVertex(), spt));
            }   return;
            case MapSelection::Type::MAP_EDGE:
            {
                auto map = editor->getEditMap();
                NeighbourMap  * nmap = new NeighbourMap(map);
                db->setMouseInteraction(std::make_shared<MoveEdge>(nmap,sel->getEdge(), spt));
            }   return;
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

void MapEditorView::startConstructionLineInteraction(QPointF spt, Qt::MouseButton mouseButton)
{
    int selected = db->isCLine(db->selectedCLine) ? db->selectedCLine : -1;
    int endpoint = (selected >= 0) ? ConstructionLinesView::findEndpointNear(db->constructionLines[selected],viewT,spt) : -1;

    if (mouseButton == Qt::RightButton)
    {
        bool onBody = (selected >= 0) && (ConstructionLinesView::findLineNear(db->constructionLines,viewT,spt) == selected);

        // a line this short on screen is all endpoint grab-zone - offer delete instead of extend
        constexpr qreal shortLineLength = 50.0;
        bool tooShortToExtend = (selected >= 0) && (viewT.map(db->constructionLines[selected]).length() < shortLineLength);

        if (endpoint >= 0 && !tooShortToExtend)
        {
            db->setMouseInteraction(std::make_shared<ExtendConstructionLine>(selected,endpoint,spt));
        }
        else if (endpoint >= 0 || onBody)
        {
            QMenu menu;
            menu.addAction("Delete", [this,selected]
            {
                db->constructionLines.remove(selected);
                db->selectedCLine = -1;
                editor->stash();
                forceRedraw();
            });
            menu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
        }
        else
        {
            db->selectedCLine = ConstructionLinesView::findLineNear(db->constructionLines,viewT,spt);
            forceRedraw();
        }
    }
    else if (mouseButton == Qt::LeftButton)
    {
        if (endpoint >= 0)
        {
            db->setMouseInteraction(std::make_shared<MoveConstructionEndpoint>(selected,endpoint,spt));
            return;
        }

        if (selected >= 0)
        {
            db->selectedCLine = -1;
            forceRedraw();
        }
        SelectionSet set = selector->findSelectionsUsingDB(spt);
        db->setMouseInteraction(std::make_shared<ConstructionLine>(set,spt));
    }
}

//////////////////////////////////////////////////////////////////
///
/// slots and events
///
//////////////////////////////////////////////////////////////////

void MapEditorView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    if (!viewControl()->isEnabled(Type::MapEditor)) return;

    setMousePos(spt);

    if (debugMouse)
    {
        MapMouseActionPtr mma = db->getMouseInteraction();
        if (mma)
            qDebug() << "press start:" << mousePos << screenToModel(mousePos) << mma->desc;
        else
            qDebug() << "press start:" << mousePos << screenToModel(mousePos) << "no mouse interaction";
    }

    SelectionSet set;

    switch (db->getMouseMode())
    {
    case MapEditor::MouseMode::None:
        startMouseInteraction(mousePos,btn);
        break;

    case MapEditor::MouseMode::DrawLine:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(std::make_shared<DrawLine>(set,mousePos));
        break;

    case MapEditor::MouseMode::CreateLine:
    {
        EdgeSet qvep;
        SelectionSet set;
        for (auto & map : editor->getMapLayerMaps())
        {
            selector->findEdges(map,mousePos, qvep, set);
        }
        if (set.size())
        {
            MapPtr map =editor->getEditMap();
            if (map)
            {
                qreal len    = config->mapedLen;
                EdgePtr ep   = set.first()->getEdge();
                QLineF eline = ep->getLine();
                QPointF mid  = ep->getMidPoint();

                // can't use ep below since it becomes modified
                QLineF line  = Geo::createLine(eline,mid,qDegreesToRadians(config->mapedAngle),len);
                VertexPtr v1 = map->insertVertex(line.p1());
                VertexPtr v2 = map->insertVertex(line.p2());
                map->insertEdge(v1,v2);
                
                auto line2  = Geo::createLine(eline,mid,qDegreesToRadians(-config->mapedAngle),len);
                auto v3 = map->insertVertex(line2.p1());
                auto v4 = map->insertVertex(line2.p2());
                map->insertEdge(v3,v4);
            }
            else
            {
                QMessageBox box(Sys::systemViewController->getWidget());
                box.setIcon(QMessageBox::Warning);
                box.setText("Cannot create lines here.\nClick on an existing map line to insert.");
                box.exec();
            }
        }
        else
        {
            QMessageBox box(Sys::systemViewController->getWidget());
            box.setIcon(QMessageBox::Warning);
            box.setText("Cannot create lines here.\nClick on an existing map line to insert.");
            box.exec();
        }
        break;
    }

    case MapEditor::MouseMode::ConstructionLines:
        startConstructionLineInteraction(mousePos,btn);
        break;

    case MapEditor::MouseMode::Delete:
        set = selector->findSelectionsUsingDB(mousePos);
        // only delete one line
        for (const auto & sel : std::as_const(set))
        {
            if (sel->getType() == MapSelection::Type::MAP_EDGE)
            {
                MapPtr map = editor->getEditMap();
                if (map)
                {
                    map->removeEdge(sel->getEdge());

                    MapCleanser mc(map);
                    mc.cleanse(joinupColinearEdges | badVertices_0,Sys::config->mapedMergeSensitivity);

                    MapVerifier mv(map);
                    mv.verify();
                }
                break;
            }
            else if (sel->getType() == MapSelection::Type::MAP_LINE && sel->isConstructionLine())
            {
                QLineF line = sel->getLine();
                db->constructionLines.removeAll(line);
                db->selectedCLine = -1;
                Sys::mapEditor->stash();
                break;
            }
            else if (sel->getType() == MapSelection::Type::MAP_CIRCLE)
            {
                CirclePtr c = sel->getCircle();
                Q_ASSERT(c);
                db->constructionCircles.removeOne(c);
                Sys::mapEditor->stash();
                break;
            }
        }
        forceRedraw();
        //setMapedMouseMode(MAPED_MOUSE_NONE);
        break;

    case MapEditor::MouseMode::SplitLine:
    {
        EdgeSet qvep;
        SelectionSet set;
        for (auto & map : editor->getMapLayerMaps())
        {
            selector->findEdges(map, mousePos, qvep, set);
        }
        if (set.size())
        {
            MapPtr map = editor->getEditMap();
            for (const auto & sel : std::as_const(set))
            {
                map->splitEdge(sel->getEdge());

                MapVerifier mv(map);
                mv.verify();
            }
            forceRedraw();
            //setMapedMouseMode(MAPED_MOUSE_NONE);
        }
    }
    break;

    case MapEditor::MouseMode::ExtendP1:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(std::make_shared<ExtendLineP1>(set,mousePos));
        break;

    case MapEditor::MouseMode::ExtendP2:
        set = selector->findSelectionsUsingDB(mousePos);
        db->setMouseInteraction(std::make_shared<ExtendLineP2>(set,mousePos));
        break;

    case MapEditor::MouseMode::ConstructionCircles:
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
            db->constructionCircles.push_back(std::make_shared<Circle>(center, config->mapedRadius));
            Sys::mapEditor->stash();
            forceRedraw();
        }
        else if (btn == Qt::LeftButton)
        {
            MapSelectionPtr sel = selector->findConstructionCircle(mousePos);
            if (sel)
            {
                CirclePtr c = sel->getCircle();
                if (c)
                {
                    db->setMouseInteraction(std::make_shared<EditConstructionCircle>(c,mousePos));
                }
            }
        }
        break;
    }

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
        qDebug().noquote() << "press end:"  << mma->desc;
    else
        qDebug() << "press end: no mouse_interaction";
}

void MapEditorView::slot_mouseDragged(QPointF spt)
{
    if (!viewControl()->isEnabled(Type::MapEditor)) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToModel(mousePos)  << sMapEditorMouseMode[(int)db->getMouseMode()];

    selector->setCurrentSelections(selector->findSelectionsUsingDB(mousePos));

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->updateDragging(mousePos);
    }

    forceRedraw();
}


void MapEditorView::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(Type::MapEditor)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    selector->setCurrentSelections(selector->findSelectionsUsingDB(mousePos));

    forceRedraw();
}

void MapEditorView::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(Type::MapEditor)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToModel(mousePos);

    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        db->resetMouseInteraction();
    }
}

void MapEditorView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

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


