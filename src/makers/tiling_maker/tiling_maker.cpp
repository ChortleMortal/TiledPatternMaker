////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly tileful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#include <QMessageBox>
#include <QMenu>
#include <QKeyEvent>
#include <QApplication>

#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tile_selection.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "misc/cycler.h"
#include "misc/shortcuts.h"
#include "geometry/edge.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"
#include "widgets/dlg_edgepoly_edit.h"
#include "widgets/dlg_magnitude.h"

using std::make_shared;

TilingMakerPtr TilingMaker::spThis;


TilingMakerPtr TilingMaker::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<TilingMaker>();
    }
    return spThis;
}


TilingMaker::TilingMaker() : TilingMakerView(this)
{
    //qDebug() << "TilingMaker::TilingMaker";
    debugMouse = false;
}

void TilingMaker::init()
{
    view            = ViewControl::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
    maped           = MapEditor::getInstance();
    controlPanel    = ControlPanel::getInstance();
    auto cycler     = Cycler::getInstance();

    connect(cycler,    &Cycler::sig_cycleLoadTiling,    this,  &TilingMaker::slot_cyclerLoadTiling);
    connect(this,      &TilingMaker::sig_cycler_ready,  cycler, &Cycler::slot_ready);

    tilingMakerMouseMode = TM_NO_MOUSE_MODE;
    poly_side_count = config->polySides;
    poly_rotation   = 0.0;
    filled          = false;

    unload();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    sm_takeUp(tiling,TILM_LOAD_EMPTY);
}

TilingPtr TilingMaker::findTilingByName(QString name)
{
    for (auto tiling : qAsConst(tilings))
    {
        if (tiling->getName() == name)
        {
            return tiling;
        }
    }
    TilingPtr tp;
    return tp;
}

bool  TilingMaker::isLoaded(QString name)
{
    TilingPtr tp = findTilingByName(name);
    if (tp)
        return true;
    else
        return false;
}

bool TilingMaker::isValidTiling(TilingPtr tiling)
{
    return tilings.contains(tiling);
}

// called by menu to clear tiling
void TilingMaker::removeTiling(TilingPtr tp)
{
    tilings.removeOne(tp);
    if (tp == selectedTiling)
    {
        if (tilings.size() == 0)
        {
            // there is always at least an emtpy tiling
            TilingPtr tiling = make_shared<Tiling>();
            sm_takeUp(tiling,TILM_LOAD_EMPTY);
        }
        else
        {
            select(tilings.first());
        }
    }
}

void TilingMaker::unload()
{
    eraseTilings();
}

void TilingMaker::eraseTilings()
{
    tilings.clear();
    selectedTiling.reset();
}

void TilingMaker::slot_loadTiling(QString name, eTILM_Event event)
{
    controlPanel->setLoadState(ControlPanel::LOADING_TILING,name);

    TilingManager tm;
    if (tm.loadTiling(name,event))
    {
        config->lastLoadedTileName = name;

        view->removeAllImages();

        emit sig_cycler_ready();
        emit sig_tilingLoaded(name);
        emit sig_refreshView();
    }
    else
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Tile <%1> NOT FOUND").arg(name));
        box.exec();
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TilingMaker::slot_cyclerLoadTiling(QString name)
{
    controlPanel->setLoadState(ControlPanel::LOADING_TILING,name);

    TilingManager tm;
    if (tm.loadTiling(name,TILM_LOAD_SINGLE))
    {
        config->lastLoadedTileName = name;

        view->removeAllImages();

        emit sig_refreshView();
        emit sig_cycler_ready();
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TilingMaker::slot_saveTiling(QString name)
{
    TilingPtr tiling = getSelected();

    TilingManager tm;
    tm.saveTiling(name,tiling); // Tiling Manager haandles OK/FAIL status
}



// call this when Prototype needs to be rebuilt
void TilingMaker::pushTilingToPrototypeMaker(ePROM_Event event)
{
    prototypeMaker->sm_takeUp(selectedTiling, event);
}

void TilingMaker::pushTileToPrototypeMaker(ePROM_Event event, TilePtr tile)
{
    prototypeMaker->sm_takeUp(selectedTiling,event,tile);
}

/* there are three possible actions
   1. reset all and add
   2. erase current and add
   3. add
   (future - delete)
*/

void TilingMaker::sm_resetAllAndAdd(TilingPtr tiling)
{
   eraseTilings();
   tilings.push_front(tiling);
   select(tiling);
}

void TilingMaker::sm_resetCurrentAndAdd(TilingPtr tiling)
{
    int i = tilings.indexOf(selectedTiling);
    tilings.replace(i,tiling);
    select(tiling);
}

void TilingMaker::sm_add(TilingPtr tiling)
{
    tilings.push_front(tiling);
    select(tiling);
}

void TilingMaker::sm_takeDown(QVector<TilingPtr> & proto_tilings, eTILM_Event event)
{
    switch (event)
    {
    case TILM_LOAD_FROM_MOSAIC:
        filled = false;
        tilings = proto_tilings;
        if (tilings.count())
        {
            select(tilings.first());
        }
        break;

    default:
        qCritical("bad call to TilingMaker::sm_takeDown");
    }
}

void TilingMaker::sm_takeUp(TilingPtr tiling, eTILM_Event event)
{
    eTMState state = sm_getState();
    qInfo().noquote() << "TilingMaker::take() state:" << tm_states[state]  << "event:" <<  sTILM_Events[event];

    switch(event)
    {
    case TILM_LOAD_EMPTY:
        filled = false;
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        prototypeMaker->sm_takeUp(tiling, PROM_LOAD_EMPTY);
        break;

    case TILM_LOAD_SINGLE:
        filled = false;
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        prototypeMaker->sm_takeUp(tiling, PROM_LOAD_SINGLE);
        break;

    case TILM_LOAD_MULTI:
        filled = false;
        clearConstructionLines();
        if (state == TM_EMPTY)
        {
            sm_resetAllAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_LOAD_SINGLE);
        }
        else if (state == TM_SINGLE)
        {
            sm_add(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_LOAD_MULTI);
        }
        else if (state == TM_MULTI)
        {
            sm_add(tiling);
            prototypeMaker->sm_takeUp(tiling,PROM_LOAD_MULTI);
        }
        break;

    case TILM_RELOAD:
        clearConstructionLines();
        filled = false;
        if (state == TM_EMPTY)
        {
            qWarning("Trying to modify when tiling not loaded");
            sm_takeUp(tiling,TILM_LOAD_SINGLE);
        }
        else if (state == TM_SINGLE)
        {
            sm_resetAllAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling, PROM_RELOAD_SINGLE);
        }
        else if (state == TM_MULTI)
        {
            sm_resetCurrentAndAdd(tiling);
            prototypeMaker->sm_takeUp(tiling,PROM_RELOAD_MULTI);
        }
        break;

    case TILM_LOAD_FROM_MOSAIC:
        qCritical("use takeDown for TILM_LOAD_FROM_MOSAIC");
        break;
    }
}

void TilingMaker::select(TilingPtr tiling, bool force)
{
    if (!force && (tiling == selectedTiling))
       return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling:" << tiling->getName();

    clearMakerData();

    setupMaker(tiling);

    if (config->getViewerType() == VIEW_TILING_MAKER)
    {
        forceLayerRecalc();
    }

    selectedTiling = tiling;
#if 0
    ProtoPtr proto = prototypeMaker->getPrototype(tiling);
    if (proto)
    {
        prototypeMaker->setSelectedPrototype(proto);
    }
#endif
}

void TilingMaker::select(ProtoPtr prototype)
{
    TilingPtr tiling = prototype->getTiling();

    if (tiling == selectedTiling)
        return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling from prototype:" << tiling->getName();

    clearMakerData();

    setupMaker(tiling);

    if (config->getViewerType() == VIEW_TILING_MAKER)
    {
        forceLayerRecalc();
    }

    selectedTiling = tiling;
}

eTMState TilingMaker::sm_getState()
{
    if (tilings.size() > 1)
    {
        return  TM_MULTI;
    }
    else if (tilings.size() == 0)
    {
        return TM_EMPTY;
    }
    else
    {
        Q_ASSERT(tilings.size() == 1);
        return TM_SINGLE;
    }
}

bool TilingMaker::sm_askAdd()
{
    // warn
    QMessageBox box(ControlPanel::getInstance());
    box.setIcon(QMessageBox::Question);
    box.setText("Loading a tiling in multi-mode adds an addtional tiling. Is this what you want?");
    QPushButton *addButton     = box.addButton("YES - Continue to add another tiling", QMessageBox::ActionRole);
    QPushButton *replaceButton = box.addButton("NO  - Replace existing tiling", QMessageBox::ActionRole);
    Q_UNUSED(replaceButton);
    box.exec();

    bool add = (box.clickedButton() == addButton);
    return add;
}

void TilingMaker::clearMakerData()
{
    qDebug() << "TilingMaker::clearMakerData";
    allPlacedTiles.clear();
    in_tiling.clear();
    for (auto m : wMeasurements)
    {
        delete m;
    }
    wMeasurements.clear();
    overlapping.clear();
    touching.clear();

    selectedTiling.reset();
    tileSelector.reset();
    resetCurrentPlacedTile();
    editPlacedTile.reset();
    mouse_interaction.reset();
    clickedSelector.reset();

    tilingMakerMouseMode    = TM_NO_MOUSE_MODE;
}

void TilingMaker::setupMaker(TilingPtr tiling)
{
    auto & placedTiles = tiling->getData().getPlacedTiles();
    for(auto it = placedTiles.begin(); it != placedTiles.end(); it++)
    {
        PlacedTilePtr pf = *it;
        allPlacedTiles.push_back(pf);
        in_tiling.push_back(pf);
    }

    QPointF trans_origin;
    if (allPlacedTiles.size() > 0)
    {
        PlacedTilePtr pf = allPlacedTiles.first();  // at this time allplacedTile and in_tiling are the same
        QTransform T        = pf->getTransform();
        trans_origin        = T.map(pf->getTile()->getCenter());
    }

    visibleT1.setP1(trans_origin);
    visibleT1.setP2(trans_origin + tiling->getData().getTrans1());

    visibleT2.setP1(trans_origin);
    visibleT2.setP2(trans_origin + tiling->getData().getTrans2());

    view->frameSettings.initialise(VIEW_TILING_MAKER,tiling->getData().getSettings().getSize(),tiling->getData().getSettings().getZSize());
}

void TilingMaker::updateTilingplacedTiles()
{
    if (in_tiling != selectedTiling->getData().getPlacedTiles())
    {
        auto & placed = selectedTiling->getDataAccess(true).getPlacedTileAccess();
        placed = in_tiling;
    }
}

bool TilingMaker::verifyTiling()
{
    if (allPlacedTiles.size() <= 0)
    {
        qWarning("There are no polygons");
        return false;
    }

    if (in_tiling.size() <= 0)
    {
        qWarning("No selected tiles");
        return false;
    }

    TilingPtr tp = getSelected();
    const FillData & fd = tp->getData().getFillData();
    int minX, maxX, minY, maxY;
    bool singleton;
    fd.get(singleton,minX, maxX, minY, maxY);

    if (!singleton)
    {
        if (tp->getData().getTrans1().isNull() || tp->getData().getTrans2().isNull())
        {
            qWarning("Translation vectors not defined");
            return false;
        }
    }

    return true;
}

void TilingMaker::setTilingMakerMouseMode(eTilingMakerMouseMode mode)
{
    tilingMakerMouseMode = mode;

    if (mode == TM_EDIT_TILE_MODE)
    {
        editPlacedTile = currentPlacedTile;
    }
    else
    {
        editPlacedTile.reset();
    }

    wAccum.clear();

    forceRedraw();
}

eTilingMakerMouseMode TilingMaker::getTilingMakerMouseMode()
{
    return tilingMakerMouseMode;
}

// tile management.

void TilingMaker::addNewPlacedTile(PlacedTilePtr placedTile)
{
    allPlacedTiles.push_front(placedTile);   // push_front so it (the new) becomes selected for move
    addInTiling(placedTile);                    // 03AUG21 - additions are always placed in tiling (they can be removed)
    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::addNewPlacedTiles(PlacedTiles & placedTiles)
{
    allPlacedTiles += placedTiles;
    addInTilings(placedTiles);                    // 03AUG21 - additions are always placed in tiling (they can be removed)
    forceRedraw();
    emit sig_buildMenu();
}

TileSelectorPtr TilingMaker::addTileSelectionPointer(TileSelectorPtr sel)
{
    PlacedTilePtr pf    = sel->getPlacedTile();
    PlacedTilePtr pfnew = make_shared<PlacedTile>(pf->getTile(), pf->getTransform());
    addNewPlacedTile(pfnew);
    
    TileSelectorPtr ret;
    switch (sel->getType())
    {
    case ARC_POINT:
    case FEAT_CENTER:
    case SCREEN_POINT:
        break;
    case INTERIOR:
        ret = make_shared<InteriorTilleSelector>(pfnew);
        break;
    case EDGE:
        ret = make_shared<EdgeTileSelector>(pfnew,sel->getModelEdge());  // FIXME - why is this model edge
        break;
    case VERTEX:
        ret = make_shared<VertexTileSelector>(pfnew,sel->getModelPoint());
        break;
    case MID_POINT:
        ret = make_shared<MidPointTileSelector>(pfnew,sel->getModelEdge(),sel->getModelPoint());
        break;
    }

    emit sig_buildMenu();

    return ret;
}

QVector<TilePtr> TilingMaker::getUniqueTiles()
{
    UniqueQVector<TilePtr> fs;

    for (auto pfp : qAsConst(in_tiling))
    {
        TilePtr fp = pfp->getTile();
        fs.push_back(fp);
    }

    return static_cast<QVector<TilePtr>>(fs);
}

void TilingMaker::slot_deleteTile()
{
    if (clickedSelector)
    {
        deleteTile(clickedSelector);
        clickedSelector.reset();
    }
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::deleteTile(TileSelectorPtr sel)
{
    if (!sel) return;

    PlacedTilePtr tile = sel->getPlacedTile();
    if (tileSelector &&  tileSelector->getPlacedTile() == tile)
    {
        tileSelector.reset();
    }

    if (tile)
    {
        deleteTile(tile);

        emit sig_buildMenu();
        forceRedraw();
    }
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::deleteTile(PlacedTilePtr pf)
{
    if (pf == currentPlacedTile)
    {
        resetCurrentPlacedTile();
    }
    allPlacedTiles.removeOne(pf);
    in_tiling.removeOne(pf);
    updateTilingplacedTiles();
    forceRedraw();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::addInTiling(PlacedTilePtr pf)
{
    in_tiling.push_back(pf);
    updateTilingplacedTiles();
    pushTilingToPrototypeMaker(PROM_TILING_ADDED);
}

void TilingMaker::addInTilings(PlacedTiles & placedTiles)
{
    in_tiling += placedTiles;
    updateTilingplacedTiles();
    pushTilingToPrototypeMaker(PROM_TILING_ADDED);
}

void TilingMaker::removeFromInTiling(PlacedTilePtr pf)
{
    in_tiling.removeOne(pf);
    updateTilingplacedTiles();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::updateVectors()
{
    updateVisibleVectors();
    refillUsingTranslations();
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);

    MapEditorDb * db = maped->getDb();
    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        maped->loadTilingRepeated();
    }
}

void TilingMaker::updateReps()
{
   refillUsingTranslations();
   pushTilingToPrototypeMaker(PROM_TILING_CHANGED);

   MapEditorDb * db = maped->getDb();
   if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
   {
       maped->loadTilingRepeated();
   }
}

void TilingMaker::fillUsingTranslations()
{
    if (!verifyTiling())
        return;

    removeExcluded();

    if (!filled)
    {
        createFillCopies();
        filled = true;
    }
    else
    {
        filled = false;
    }

    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::refillUsingTranslations()
{

    if (filled)
    {
        filled = false;
        fillUsingTranslations();
    }
}

void TilingMaker::removeExcluded()
{
    PlacedTiles toRemove;

    for (auto& pf : allPlacedTiles)
    {
        if (!in_tiling.contains(pf))
        {
            toRemove.push_back(pf);
        }
    }

    for (auto& pf : toRemove)
    {
        allPlacedTiles.removeAll(pf);
    }

    tileSelector.reset();
    resetCurrentPlacedTile();

    forceRedraw();
    emit sig_buildMenu();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::excludeAll()
{
    in_tiling.clear();
    forceRedraw();
    emit sig_buildMenu();
    pushTilingToPrototypeMaker(PROM_TILING_DELETED);
}

void TilingMaker::slot_includeTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        if(!in_tiling.contains(pf))
        {
            addInTiling( pf );
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILING_ADDED);
        }
    }
}

void TilingMaker::slot_excludeTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        if (in_tiling.contains(pf))
        {
            removeFromInTiling(pf);
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            pushTilingToPrototypeMaker(PROM_TILING_DELETED);
        }
    }
}

void TilingMaker::slot_editTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pfp = clickedSelector->getPlacedTile();
        TilePtr      tile = pfp->getTile();
        QTransform      t = pfp->getTransform();

        DlgEdgePolyEdit * fe  = new DlgEdgePolyEdit(tile ,t);
        fe->show();
        fe->raise();
        fe->activateWindow();

        connect(fe,   &DlgEdgePolyEdit::sig_currentPoint, this, &TilingMaker::setTileEditPoint, Qt::UniqueConnection);
        connect(this, &TilingMaker::sig_refreshMenu,      fe,   &DlgEdgePolyEdit::display,      Qt::UniqueConnection);
    }
}

void TilingMaker::slot_copyMoveTile()
{
    if (clickedSelector)
    {
        mouse_interaction = make_shared<CopyMovePolygon>(this, clickedSelector, clickedSpt);
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        TilePtr tile2 = tile->recreate();  // creates a new tile same as other
        pf->setTile(tile2);
        emit sig_buildMenu();
        pushTilingToPrototypeMaker(PROM_TILING_ADDED);
    }
}

void TilingMaker::slot_convertTile()
{
    if (clickedSelector)
    {
        PlacedTilePtr pf = clickedSelector->getPlacedTile();
        TilePtr tile = pf->getTile();
        flipTileRegularity(tile);
        emit sig_buildMenu();
    }
}

void TilingMaker::flipTileRegularity(TilePtr tile)
{
    tile->flipRegularity();
    pushTileToPrototypeMaker(PROM_TILE_REGULARITY_CHANGED,tile);
}

void TilingMaker::createFillCopies()
{
    // Create copies of the tile to help visualise the result in the panel.
    const FillData fd = getSelected()->getData().getFillData();
    int minX, maxX, minY, maxY;
    bool singleton;
    fd.get(singleton,minX, maxX, minY, maxY);

    if (singleton)
    {
        return;
    }

    QPointF t1    = getSelected()->getData().getTrans1();
    QPointF t2    = getSelected()->getData().getTrans2();

    if (t1.isNull() || t2.isNull())
    {
        return;
    }

    for (auto pf : qAsConst(in_tiling))
    {
        if (!pf->show())
        {
            continue;
        }
        TilePtr f = pf->getTile();
        QTransform T = pf->getTransform();

        for( int y = minY; y <= maxY; ++y )
        {
            for( int x = minX; x <= maxX; ++x )
            {
                if ( y == 0 && x == 0 )
                    continue;
                QPointF pt = (t1*x) + (t2 * y);
                QTransform tt = QTransform::fromTranslate(pt.x(),pt.y());
                QTransform placement= T * tt;
                allPlacedTiles.push_back(make_shared<PlacedTile>(f, placement));
            }
        }
    }
    forceRedraw();
}

void TilingMaker::tilingDeltaX(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedTiles))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaY(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedTiles))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaScale(int delta)
{
    Q_ASSERT(view->getKbdMode(KBD_MODE_XFORM_TILING));
    qreal scale = 1.0 + (0.01 * delta);
    for (auto pfp : qAsConst(allPlacedTiles))
    {
        QTransform t = pfp->getTransform();
        qDebug() << "t0" << Transform::toInfoString(t);
        QTransform t1 = t.scale(scale,scale);

        t = pfp->getTransform();
        QTransform t2 = t *QTransform::fromScale(scale,scale);

        qDebug() << "t1" << Transform::toInfoString(t1);
        qDebug() << "t2" << Transform::toInfoString(t2);

        t = pfp->getTransform();
        // scales position too
        t *= QTransform::fromScale(scale,scale);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedTiles))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setTransform(t);
    }

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaX(int delta)
{
    if (!currentPlacedTile)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedTile->getTransform();
    t *= QTransform::fromTranslate(qdelta,0.0);
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaY(int delta)
{
    if (!currentPlacedTile)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedTile->getTransform();
    t *= QTransform::fromTranslate(0.0,qdelta);
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);
    placedTileDeltaScale(scale);
}

void TilingMaker::placedTileDeltaScale(qreal scale)
{
    if (!currentPlacedTile)
        return;

    QPolygonF pts = currentPlacedTile->getPlacedPoints();
    QPointF center = Point::center(pts);

    QTransform ts;
    ts.scale(scale,scale);
    Xform xf(ts);
    xf.setModelCenter(center);
    ts = xf.toQTransform(QTransform());

    QTransform t = currentPlacedTile->getTransform();
    t *= ts;
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    placedTileDeltaRotate(qdelta);
}

void TilingMaker::placedTileDeltaRotate(qreal rotate)
{
    if (!currentPlacedTile)
        return;

    QPolygonF pts = currentPlacedTile->getPlacedPoints();
    QPointF center = Point::center(pts);

    Xform xf(QTransform().rotate(rotate));
    xf.setModelCenter(center);
    QTransform tr = xf.toQTransform(QTransform());

    QTransform t = currentPlacedTile->getTransform();
    t *= tr;
    currentPlacedTile->setTransform(t);

    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::uniqueTileDeltaScale(int delta)
{
    qreal scale = 0.01 * delta;
    uniqueTileDeltaScale(scale);
}

void TilingMaker::uniqueTileDeltaScale(qreal scale)
{
    if (!currentPlacedTile)
        return;

    TilePtr tile = currentPlacedTile->getTile();

    if (!tile)
        return;

    tile->deltaScale(scale);
}

void TilingMaker::uniqueTileDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    uniqueTileDeltaRotate(qdelta);
}

void TilingMaker::uniqueTileDeltaRotate(qreal rotate)
{
    if (!currentPlacedTile)
        return;

    TilePtr tile = currentPlacedTile->getTile();
    if (!tile)
        return;

    tile->deltaRotation(rotate);
}


////////////////////////////////////////////////////////////////////////////
//
// Tiling translation vector.
//
// We treat the two vector as a circular buffer so that each one will
// get over-written in turn, so that both can be alternatively changed
// by the end-user.
// Casper: given this reasoning a sheet of writing paper is a circlular buffer
// IMHO he was thinking of a ping-pong

void TilingMaker::addToTranslate(QLineF mLine)
{
    static bool setT1 = true;

    QPointF tran = mLine.p2() - mLine.p1();

    if (setT1)
    {
        setT1      = false;
        visibleT1  = mLine;
        getSelected()->getDataAccess(true).setTrans1(tran);
    }
    else
    {
        setT1      = true;
        visibleT2  = mLine;
        getSelected()->getDataAccess(true).setTrans2(tran);
    }
    emit sig_refreshMenu();
    pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
}

void TilingMaker::updateVisibleVectors()
{
    TilingPtr tp = getSelected();
    visibleT1 = QLineF(visibleT1.p1(), visibleT1.p1() + tp->getData().getTrans1());
    visibleT2 = QLineF(visibleT2.p1(), visibleT2.p1() + tp->getData().getTrans2());
    forceRedraw();
}

void TilingMaker::toggleInclusion(TileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTilePtr pf = sel->getPlacedTile();
        if (!pf) return;
        if( in_tiling.contains(pf))
        {
            removeFromInTiling(pf); // pushed
        }
        else
        {
            addInTiling(pf);        // pushes
        }
        forceRedraw();
        emit sig_buildMenu();
    }
}


void TilingMaker::clearTranslationVectors()
{
    visibleT1 = QLineF();
    visibleT2 = QLineF();
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::updatePolygonSides(int number)
{
    poly_side_count   = number;
    config->polySides = number;
}

void TilingMaker::updatePolygonRot(qreal angle)
{
    poly_rotation = angle;
}

void TilingMaker::addRegularPolygon()
{
    TilePtr f;
    if (poly_side_count > 2)
    {
        f = make_shared<Tile>(poly_side_count,poly_rotation);
    }
    else
    {
        Circle c(QPointF(0,0), 1.0);
        EdgePoly ep;
        ep.set(c);

        f = make_shared<Tile>(ep,poly_rotation,1.0);
    }

    QTransform t;
    addNewPlacedTile(make_shared<PlacedTile>(f,t));
    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::mirrorPolygonX(TileSelectorPtr sel )
{
    if (sel)
    {
        PlacedTilePtr pfp = sel->getPlacedTile();
        if (!pfp) return;
        selectedTiling->pushStack();
        EdgePoly & ep = pfp->getTile()->getEdgePoly();
#if 1
        QTransform t = QTransform::fromScale(-1,1);
        ep.mapD(t);
#else
        QPolygonF pts = ep.getPoly();
        qreal x = Point::center(pts).x();
        for (auto edge : ep)
        {
            QPointF pos = edge->getV1()->pt;
            qreal px = pos.x();
            qreal diff = px-x;
            px -= (diff *2);
            pos.setX(px);
            edge->getV1()->setPosition(pos);
            if (edge->getType() == EDGETYPE_CURVE)
            {
                QPointF pos = edge->getArcCenter();
                qreal px = pos.x();
                qreal diff = px-x;
                px -= (diff *2);
                pos.setX(px);
                edge->setArcCenter(pos,edge->isConvex());
            }
        }
        ep.reverseWindingOrder();
#endif
        pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,pfp->getTile());
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::mirrorPolygonY(TileSelectorPtr sel )
{
    if (sel)
    {
        PlacedTilePtr ptp = sel->getPlacedTile();
        selectedTiling->pushStack();
        EdgePoly & ep = ptp->getTile()->getEdgePoly();
        QTransform t = QTransform::fromScale(1,-1);
        ep.mapD(t);
        pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,ptp->getTile());
        forceRedraw();
        emit sig_refreshMenu();
    }
}

bool TilingMaker::reflectPolygon(TileSelectorPtr sel)
{
    if (sel)
    {
        TileSelectorPtr sel2 = findEdge(sMousePos);
        if (sel2)
        {
            if (sel2->getType() == MID_POINT || sel2->getType() == EDGE)
            {
                PlacedTilePtr ptp = sel->getPlacedTile();
                selectedTiling->pushStack();
                EdgePoly & ep   = ptp->getTile()->getEdgePoly();
                QPolygonF poly  = ep.getPoly();
                QLineF line     = sel2->getModelLine();
                QPolygonF poly2 = Point::reflectPolygon(poly,line);
                ep.set(poly2);
                // TODO - this reflects the polygon itself
                // TODO - it should set transform of placed poly and not require uniquify

                pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,ptp->getTile());
                forceRedraw();
                emit sig_refreshMenu();
                return true;
            }
            else
            {
                qInfo() << "reflection ignored - edge type = " << sel->getTypeString();
            }
        }
        else
        {
            qInfo() << "reflection ignored - not an edge";
        }
    }
    return false;
}

void TilingMaker::copyPolygon(TileSelectorPtr sel)
{
    if (sel)
    {
        PlacedTilePtr pf = sel->getPlacedTile();
        addNewPlacedTile(make_shared<PlacedTile>(pf->getTile(), pf->getTransform())); // pushes
        forceRedraw();
        emit sig_buildMenu();
    }
}

TileSelectorPtr TilingMaker::findTileUnderMouse()
{
    return findTile(sMousePos);
}

void TilingMaker::updateUnderMouse(QPointF spt)
{
    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
    case TM_MEASURE_MODE:
    case TM_COPY_MODE:
    case TM_DELETE_MODE:
    case TM_INCLUSION_MODE:
    case TM_EDIT_TILE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_REFLECT_EDGE:
        tileSelector = findSelection(spt);
        if (tileSelector)
            forceRedraw();  // NOTE this triggers a lot of repainting
        break;

    case TM_TRANSLATION_VECTOR_MODE:
    case TM_CONSTRUCTION_LINES:
        tileSelector = findSelection(spt);
        if (tileSelector)
            forceRedraw();
        break;

    case TM_DRAW_POLY_MODE:
        tileSelector = findVertex(spt);
        if (!tileSelector)
        {
            tileSelector = findMidPoint(spt);
        }
        if (!tileSelector)
        {
            tileSelector = findNearGridPoint(spt);
        }
        forceRedraw();
        break;

    case TM_POSITION_MODE:
        if (!mouse_interaction)
        {
            mouse_interaction = make_shared<Position>(this, spt);
        }
        else
        {
            // unusual use case
            mouse_interaction->updateDragging(spt);
        }
        break;

    case TM_EDGE_CURVE_MODE:
        tileSelector = findArcPoint(spt);
        if (tileSelector)
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            tileSelector = findEdge(spt);
            if (tileSelector)
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                tileSelector.reset();
            }
        }
        forceRedraw();
        break;
    }
}

void TilingMaker::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    Q_ASSERT(tilingMakerMouseMode == TM_NO_MOUSE_MODE);
    
    TileSelectorPtr sel = findSelection(spt);
    if(sel)
    {
        // do this first
        wAccum.clear();
        mouse_interaction.reset();

        switch (mouseButton)
        {
        case Qt::LeftButton:
            qDebug().noquote() << "left button selection:" << sel->getTypeString();
            switch (sel->getType())
            {
            case VERTEX:
            case FEAT_CENTER:
                mouse_interaction = make_shared<JoinPoint>(this,sel, spt);
                break;
            case MID_POINT:
                mouse_interaction = make_shared<JoinMidPoint>(this,sel, spt);
                break;
            case EDGE:
                mouse_interaction = make_shared<JoinEdge>(this, sel, spt);
                break;
            case INTERIOR:
                setCurrentPlacedTile(sel->getPlacedTile());
                mouse_interaction = make_shared<MovePolygon>(this, sel, spt);
                break;
            case ARC_POINT:
                break;
            case SCREEN_POINT:
                break;
            }
            break;

        case Qt::MiddleButton:
            mouse_interaction = make_shared<DrawTranslation>(this, sel, spt, layerPen);
            break;

        case Qt::RightButton:
            qDebug().noquote() << "right button selection:" << sel->getTypeString();
            switch (sel->getType())
            {
            case VERTEX:
            case FEAT_CENTER:
                mouse_interaction = make_shared<CopyJoinPoint>(this, sel, spt);
                break;
            case MID_POINT:
                mouse_interaction = make_shared<CopyJoinMidPoint>(this, sel, spt);
                break;
            case EDGE:
                mouse_interaction = make_shared<CopyJoinEdge>(this, sel, spt);
                break;
            case INTERIOR:
            {
                clickedSelector = sel;    // save
                clickedSpt      = spt;    // save
                PlacedTilePtr tile = sel->getPlacedTile();
                QMenu myMenu;
                myMenu.addSection("Options");
                myMenu.addSeparator();
                myMenu.addAction("Copy/Move", this, &TilingMaker::slot_copyMoveTile);
                myMenu.addAction("Edit Tile", this, &TilingMaker::slot_editTile);
                if (isIncluded(tile))
                {
                    myMenu.addAction("Exclude", this, &TilingMaker::slot_excludeTile);
                }
                else
                {
                    myMenu.addAction("Include", this, &TilingMaker::slot_includeTile);
                }
                myMenu.addAction("Delete", this, &TilingMaker::slot_deleteTile);
                myMenu.addAction("Uniquify", this, &TilingMaker::slot_uniquifyTile);
                if (tile->getTile()->isRegular())
                {
                    myMenu.addAction("Make irregular", this, &TilingMaker::slot_convertTile);
                }
                else
                {
                    myMenu.addAction("Make regular", this, &TilingMaker::slot_convertTile);
                }
                myMenu.exec(view->mapToGlobal(spt.toPoint()));
            }
                break;

            case ARC_POINT:
            case SCREEN_POINT:
                break;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        resetCurrentPlacedTile();
    }
}

void TilingMaker::setMousePos(QPointF spt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km == Qt::SHIFT)
    {
        sMousePos.setX(spt.x());
    }
    else if (km == Qt::CTRL)
    {
        sMousePos.setY(spt.y());
    }
    else
    {
        sMousePos = spt;
    }
}

void  TilingMaker::drawMouseInteraction(GeoGraphics * g2d)
{
    if (mouse_interaction)
    {
        mouse_interaction->draw(g2d);
    }
}


void TilingMaker::setTileEditPoint(QPointF pt)
{
    tileEditPoint = pt;
    qDebug() << "tile edit point =" << pt;
    forceRedraw();
}

bool TilingMaker::accumHasPoint(QPointF wpt)
{
    QPointF newpoint = worldToScreen(wpt);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        QPointF existing = worldToScreen(edge->v1->pt);
        if (Point::isNear(newpoint,existing))
        {
            return true;
        }
    }
    return false;
}

QString TilingMaker::getStatus()
{
    QString s = sTilingMakerMouseMode[tilingMakerMouseMode];
    if (mouse_interaction)
    {
        s += " ";
        s += mouse_interaction->desc;
    }
    s+= QString("  in_tiling: %1  all: %2").arg(in_tiling.count()).arg(allPlacedTiles.count());
    return s;
}


void TilingMaker::slot_flatenCurve()
{
    EdgePtr ep = tileSelector->getModelEdge();
    ep->resetCurve();
    forceRedraw();
    qDebug() << "edge converted to LINE";
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tileSelector->getPlacedTile()->getTile());
}

void TilingMaker::slot_makeConvex()
{
    EdgePtr ep = tileSelector->getModelEdge();
    ep->setConvex(true);
    forceRedraw();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tileSelector->getPlacedTile()->getTile());
}

void TilingMaker::slot_makeConcave()
{
    EdgePtr ep = tileSelector->getModelEdge();
    ep->setConvex(false);
    forceRedraw();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tileSelector->getPlacedTile()->getTile());
}

void TilingMaker::slot_moveArcCenter()
{
    mouse_interaction = make_shared<EditEdge>(this,tileSelector,QPointF());
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tileSelector->getPlacedTile()->getTile());
}

void TilingMaker::slot_editMagnitude()
{
    mouse_interaction.reset();
    DlgMagnitude dlg(tileSelector);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();
    pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tileSelector->getPlacedTile()->getTile());
}

//////////////////////////////////////////////////////////////////
///
/// Layer slots
///
//////////////////////////////////////////////////////////////////

void TilingMaker::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {

    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {

    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {

    }
}

void TilingMaker::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_TILING_MAKER)
        return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "slot_mousePressed:" << sMousePos << screenToWorld(sMousePos);
    
    TileSelectorPtr sel;

    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
        if (view->getMouseMode(MOUSE_MODE_NONE))
        {
            startMouseInteraction(sMousePos,btn);
        }
        break;

    case TM_COPY_MODE:
        sel = findTileUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(this, sel, sMousePos);
        break;

    case TM_DELETE_MODE:
        deleteTile( findTileUnderMouse() );
        setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
        break;

    case TM_TRANSLATION_VECTOR_MODE:
        sel = findSelection(sMousePos);
        if (sel)
            mouse_interaction = make_shared<DrawTranslation>(this, sel, sMousePos, layerPen);
        break;

    case TM_DRAW_POLY_MODE:
        mouse_interaction = make_shared<CreatePolygon>(this, sMousePos);
        break;

    case TM_INCLUSION_MODE:
        toggleInclusion(findTileUnderMouse());
        break;

    case TM_MEASURE_MODE:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            TileSelectorPtr nothingToIgnore;
            sel = findEdge(spt,nothingToIgnore);
            if (sel)
            {
                // FIXME  - this looks like incomplete code
                QLineF line = sel->getPlacedLine();
                qreal dist = Point::distToLine(screenToWorld(spt),line);
                qDebug() << "dist" << dist << "line" << line;
                mouse_interaction = make_shared<Measure>(this, sMousePos,sel);
            }
        }
        else
        {
            mouse_interaction = make_shared<Measure>(this, sMousePos,sel);
        }
        break;
    }

    case TM_POSITION_MODE:
        setCenterScreenUnits(spt);
        break;

    case TM_EDIT_TILE_MODE:
        sel = findSelection(spt);
        if (sel)
        {
            if (!editPlacedTile && sel->getType() == INTERIOR)
            {
                editPlacedTile = sel->getPlacedTile();
            }
            else if (editPlacedTile && sel->getType() == VERTEX )
            {
                mouse_interaction = make_shared<EditTile>(this, sel, editPlacedTile, sMousePos);
            }
        }
        break;
    case TM_EDGE_CURVE_MODE:
        sel = findArcPoint(spt);
        if (sel)
        {
            tileSelector = sel;
            QMenu myMenu;
            myMenu.addAction("Use Cursor to change curve", this, SLOT(slot_moveArcCenter()));
            myMenu.addAction("Edit Magnitude", this, SLOT(slot_editMagnitude()));
            myMenu.exec(view->mapToGlobal(spt.toPoint()));
        }
        else
        {
            sel = findEdge(spt);
            if (sel)
            {
                EdgePtr edge =  sel->getModelEdge();
                if (edge->getType() == EDGETYPE_LINE)
                {
                    PlacedTilePtr ptp = sel->getPlacedTile();
                    TilePtr tile = ptp->getTile();
                    if (tile->isRegular())
                    {
                        flipTileRegularity(tile);
                    }
                    edge->calcArcCenter(true,false);  // default to convex
                    qDebug() << "edge converted to curve";
                }
                else if (edge->getType() == EDGETYPE_CURVE)
                {
                    tileSelector = sel;
                    QMenu myMenu;
                    myMenu.addAction("Flatten Edge",  this, SLOT(slot_flatenCurve()));
                    if (edge->isConvex())
                        myMenu.addAction("Make Concave",  this, SLOT(slot_makeConcave()));
                    else
                        myMenu.addAction("Make Convex",  this, SLOT(slot_makeConvex()));
                    myMenu.exec(view->mapToGlobal(spt.toPoint()));
                }
                // Methinks tilings cannot have chords only curves(arcs)
            }
        }
        break;

    case TM_MIRROR_X_MODE:
        mirrorPolygonX(findTileUnderMouse());
        break;

    case TM_MIRROR_Y_MODE:
        mirrorPolygonY(findTileUnderMouse());
        break;

    case TM_CONSTRUCTION_LINES:
        mouse_interaction = make_shared<TilingConstructionLine>(this, sel, sMousePos);
        break;

    case TM_REFLECT_EDGE:
        auto fsel = findTileUnderMouse();
        if (fsel)
        {
            if (reflectPolygon(findTileUnderMouse()))
            {
                tilingMakerMouseMode = TM_NO_MOUSE_MODE;
            }
        }
        break;
    }

    if (debugMouse)
    {
        if (mouse_interaction)
            qDebug().noquote() << "press end:"  << mouse_interaction->desc;
        else
            qDebug() << "press end: no mouse_interaction";
    }
}

void TilingMaker::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_TILING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToWorld(sMousePos)  << sTilingMakerMouseMode[tilingMakerMouseMode];

    updateUnderMouse(sMousePos);

    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
    case TM_COPY_MODE:
    case TM_TRANSLATION_VECTOR_MODE:
    case TM_DRAW_POLY_MODE:
    case TM_MEASURE_MODE:
    case TM_POSITION_MODE:
    case TM_EDIT_TILE_MODE:
    case TM_EDGE_CURVE_MODE:
    case TM_CONSTRUCTION_LINES:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case TM_DELETE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_INCLUSION_MODE:
    case TM_REFLECT_EDGE:
        break;
    }
}

void TilingMaker::slot_mouseTranslate(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
        for (auto placedTile : qAsConst(allPlacedTiles))
        {
            QTransform t = placedTile->getTransform();
            t *= tt;
            placedTile->setTransform(t);
        }

        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        if (!currentPlacedTile)
            return;

        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        QTransform t = currentPlacedTile->getTransform();
        t *= tt;
        currentPlacedTile->setTransform(t);

        forceRedraw();
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();

        xf.setTranslateX(xf.getTranslateX() + spt.x());
        xf.setTranslateY(xf.getTranslateY() + spt.y());
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateX(xf.getTranslateX() + spt.x());
            xf.setTranslateY(xf.getTranslateY() + spt.y());
            setCanvasXform(xf);
        }
    }
}


void TilingMaker::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_TILING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}


void TilingMaker::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (config->getViewerType() != VIEW_TILING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << sMousePos << screenToWorld(sMousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(spt);
        mouse_interaction.reset();
    }
    forceRedraw();
}

void TilingMaker::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void TilingMaker::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);
        for (auto pplacedTile : qAsConst(allPlacedTiles))
        {
            QTransform t = pplacedTile->getTransform();
            t *= ts;
            pplacedTile->setTransform(t);
        }

        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaScale(1.0 + delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        uniqueTileDeltaScale(delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void TilingMaker::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setModelCenter(getCenterModelUnits());
        QTransform tr2 = xf.toQTransform(QTransform());

        for (auto pfp : qAsConst(allPlacedTiles))
        {
            QTransform t = pfp->getTransform();
            t *= tr2;
            pfp->setTransform(t);
        }

        pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaRotate(0.5 * delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        uniqueTileDeltaRotate(0.5 * delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setRotateDegrees(xf.getRotateDegrees() + delta);
            setCanvasXform(xf);
        }
    }
}

void TilingMaker::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_scale" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaScale(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        uniqueTileDeltaScale(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingDeltaScale(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void TilingMaker::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_rotate" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaRotate(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        uniqueTileDeltaRotate(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingDeltaRotate(amount);
        emit sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
            setCanvasXform(xf);
        }
    }
}


void TilingMaker::slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_moveX" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaX(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingDeltaX(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateX(xf.getTranslateX() + amount);
            setCanvasXform(xf);
        }
    }
}

void TilingMaker::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_moveY" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        placedTileDeltaY(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingDeltaY(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateY(xf.getTranslateY() + amount);
            setCanvasXform(xf);
        }
    }
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool TilingMaker::procKeyEvent(QKeyEvent * k)
{
    if (config->getViewerType() != VIEW_TILING_MAKER)
    {
        return false;
    }

    switch (k->key())
    {
        // actions
        case 'A': addRegularPolygon(); break;
        case 'C': addTileSelectionPointer(findTileUnderMouse()); break;
        case 'D': deleteTile(findTileUnderMouse()); break;
        case 'E': excludeAll(); break;
        case 'F': fillUsingTranslations(); break;
        case 'I': toggleInclusion(findTileUnderMouse()); break;
        case 'M': emit view->sig_raiseMenu(); break;
        case 'R': removeExcluded(); break;
        case 'Q': QApplication::quit(); break;
        case 'X': clearTranslationVectors(); break;
        case '0': updatePolygonSides(0); break;
        case '1': updatePolygonSides(1); break;
        case '2': updatePolygonSides(2); break;
        case '3': updatePolygonSides(3); break;
        case '4': updatePolygonSides(4); break;
        case '5': updatePolygonSides(5); break;
        case '6': updatePolygonSides(6); break;
        case '7': updatePolygonSides(7); break;
        case '8': updatePolygonSides(8); break;
        case '9': updatePolygonSides(9); break;
        case Qt::Key_F1:
            {
                QMessageBox  * box = new QMessageBox();
                box->setWindowTitle("Tiling Designer Shortcuts");
                box->setText(Shortcuts::getTilingMakerShortcuts());
                box->setModal(false);
                box->show();
                break;
            }

        // modes
        case Qt::Key_Escape: setTilingMakerMouseMode(TM_NO_MOUSE_MODE); return false;     // propagate
        case Qt::Key_F3: setTilingMakerMouseMode(TM_TRANSLATION_VECTOR_MODE); break;
        case Qt::Key_F4: setTilingMakerMouseMode(TM_DRAW_POLY_MODE); break;
        case Qt::Key_F5: setTilingMakerMouseMode(TM_COPY_MODE); break;
        case Qt::Key_F6: setTilingMakerMouseMode(TM_DELETE_MODE); break;
        case Qt::Key_F7: setTilingMakerMouseMode(TM_INCLUSION_MODE); break;
        case Qt::Key_F9: setTilingMakerMouseMode(TM_MEASURE_MODE); break;
        case Qt::Key_F11: setTilingMakerMouseMode(TM_EDIT_TILE_MODE); break;
        case Qt::Key_F12: setTilingMakerMouseMode(TM_EDGE_CURVE_MODE); break;

        default: return false;
   }

   return true;
}
