#include <QMessageBox>
#include <QKeyEvent>
#include <QApplication>

#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "makers/map_editor/map_editor_selection.h"
#include "motifs/explicit_map_motif.h"
#include "viewers/map_editor_view.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "motifs/motif.h"
#include "motifs/irregular_motif.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "misc/shortcuts.h"
#include "misc/sys.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "viewers/view_controller.h"

using std::make_shared;

MapEditor * MapEditor::mpThis = nullptr;

MapEditor * MapEditor::getInstance()
{
    if (!mpThis)
    {
        mpThis = new MapEditor();
    }
    return mpThis;
}

void MapEditor::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

MapEditor::MapEditor()
{
    //qDebug() << "MapEditor::MapEditor";
    db              = new MapEditorDb();
    meView          = MapEditorView::getInstance();
    selector        = new MapEditorSelection(db);

    meView->init(db,selector);

    config          = Configuration::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    view            = Sys::view;
    viewControl     = Sys::viewController;
    cpanel          = ControlPanel::getInstance();

    unload();
}

void MapEditor::unload()
{
    db->reset();
    setMapedMouseMode(MAPED_MOUSE_NONE);
}

bool MapEditor::loadMosaicPrototype()
{
    qDebug() << "MapEditor::loadFromMosaic()";

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No mosaic found");
        box.exec();
        return false;
    }

    MapPtr map = mosaic->getPrototypeMap();
    if (!map)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("Mosaic has no content");
        box.exec();
        return false;
    }

    auto type = db->insertLayer(map,MAPED_LOADED_FROM_MOSAIC);
    if (type != COMPOSITE)
    {
        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }
    forceRedraw();

    return true;
}

void  MapEditor::loadMotifPrototype()
{
    auto data  = prototypeMaker->getProtoMakerData();
    auto proto = data->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found");
        box.exec();
        return;
     }

    if (!proto->hasContent())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("Prototype has no content");
        box.exec();
        return;
    }

    db->setMotfiPrototype(proto);

    MapPtr map = proto->getProtoMap();
    auto type  = db->insertLayer(map,MAPED_LOADED_FROM_MOTIF_PROTOTYPE);

    if (type != COMPOSITE)
    {
        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }
    forceRedraw();
}

bool  MapEditor::loadSelectedMotifs()
{
    auto data  = prototypeMaker->getProtoMakerData();
    auto proto = data->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Motif Maker");
        box.exec();
        return false;
     }

    db->setMotfiPrototype(proto);

    auto dels = data->getSelectedDELs(MVD_DELEM);
    if (dels.size() == 0)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No design elements found found in Motif Maker's prototype");
        box.exec();
        return false;
    }

    for (const auto & del : std::as_const(dels))
    {
        db->getDesignElements().push_back(del);

        auto motif  = del->getMotif();
        if (motif)
        {
            MapPtr map  = motif->getMotifMap();
            auto tile = del->getTile();
            QTransform placement = meView->getPlacement(tile);
            MapPtr tmap = map->getTransformed(placement);

            auto type = db->insertLayer(tmap,MAPED_LOADED_FROM_MOTIF);
            if (type != COMPOSITE)
            {
                MapEditorLayer &layer = db->getLayer(type);
                layer.setDel(del);

                if (config->mapEditorMode == MAPED_MODE_DCEL)
                {
                    if (!useExistingDCEL(map))
                        createLocalDCEL(map);
                }
            }
        }
    }
    forceRedraw();
    return true;
}

void MapEditor::loadTilingUnit()
{
    TilingPtr tp = tilingMaker->getSelected();
    if (!tp || tp->isEmpty())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return;
    }

    db->setTiling(tp);

    MapPtr map       = tp->createMapSingle();
    db->createdTilingMap = map;
    db->currentTilingMap = map->copy();

    auto type = db->insertLayer(map,MAPED_LOADED_FROM_TILING_UNIT);

    if (type != COMPOSITE)
    {
        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }

    forceRedraw();
}

void  MapEditor::loadTilingRepeated()
{
    TilingPtr tp = tilingMaker->getSelected();
    if (!tp || tp->isEmpty())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return;
    }

    db->setTiling(tp);

    MapPtr map           = tp->createMapFullSimple();
    db->currentTilingMap = map;

    auto type = db->insertLayer(map,MAPED_LOADED_FROM_TILING_REPEATED);

    if (type != COMPOSITE)
    {
        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }

    forceRedraw();
}

void  MapEditor::loadFromMap(MapPtr map, eMapEditorMapType mtype)
{
    auto type = db->insertLayer(map,mtype);

    if (type != COMPOSITE)
    {
        if (config->mapEditorMode == MAPED_MODE_DCEL)
        {
            if (!useExistingDCEL(map))
                createLocalDCEL(map);
        }
    }
    emit meView->sig_refreshView(); // because it can load a background
}

bool MapEditor::useExistingDCEL(MapPtr map)
{
    if (!map)
        return false;

    auto dcel = map->getDerivedDCEL();
    if (dcel)
    {
        db->setLocalDCEL(dcel);
        return true;
    }

    return false;
}

bool MapEditor::createLocalDCEL(MapPtr map)
{
    if (!map)
        return false;

    auto dcel = make_shared<DCEL>(map);
    db->setLocalDCEL(dcel);
    map->setDerivedDCEL(dcel);
    return true;
}

bool MapEditor::pushToMosaic(MapEditorLayer & layer)
{
    qDebug().noquote() << "MapEditor::pushToMosaic" << Transform::toInfoString(meView->getLayerTransform());
    QVector<ProtoPtr> protos;

    if (layer.getLayerMapType() == MAPED_LOADED_FROM_MOTIF_PROTOTYPE)
    {
        auto proto = db->getMotifPrototype();
        if (proto)
        {
            protos.push_back(proto);
        }
    }
    else
    {
        // is there an existing prototype?
        auto map = layer.getMapedLayerMap();

        auto mosaic = mosaicMaker->getMosaic();
        if (mosaic)
        {
            auto mosmap = mosaic->getPrototypeMap();
            if (mosmap != map)
            {
                mosmap = map;
            }

            if (config->mapEditorMode == MAPED_MODE_DCEL)
            {
                if (!useExistingDCEL(mosmap))
                    createLocalDCEL(mosmap);
            }

            const StyleSet & sset = mosaic->getStyleSet();
            for (auto & style : std::as_const(sset))
            {
                style->resetStyleRepresentation();
            }

            emit meView->sig_refreshView(); // triggers createSyleRepresentation

            return true;     // done
        }

        // the mosaic needs to be fed a map in to prototype which it can style
        // we could use passing a prototype which has a map but no tiling
        // but lets fake it out by creating an explicit motif from the tile
        // the real issue is having a prototype disconnected from a tiling !

        EdgePoly ep(map->getEdges());
        auto tile = make_shared<Tile>(ep);
        auto motif  = make_shared<ExplicitMapMotif>(map);
        motif->setN(tile->numSides());
        auto del = make_shared<DesignElement>(tile,motif);

        auto proto = make_shared<Prototype>(map);
        proto->addElement(del);
        protos.push_back(proto);
    }

    if (protos.size())
    {
        QString oldname = mosaicMaker->getMosaic()->getName();

        // This makes a new mosaic which is a representation of the new map
        // which in turn was derived from old mosaic
        mosaicMaker->sm_takeUp(protos, MOSM_LOAD_SINGLE);

        mosaicMaker->getMosaic()->setName(oldname);

        emit meView->sig_refreshView(); // triggers createSyleRepresentation

        // does not switch view to mosaic
        qDebug().noquote() << "MapEditor::pushToMosaic - end" << Transform::toInfoString(meView->getLayerTransform());
        return true;
    }

    return false;
}

bool MapEditor::convertToMotif(MapPtr map)
{
    qDebug().noquote() << "MapEditor::convertToMotif" << Transform::toInfoString(meView->getLayerTransform());

    eMapEditorMapType mtype = db->getMapType(map);
    if (!db->isMotif(mtype) && mtype != MAPED_TYPE_CREATED)
        return false;

    if (!convertToTiling(map,true))
        return false;

    auto data  = prototypeMaker->getProtoMakerData();
    auto del   = data->getSelectedDEL();
    if (!del)
        return false;

    auto motif = make_shared<ExplicitMapMotif>(map);
    del->setMotif(motif);

    qDebug().noquote() << "MapEditor::convertToMotif - end" << Transform::toInfoString(meView->getLayerTransform());
    return true;
}

bool MapEditor::convertToTiling(MapPtr map, bool outer)
{
    qDebug().noquote() << "MapEditor::convertToTiling" << Transform::toInfoString(meView->getLayerTransform());

    eMapEditorMapType mapType = db->getMapType(map);

    // if a tiling map has been modified, this backs out the map, leaving just the additions.
    if (mapType == MAPED_LOADED_FROM_TILING_UNIT && db->getTiling())
    {
        qDebug() << map->summary();
        map->removeMap(db->createdTilingMap);
        qDebug() << map->summary();
        forceRedraw();
    }

    // converts the remaining map to DCELs so that tiles can be made
    createLocalDCEL(map);
    DCELPtr dcel = db->getLocaldDCEL();

    const FaceSet & faces = dcel->getFaceSet();
    qDebug() << "num new faces =" << faces.size();

    auto tiling = tilingMaker->getSelected();
    PlacedTiles placedTiles;
    for (auto & face : std::as_const(faces))
    {
        if (  (!outer && !face->outer)
            ||( outer &&  face->outer))
        {
            EdgePoly ep = *face;
            TilePtr fp        = make_shared<Tile>(ep);
            auto placedTile = make_shared<PlacedTile>(fp,QTransform());
            placedTiles.push_back(placedTile);
        }
    }

    tilingMaker->addNewPlacedTiles(placedTiles);

    // aligns the tiling maker to the map editor
    const Xform & xf = meView->getModelXform();
    viewControl->getCanvas().setModelAlignment(M_ALIGN_TILING);
    TilingMakerView::getInstance()->setModelXform(xf,true);

    qDebug().noquote() << "MapEditor::convertToTiling - end" << Transform::toInfoString(meView->getLayerTransform());
    return true;
}

void MapEditor::cleanupMapPoints()
{
    auto map = db->getEditMap();

    qreal tolerance = Configuration::getInstance()->mapedMergeSensitivity;

    MapPtr cleaned = std::make_shared<Map>("Cleanup map");
    qDebug() << map->summary();
    cleaned->mergeMap(map,tolerance);
    qDebug() << cleaned->summary();
    cleaned->deDuplicateVertices(tolerance);

    map->set(cleaned);
}


void MapEditor::setMapedMouseMode(eMapEditorMouseMode mode)
{
    db->setMouseMode(mode);

    db->resetMouseInteraction();      // only needed by border edit

    switch (mode)
    {
    default:
    case MAPED_MOUSE_NONE:
    case MAPED_MOUSE_DRAW_LINE:
    case MAPED_MOUSE_DELETE:
    case MAPED_MOUSE_SPLIT_LINE:
    case MAPED_MOUSE_EXTEND_LINE_P1:
    case MAPED_MOUSE_EXTEND_LINE_P2:
    case MAPED_MOUSE_CREATE_LINE:
        break;

    case MAPED_MOUSE_CONSTRUCTION_LINES:
    case MAPED_MOUSE_CONSTRUCTION_CIRCLES:
        db->showConstructionLines = true;
        break;
    }

    forceRedraw();
}

eMapEditorMouseMode MapEditor::getMouseMode()
{
    return db->getMouseMode();
}

void MapEditor::flipLineExtension()
{
    MapMouseActionPtr mma  = db->getMouseInteraction();
    if (mma)
    {
        ExtendLine * el = dynamic_cast<ExtendLine*>(mma.get());
        if (el)
        {
            el->flipDirection();
        }
    }
}

QString MapEditor::getStatus()
{
    QString s("Map Editor:: ");
    s += sMapEditorMouseMode[db->getMouseMode()];
    MapMouseActionPtr mma  = db->getMouseInteraction();
    if (mma)
    {
        s += "  ";
        s += mma->desc;
    }
    return s;
}

bool MapEditor::loadCurrentStash()
{
    bool rv = db->getStash()->destash();
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

bool MapEditor::loadNamedStash(QString file, bool animate)
{
    bool rv;
    MapEditorStash * stash = db->getStash();
    if (animate)
    {
        rv = stash->animateReadStash(file);
    }
    else
    {
        rv = stash->readStash(file);
    }
    if (!rv) return false;

    forceRedraw();
    db->getStash()->stash();

    return true;
}

bool MapEditor::keepStash(QString name)
{
    MapEditorStash * stash = db->getStash();
    bool rv = stash->keepStash(name);
    return rv;
}

bool MapEditor::initStashFrom(QString name)
{
    MapEditorStash * stash = db->getStash();
    return stash->initStash(name);
}

void MapEditor::forceRedraw()
{
    if (view->isActiveLayer(meView))
    {
        view->update();
    }
}


/////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (!viewControl->isEnabled(VIEW_MAP_EDITOR))
    {
        return false;
    }

    int key = k->key();

    switch (key)
    {
    // actions
    case 'F': flipLineExtension(); break;
    case 'M': emit view->sig_raiseMenu(); break;
    case 'Q': QApplication::quit(); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Map Editor Shortcuts");
        box->setText(Shortcuts::getMapEditorShortcuts());
        box->setModal(false);
        box->show();
        break;
    }

        // modes
    case Qt::Key_Escape: setMapedMouseMode(MAPED_MOUSE_NONE);  return false; // propagate
    case Qt::Key_F3:     setMapedMouseMode(MAPED_MOUSE_DRAW_LINE); break;
    case Qt::Key_F4:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_LINES); break;
    case Qt::Key_F5:     setMapedMouseMode(MAPED_MOUSE_DELETE); break;
    case Qt::Key_F6:     setMapedMouseMode(MAPED_MOUSE_SPLIT_LINE); break;
    case Qt::Key_F7:     setMapedMouseMode(MAPED_MOUSE_EXTEND_LINE_P1); break;
    case Qt::Key_F9:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_CIRCLES); break;

    default: return false;
    }

    return true;
}
