#include <QMessageBox>
#include <QKeyEvent>
#include <QApplication>

#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "makers/map_editor/map_editor_selection.h"
#include "viewers/map_editor_view.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "figures/figure.h"
#include "figures/explicit_figure.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "misc/shortcuts.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "mosaic/mosaic.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "viewers/viewcontrol.h"

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

MapEditor::MapEditor()
{
    //qDebug() << "MapEditor::MapEditor";
    db              = new MapEditorDb();
    meView          = MapEditorView::getSharedInstance();
    selector        = new MapEditorSelection(db);

    meView->init(db,selector);

    config          = Configuration::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    view            = ViewControl::getInstance();
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

    db->insertLayer(MapEditorLayer(map,MAPED_LOADED_FROM_MOSAIC));

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);
    }

    forceRedraw();

    return true;
}

void  MapEditor::loadMotifPrototype()
{
    PrototypePtr proto = motifMaker->getSelectedPrototype();
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

    MapPtr map  = proto->getProtoMap();
    db->insertLayer(MapEditorLayer(map,MAPED_LOADED_FROM_MOTIF_PROTOTYPE));

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);    }

    forceRedraw();
}

bool  MapEditor::loadSelectedMotifs()
{
    auto proto = motifMaker->getSelectedPrototype();
    if (!proto)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No prototype found in Motif Maker");
        box.exec();
        return false;
     }

    db->setMotfiPrototype(proto);

    auto delps = motifMaker->getSelectedDesignElements();
    if (delps.size() ==0)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Warning);
        box.setText("No design elements found found in Motif Maker's prototype");
        box.exec();
        return false;
    }

    for (auto delp : delps)
    {
        db->getDesignElements().push_back(delp);

        auto figure  = delp->getFigure();
        if (figure)
        {
            MapPtr map  = figure->getFigureMap();
            auto feature = delp->getFeature();
            QTransform placement = meView->getPlacement(feature);
            MapPtr tmap = map->getTransformed(placement);

            db->insertLayer(MapEditorLayer(tmap,MAPED_LOADED_FROM_MOTIF,delp));

            if (config->mapEditorMode == MAPED_MODE_DCEL)
            {
                if (!useExistingDCEL(map))
                    createLocalDCEL(map);            }
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
    db->insertLayer(MapEditorLayer(map,MAPED_LOADED_FROM_TILING_UNIT));

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);    }

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
    db->insertLayer(MapEditorLayer(map,MAPED_LOADED_FROM_TILING_REPEATED));

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);    }

    forceRedraw();
}

void  MapEditor::loadFromMap(MapPtr map, eMapEditorMapType type)
{
    db->insertLayer(MapEditorLayer(map,type));

    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        if (!useExistingDCEL(map))
            createLocalDCEL(map);    }

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
    QVector<PrototypePtr> protos;

    if (layer.type == MAPED_LOADED_FROM_MOTIF_PROTOTYPE)
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
        auto map = layer.getMap();

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
            for (auto style : sset)
            {
                style->resetStyleRepresentation();
            }

            emit meView->sig_refreshView(); // triggers createSyleRepresentation

            return true;     // done
        }

        // the mosaic needs to be fed a map in to prototype whichi it can style
        // we could use passing a prortype which has a map but no tiling
        // but lets fake it out by creating an explicit figure from the feature
        // the real issue is having a prototype disconnected from a tiling !

        EdgePoly ep(map->getEdges());
        auto feature = make_shared<Feature>(ep);
        auto figure  = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT,10);   // FIXME 10 is arbitrary
        auto del = make_shared<DesignElement>(feature,figure);

        auto proto = make_shared<Prototype>(map);
        proto->addElement(del);
        protos.push_back(proto);
    }

    if (protos.size())
    {
        QString oldname = mosaicMaker->getMosaic()->getName();

        // This makes a new mosaic which is a representation of the new map
        // which in turn was derived from old mosaic
        mosaicMaker->sm_takeUp(protos, SM_LOAD_SINGLE);

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
    eMapEditorMapType mtype = db->getMapType(map);
    if (!db->isMotif(mtype) && mtype != MAPED_TYPE_CREATED)
        return false;

    if (!convertToTiling(map,true))
        return false;

    DesignElementPtr delp = motifMaker->getSelectedDesignElement();
    if (!delp)
        return false;

    FigurePtr fig = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT,10); // todo - specify number of sides
    delp->setFigure(fig);

    return true;
}

bool MapEditor::convertToTiling(MapPtr map, bool outer)
{
    eMapEditorMapType mapType = db->getMapType(map);

    // if a tiling map has been modified, this backs out the map, leaving just the additions.
    if (mapType == MAPED_LOADED_FROM_TILING_UNIT && db->getTiling())
    {
        qDebug() << map->summary();
        map->removeMap(db->createdTilingMap);
        qDebug() << map->summary();
        forceRedraw();
    }

    // converts the remaining map to DCELs so that featurees can be made
    createLocalDCEL(map);
    DCELPtr dcel = db->getLocaldDCEL();

    const FaceSet & faces = dcel->getFaceSet();
    qDebug() << "num new faces =" << faces.size();

    QVector<PlacedFeaturePtr> qvec_pf;
    for (auto & face : qAsConst(faces))
    {
        if (  (!outer && !face->outer)
            ||( outer &&  face->outer))
        {
            EdgePoly ep = *face;
            FeaturePtr fp        = make_shared<Feature>(ep);
            PlacedFeaturePtr pfp = make_shared<PlacedFeature>(fp,QTransform());
            qvec_pf.push_back(pfp);
        }
    }

    tilingMaker->addNewPlacedFeatures(qvec_pf);

    // aligns the tiling maker to the map editor
    const Xform & xf = meView->getCanvasXform();
    view->frameSettings.setModelAlignment(M_ALIGN_TILING);
    tilingMaker->setCanvasXform(xf);

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

void MapEditor::updateStatus()
{
    QString s("Map Editor:: ");
    s += sMapEditorMouseMode[db->getMouseMode()];
    MapMouseActionPtr mma  = db->getMouseInteraction();
    if (mma)
    {
        s += "  ";
        s += mma->desc;
    }
    view->setWindowTitle(s);
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
    view->update();
}


/////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (config->getViewerType() != VIEW_MAP_EDITOR)
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
