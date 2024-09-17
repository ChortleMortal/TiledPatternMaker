#include <QMessageBox>
#include <QKeyEvent>
#include <QApplication>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_selection.h"
#include "gui/map_editor/map_editor_selection.h"
#include "model/motifs/explicit_map_motif.h"
#include "gui/viewers/map_editor_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/motifs/motif.h"
#include "model/motifs/irregular_motif.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "gui/panels/shortcuts.h"
#include "sys/sys.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/mosaics/mosaic.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/tile.h"
#include "model/tilings/placed_tile.h"
#include "gui/top/view_controller.h"

using std::make_shared;

MapEditor::MapEditor()
{
//qDebug() << "MapEditor::MapEditor";
}

void MapEditor::init()
{
    db              = new MapEditorDb();
    meView          = Sys::mapEditorView;
    selector        = new MapEditorSelection(db);

    meView->init(db,selector);
    config          = Sys::config;
    mosaicMaker     = Sys::mosaicMaker;
    prototypeMaker  = Sys::prototypeMaker;
    tilingMaker     = Sys::tilingMaker;

    timer           = nullptr;

    unload();

    connect(this, &MapEditor::sig_updateView, Sys::view, &View::slot_update);
    connect(this, &MapEditor::sig_raiseMenu,  Sys::controlPanel, &ControlPanel::slot_raisePanel);
}

MapEditor::~MapEditor()
{
    unload();
    delete selector;
    delete db;
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
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No mosaic found");
        box.exec();
        return false;
    }

    MapPtr map = mosaic->getFirstExistingPrototypeMap();
    if (!map)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Mosaic has no content");
        box.exec();
        return false;
    }

    if (config->mapedLoadCopies)
    {
        map = map->recreate();
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
    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found");
        box.exec();
        return;
     }

    if (!proto->hasContent())
    {
        QMessageBox box(Sys::controlPanel);
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
    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Motif Maker");
        box.exec();
        return false;
     }

    db->setMotfiPrototype(proto);

    auto dels = prototypeMaker->getSelectedDELs(MVD_DELEM);
    if (dels.size() == 0)
    {
        QMessageBox box(Sys::controlPanel);
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
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Warning);
        box.setText("No tiling found");
        box.exec();
        return;
    }

    db->setTiling(tp);

    MapPtr map            = tp->createMapSingle();
    db->importedTilingMap = map;
    db->currentTilingMap  = map->copy();

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
        QMessageBox box(Sys::controlPanel);
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
    emit meView->sig_reconstructView(); // because it can load a background
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

    auto dcel = make_shared<DCEL>(map.get());
    db->setLocalDCEL(dcel);
    map->setDerivedDCEL(dcel);
    return true;
}

bool MapEditor::pushToMosaic(MapEditorLayer & layer)
{
    qDebug().noquote() << "MapEditor::pushToMosaic" << Transform::info(meView->getLayerTransform());
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
        auto editor_map = layer.getMapedLayerMap();

        auto mosaic = mosaicMaker->getMosaic();
        if (mosaic)
        {
            auto mosaic_map  = mosaic->getFirstExistingPrototypeMap();
            auto olddcel     = mosaic_map->getDerivedDCEL();
            if (mosaic_map != editor_map)
            {
                mosaic_map = editor_map;
            }

            if (config->mapEditorMode == MAPED_MODE_DCEL)
            {
                if (!useExistingDCEL(mosaic_map))
                    createLocalDCEL(mosaic_map);
            }
            else
            {
                if (olddcel)
                {
                    auto dcel = std::make_shared<DCEL>(editor_map.get());
                    mosaic_map->setDerivedDCEL(dcel);
                }
            }

            const StyleSet & sset = mosaic->getStyleSet();
            for (auto & style : std::as_const(sset))
            {
                style->resetStyleRepresentation();
            }

            emit meView->sig_reconstructView(); // triggers createSyleRepresentation

            return true;     // done
        }

        // the mosaic needs to be fed a map in to prototype which it can style
        // we could use passing a prototype which has a map but no tiling
        // but lets fake it out by creating an explicit motif from the tile
        // the real issue is having a prototype disconnected from a tiling !

        EdgePoly ep(editor_map->getEdges());
        auto tile = make_shared<Tile>(ep);
        auto motif  = make_shared<ExplicitMapMotif>(editor_map);
        motif->setN(tile->numSides());
        auto del = make_shared<DesignElement>(tile,motif);

        auto proto = make_shared<Prototype>(editor_map);
        proto->addDesignElement(del);
        protos.push_back(proto);
    }

    if (protos.size())
    {
        VersionedName oldname = mosaicMaker->getMosaic()->getName();

        // This makes a new mosaic which is a representation of the new map
        // which in turn was derived from old mosaic
        mosaicMaker->sm_takeUp(protos, MOSM_LOAD_SINGLE);

        mosaicMaker->getMosaic()->setName(oldname);

        emit meView->sig_reconstructView(); // triggers createSyleRepresentation

        // does not switch view to mosaic
        qDebug().noquote() << "MapEditor::pushToMosaic - end" << Transform::info(meView->getLayerTransform());
        return true;
    }

    return false;
}

bool MapEditor::pushToMotif(MapPtr map)
{
    qDebug().noquote() << "MapEditor::convertToMotif" << Transform::info(meView->getLayerTransform());

    eMapEditorMapType mtype = db->getMapType(map);
    if (!db->isMotif(mtype) && mtype != MAPED_TYPE_CREATED)
        return false;

    auto del   = prototypeMaker->getSelectedDEL();
    if (!del)
        return false;

    auto transform = meView->getPlacement(del->getTile()).inverted();
    auto map2      = map->getTransformed(transform);

    auto motif = make_shared<ExplicitMapMotif>(map2);
    motif->setTile(del->getTile());
    del->setMotif(motif);

    prototypeMaker->selectDesignElement(del);

    auto proto =  prototypeMaker->getSelectedPrototype();
    prototypeMaker->sm_takeUp(proto->getTiling(),PROM_MOTIF_CHANGED,del->getTile());

    qDebug().noquote() << "MapEditor::convertToMotif - end" << Transform::info(meView->getLayerTransform());
    return true;
}

// Replaces the complete set of tiles in the tiling unit
bool MapEditor::pushToTiling(MapPtr map, bool outer)
{
    qDebug().noquote() << __FUNCTION__ << Transform::info(meView->getLayerTransform());

    // converts the map to DCELs so that tiles can be made
    createLocalDCEL(map);
    DCELPtr dcel = db->getLocaldDCEL();

    const FaceSet & faces = dcel->getFaceSet();
    qDebug() << "num new faces =" << faces.size();

    TilingUnit tilingUnit;
    for (auto & face : std::as_const(faces))
    {
        if (  (!outer && !face->outer)
            ||( outer &&  face->outer))
        {
            EdgePoly ep     = *face;
            TilePtr fp      = make_shared<Tile>(ep);
            auto placedTile = make_shared<PlacedTile>(fp,QTransform());
            tilingUnit.add(placedTile);
        }
    }

    tilingMaker->replaceTilingUnit(tilingUnit);

    // aligns the tiling maker to the map editor
    const Xform & xf = meView->getModelXform();
    Sys::viewController->getCanvas().setModelAlignment(M_ALIGN_TILING);
    Sys::tilingMakerView->setModelXform(xf,true);

    qDebug().noquote() << __FUNCTION__ << "- end" << Transform::info(meView->getLayerTransform());
    return true;
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
    bool rv = _stash.destash();
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

bool MapEditor::loadNextStash()
{
    _stash.getNext();
    bool rv = loadCurrentStash();
    return rv;
}

bool MapEditor::loadPrevStash()
{
    _stash.getPrev();
    bool rv = loadCurrentStash();
    return rv;
}

bool MapEditor::loadTemplate(VersionedFile xfile, bool animate)
{
    bool rv;
    if (animate)
    {
        rv = _stash.animateReadStash(xfile);
        if (!timer)
        {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [this] { _stash.nextAnimationStep(db,timer); emit sig_updateView(); } );
        }
        timer->start(500);
    }
    else
    {
        rv = _stash.readStash(xfile,db);
    }
    if (!rv) return false;

    forceRedraw();
    _stash.stash(db);

    return true;
}

bool MapEditor::saveTemplate(VersionedName & vname)
{
    return _stash.saveTemplate(vname);

}
bool MapEditor::keepStash(VersionedName vname)
{
    if (db->hasConstructionLines())
        return _stash.saveTemplate(vname);
    else
        return false;
}

bool MapEditor::initStashFrom(VersionedName mosaicname)
{
    // load any stash associated with the newly loaded mosaic
    bool rv = _stash.initStash(mosaicname,db);
    if (rv)
    {
        forceRedraw();
    }
    return rv;
}

void MapEditor::forceRedraw()
{
    if (Sys::view->isActiveLayer(VIEW_MAP_EDITOR))
    {
        emit sig_updateView();
    }
}


/////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (!Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
    {
        return false;
    }

    int key = k->key();

    switch (key)
    {
    // actions
    case 'F': flipLineExtension(); break;
    case 'M': emit sig_raiseMenu(); break;
    case 'Q': emit sig_close(); break;
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
