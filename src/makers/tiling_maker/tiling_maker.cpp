/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly featureful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/feature_selection.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "base/configuration.h"
#include "base/shortcuts.h"
#include "viewers/view.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "style/style.h"
#include "panels/dlg_magnitude.h"
#include "panels/panel.h"
#include "panels/dlg_edgepoly_edit.h"

TilingMaker *  TilingMaker::mpThis = nullptr;
TilingMakerPtr TilingMaker::spThis;

const bool debugMouse = false;

static QString strMouseMode[] =
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
    E2STR(CURVE_EDGE_MODE),
    E2STR(FLATTEN_EDGE_MODE),
    E2STR(TM_MIRROR_X_MODE),
    E2STR(TM_MIRROR_Y_MODE),
    E2STR(TM_CONSTRUCTION_LINES)
};


TilingMaker * TilingMaker::getInstance()
{
    if (!mpThis)
    {
        spThis = make_shared<TilingMaker>();
        mpThis = spThis.get();
    }
    return mpThis;
}


TilingMakerPtr TilingMaker::getSharedInstance()
{
    if (!mpThis)
    {
        spThis = make_shared<TilingMaker>();
        mpThis = spThis.get();
    }
    return spThis;
}


TilingMaker::TilingMaker() : TilingMakerView(this)
{
    qDebug() << "TilingMaker::TilingMaker";
}

void TilingMaker::init()
{
    view            = View::getInstance();
    motifMaker      = MotifMaker::getInstance();
    decorationMaker = DecorationMaker:: getInstance();

    connect(view, &View::sig_mouseDragged,  this, &TilingMaker::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &TilingMaker::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &TilingMaker::slot_mouseMoved);

    tilingMakerMouseMode = TM_NO_MOUSE_MODE;
    poly_side_count = config->polySides;
    poly_rotation   = 0.0;
    filled          = false;

    unload();
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
            sm_take(tiling,SM_LOAD_EMPTY);
        }
        else
        {
            select(tilings.first());
        }
    }
}

void TilingMaker::unload()
{
    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    sm_take(tiling,SM_LOAD_EMPTY);
}

void TilingMaker::eraseTilings()
{
    tilings.clear();
    selectedTiling.reset();
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
    tilings.removeOne(selectedTiling);
    tilings.push_front(tiling);
    select(tiling);
}

void TilingMaker::sm_add(TilingPtr tiling)
{
    tilings.push_front(tiling);
    select(tiling);
}


void TilingMaker::sm_title(TilingPtr tiling)
{
    view->setWindowTitle(QString("Loading tiling: %1 : %2").arg(tiling->getName(),tiling->getDescription()));
}

void TilingMaker::sm_take(TilingPtr tiling, eSM_Event event)
{
    eTMState state = sm_getState();
    qDebug().noquote() << "TilingMaker::takeUp() state:" << tm_states[state]  << "event:" <<  sSM_Events[event];

    switch(event)
    {
    case SM_LOAD_EMPTY:
        filled = false;
        clearConstructionLines();
        sm_resetAllAndAdd(tiling);
        motifMaker->sm_take(tiling, event);
        break;

    case SM_LOAD_SINGLE:
        filled = false;
        clearConstructionLines();
        sm_title(tiling);
        sm_resetAllAndAdd(tiling);
        motifMaker->sm_take(tiling, event);
        break;

    case SM_RELOAD_SINGLE:
        filled = false;
        clearConstructionLines();
        if (state == TM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else
        {
            sm_title(tiling);
            sm_resetCurrentAndAdd(tiling);
            motifMaker->sm_take(tiling, event);
        }
        break;

    case SM_LOAD_MULTI:
        filled = false;
        clearConstructionLines();
        sm_title(tiling);
        if (state == TM_EMPTY)
        {
            sm_resetAllAndAdd(tiling);
            motifMaker->sm_take(tiling, SM_LOAD_SINGLE);
        }
        else if (state == TM_SINGLE)
        {
            // the ask is for safety - for inadvertant use of multi
            bool add = sm_askAdd();
            if (add)
            {
                // what was asked for
                sm_add(tiling);
                motifMaker->sm_take(tiling,event);
            }
            else
            {
                // treat as single
                sm_resetAllAndAdd(tiling);
                motifMaker->sm_take(tiling, SM_LOAD_SINGLE);
            }
        }
        else if (state == TM_MULTI)
        {
            // always add, the user should know what they are doing by now
            sm_add(tiling);
            motifMaker->sm_take(tiling,event);
        }
        break;

    case SM_RELOAD_MULTI:
        clearConstructionLines();
        filled = false;
        sm_title(tiling);
        if (state == TM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == TM_SINGLE)
        {
            bool add = sm_askAdd();
            if (add)
            {
                sm_add(tiling);
                motifMaker->sm_take(tiling,event);
            }
            else
            {
                sm_resetAllAndAdd(tiling);
                motifMaker->sm_take(tiling, SM_RELOAD_SINGLE);
            }
        }
        else if (state == TM_MULTI)
        {
            bool add = sm_askAdd();
            if (add)
            {
                sm_add(tiling);
                motifMaker->sm_take(tiling,event);
            }
            else
            {
                sm_resetCurrentAndAdd(tiling);
                motifMaker->sm_take(tiling, SM_RELOAD_SINGLE);
            }
        }
        break;

    case SM_FEATURE_CHANGED:
        motifMaker->sm_take(tiling, event);
        break;

    case SM_TILING_CHANGED:
        motifMaker->sm_take(tiling, event);
        break;

    case SM_FIGURE_CHANGED:
        qWarning() << "Unexpected event: SM_FIGURE_CHANGED";
        break;

    case SM_LOAD_FROM_MOSAIC:
        filled = false;
        tilings.push_front(tiling);
        select(tiling);
        break;

    case SM_RENDER:
        qWarning() << "Unexpected event: SM_RENDER";
        break;
    }
}

void TilingMaker::select(TilingPtr tiling)
{
    if (tiling == selectedTiling)
        return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling:" << tiling->getName();

    clearMakerData();

    setupMaker(tiling);

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        forceLayerRecalc();
    }

    selectedTiling = tiling;

    PrototypePtr proto = motifMaker->findPrototypeByName(tiling);
    if (proto)
    {
        motifMaker->setSelectedPrototype(proto);
    }
}

void TilingMaker::select(PrototypePtr prototype)
{
    TilingPtr tiling = prototype->getTiling();

    if (tiling == selectedTiling)
        return;

    if (!isValidTiling(tiling))
        return;

    qDebug() << "Tiling Maker selected tiling from prototype:" << tiling->getName();

    clearMakerData();

    setupMaker(tiling);

    if (config->viewerType == VIEW_TILING_MAKER)
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
        Q_ASSERT(selectedTiling);
        Q_ASSERT(tilings.size() == 1);
        if (selectedTiling->isEmpty())
        {
            return TM_EMPTY;
        }
        else
        {
            return TM_SINGLE;
        }
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
    allPlacedFeatures.clear();
    in_tiling.clear();
    wAccum.clear();
    wMeasurements.clear();
    overlapping.clear();
    touching.clear();

    selectedTiling.reset();
    featureSelector.reset();
    currentPlacedFeature.reset();
    editPlacedFeature.reset();
    mouse_interaction.reset();
    clickedSelector.reset();

    tilingMakerMouseMode    = TM_NO_MOUSE_MODE;
}

void TilingMaker::setupMaker(TilingPtr tiling)
{
    const QVector<PlacedFeaturePtr> & qlpf = tiling->getPlacedFeatures();
    for(auto it = qlpf.begin(); it != qlpf.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        allPlacedFeatures.push_back(pf);
        in_tiling.push_back(pf);
    }

    QPointF trans_origin;
    if (allPlacedFeatures.size() > 0)
    {
        PlacedFeaturePtr pf = allPlacedFeatures.first();  // at this time allPlacedFeature and in_tiling are the same
        QTransform T        = pf->getTransform();
        trans_origin        = T.map(pf->getFeature()->getCenter());
    }

    visibleT1.setP1(trans_origin);
    visibleT1.setP2(trans_origin + tiling->getTrans1());

    visibleT2.setP1(trans_origin);
    visibleT2.setP2(trans_origin + tiling->getTrans2());

    // set the layer transform and the view size
    view->setDefinedFrameSize(VIEW_TILING_MAKER,tiling->getSize());

    Layer::setCanvasXform(tiling->getCanvasXform());
}

void TilingMaker::updateTilingPlacedFeatures()
{
    if (in_tiling != selectedTiling->getPlacedFeatures())
    {
        selectedTiling->setPlacedFeatures(in_tiling);
    }
}

bool TilingMaker::verifyTiling()
{
    if (allPlacedFeatures.size() <= 0)
    {
        qWarning("There are no polgons");
        return false;
    }

    if (in_tiling.size() <= 0)
    {
        qWarning("No selected tiles");
        return false;
    }

    if (isTranslationInvalid())
    {
        qWarning("Translation vectors not defined");
        return false;
    }
    return true;
}

void TilingMaker::setTilingMakerMouseMode(eTMMouseMode mode)
{
    tilingMakerMouseMode = mode;

    if (mode == TM_EDIT_FEATURE_MODE)
    {
        editPlacedFeature = currentPlacedFeature;
    }
    else
    {
        editPlacedFeature.reset();
    }

    wAccum.clear();

    forceRedraw();
}

eTMMouseMode TilingMaker::getTilingMakerMouseMode()
{
    return tilingMakerMouseMode;
}

// Feature management.

void TilingMaker::addNewPlacedFeature(PlacedFeaturePtr pf)
{
    allPlacedFeatures.push_front(pf);   // push_front so it (the new) becomes selected for move
    emit sig_buildMenu();
}

TilingSelectorPtr TilingMaker::addFeatureSelectionPointer(TilingSelectorPtr sel)
{
    PlacedFeaturePtr pf    = sel->getPlacedFeature();
    PlacedFeaturePtr pfnew = make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform());
    addNewPlacedFeature(pfnew);

    TilingSelectorPtr ret;
    switch (sel->getType())
    {
    case ARC_POINT:
    case FEAT_CENTER:
    case SCREEN_POINT:
        break;
    case INTERIOR:
        ret = make_shared<InteriorTilingSelector>(pfnew);
        break;
    case EDGE:
        ret = make_shared<EdgeTilingSelector>(pfnew,sel->getModelEdge());  // FIXME - why is this model edge
        break;
    case VERTEX:
        ret = make_shared<VertexTilingSelector>(pfnew,sel->getModelPoint());
        break;
    case MID_POINT:
        ret = make_shared<MidPointTilingSelector>(pfnew,sel->getModelEdge(),sel->getModelPoint());
        break;
    }

    emit sig_buildMenu();

    return ret;
}

void TilingMaker::slot_deleteFeature()
{
    if (clickedSelector)
    {
        deleteFeature(clickedSelector);
        clickedSelector.reset();
    }
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::deleteFeature(TilingSelectorPtr sel)
{
    if (!sel) return;

    PlacedFeaturePtr feature = sel->getPlacedFeature();
    if (featureSelector &&  featureSelector->getPlacedFeature() == feature)
    {
        featureSelector.reset();
    }

    if (feature)
    {
        deleteFeature(feature);

        emit sig_buildMenu();
        forceRedraw();
    }
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::deleteFeature(PlacedFeaturePtr pf)
{
    if (pf == currentPlacedFeature)
    {
        currentPlacedFeature.reset();
    }
    allPlacedFeatures.removeOne(pf);
    in_tiling.removeOne(pf);
    updateTilingPlacedFeatures();
    forceRedraw();
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::addInTiling(PlacedFeaturePtr pf)
{
    in_tiling.push_back(pf);
    updateTilingPlacedFeatures();
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::removeFromInTiling(PlacedFeaturePtr pf)
{
    in_tiling.removeOne(pf);
    updateTilingPlacedFeatures();
    sm_take(selectedTiling,SM_TILING_CHANGED);
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

void TilingMaker::removeExcluded()
{
    QVector<PlacedFeaturePtr> toRemove;

    for (auto& pf : allPlacedFeatures)
    {
        if (!in_tiling.contains(pf))
        {
            toRemove.push_back(pf);
        }
    }

    for (auto& pf : toRemove)
    {
        allPlacedFeatures.removeAll(pf);
    }

    featureSelector.reset();
    currentPlacedFeature.reset();

    forceRedraw();
    emit sig_buildMenu();
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::excludeAll()
{
    in_tiling.clear();
    forceRedraw();
    emit sig_buildMenu();
    sm_take(selectedTiling,SM_TILING_CHANGED);
}

void TilingMaker::slot_includeFeature()
{
    if (clickedSelector)
    {
        PlacedFeaturePtr pf = clickedSelector->getPlacedFeature();
        if(!in_tiling.contains(pf))
        {
            addInTiling( pf );
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            sm_take(selectedTiling,SM_TILING_CHANGED);
        }
    }
}

void TilingMaker::slot_excludeFeature()
{
    if (clickedSelector)
    {
        PlacedFeaturePtr pf = clickedSelector->getPlacedFeature();
        if (in_tiling.contains(pf))
        {
            removeFromInTiling(pf);
            emit sig_buildMenu();
            forceRedraw();
            clickedSelector.reset();
            sm_take(selectedTiling,SM_TILING_CHANGED);
        }
    }
}

void TilingMaker::slot_editFeature()
{
    if (clickedSelector)
    {
        PlacedFeaturePtr pfp = clickedSelector->getPlacedFeature();
        FeaturePtr        fp = pfp->getFeature();
        QTransform         t = pfp->getTransform();

        DlgEdgePolyEdit * fe  = new DlgEdgePolyEdit(fp->getEdgePoly(),t);
        fe->show();
        fe->raise();
        fe->activateWindow();

        connect(fe,   &DlgEdgePolyEdit::sig_currentPoint, this, &TilingMaker::setFeatureEditPoint, Qt::UniqueConnection);
        connect(this, &TilingMaker::sig_refreshMenu,      fe,   &DlgEdgePolyEdit::display,         Qt::UniqueConnection);
    }
}

void TilingMaker::slot_copyMoveFeature()
{
    if (clickedSelector)
    {
        mouse_interaction = make_shared<CopyMovePolygon>(this, clickedSelector, clickedSpt);
        clickedSelector.reset();
        clickedSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyFeature()
{
    if (clickedSelector)
    {
        PlacedFeaturePtr pf = clickedSelector->getPlacedFeature();
        FeaturePtr fp = pf->getFeature();
        FeaturePtr fp2 = fp->recreate();  // creates a new feature same as other
        pf->setFeature(fp2);
        emit sig_buildMenu();
        sm_take(selectedTiling,SM_FEATURE_CHANGED);
    }
}

void TilingMaker::slot_snapTo(bool checked)
{
    _snapToGrid = checked;
}

void TilingMaker::createFillCopies()
{
    // Create copies of the feature to help visualise the result in the panel.

    if ( isTranslationInvalid() )
        return;

    QPointF t1 = getSelected()->getTrans1();
    QPointF t2 = getSelected()->getTrans2();

    for (auto pf : qAsConst(in_tiling))
    {
        if (!pf->show())
        {
            continue;
        }
        FeaturePtr f = pf->getFeature();
        QTransform T = pf->getTransform();

        for( int y = -1; y <= 1; ++y )
        {
            for( int x = -1; x <= 1; ++x )
            {
                if ( y == 0 && x == 0 )
                    continue;
                QPointF pt = (t1*x) + (t2 * y);
                QTransform tt = QTransform::fromTranslate(pt.x(),pt.y());
                QTransform placement= T * tt;
                allPlacedFeatures.push_back(make_shared<PlacedFeature>(f, placement));
            }
        }
    }
    forceRedraw();
}

void TilingMaker::slot_showOverlaps(bool checked)
{
    config->tm_showOverlaps = checked;
    forceRedraw();
}

void TilingMaker::tilingDeltaX(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedFeatures))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setTransform(t);
    }

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaY(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedFeatures))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setTransform(t);
    }

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaScale(int delta)
{
    Q_ASSERT(config->kbdMode == KBD_MODE_XFORM_TILING);
    qreal scale = 1.0 + (0.01 * delta);
    for (auto pfp : qAsConst(allPlacedFeatures))
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

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : qAsConst(allPlacedFeatures))
    {
        QTransform t = pfp->getTransform();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setTransform(t);
    }

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedFeatureDeltaX(int delta)
{
    if (!currentPlacedFeature)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedFeature->getTransform();
    t *= QTransform::fromTranslate(qdelta,0.0);
    currentPlacedFeature->setTransform(t);

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedFeatureDeltaY(int delta)
{
    if (!currentPlacedFeature)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentPlacedFeature->getTransform();
    t *= QTransform::fromTranslate(0.0,qdelta);
    currentPlacedFeature->setTransform(t);

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedFeatureDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);
    placedFeatureDeltaScale(scale);
}

void TilingMaker::placedFeatureDeltaScale(qreal scale)
{
    if (!currentPlacedFeature)
        return;

    QPolygonF pts = currentPlacedFeature->getPlacedPolygon();
    QPointF center = Point::center(pts);

    QTransform ts;
    ts.scale(scale,scale);
    Xform xf(ts);
    xf.setCenter(center);
    ts = xf.toQTransform(QTransform());

    QTransform t = currentPlacedFeature->getTransform();
    t *= ts;
    currentPlacedFeature->setTransform(t);

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::placedFeatureDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    placedFeatureDeltaRotate(qdelta);
}

void TilingMaker::placedFeatureDeltaRotate(qreal rotate)
{
    if (!currentPlacedFeature)
        return;

    QPolygonF pts = currentPlacedFeature->getPlacedPolygon();
    QPointF center = Point::center(pts);

    Xform xf(QTransform().rotate(rotate));
    xf.setCenter(center);
    QTransform tr = xf.toQTransform(QTransform());

    QTransform t = currentPlacedFeature->getTransform();
    t *= tr;
    currentPlacedFeature->setTransform(t);

    sm_take(selectedTiling,SM_FEATURE_CHANGED);
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::uniqueFeatureDeltaScale(int delta)
{
    qreal scale = 0.01 * delta;
    uniqueFeatureDeltaScale(scale);
}

void TilingMaker::uniqueFeatureDeltaScale(qreal scale)
{
    FeaturePtr feature = currentPlacedFeature->getFeature();
    feature->deltaScale(scale);
}

void TilingMaker::uniqueFeatureDeltaRotate(int delta)
{
    qreal qdelta = 0.5 * delta;
    uniqueFeatureDeltaRotate(qdelta);
}

void TilingMaker::uniqueFeatureDeltaRotate(qreal rotate)
{
    FeaturePtr feature = currentPlacedFeature->getFeature();
    feature->deltaRotation(rotate);
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
        getSelected()->setTrans1(tran);
    }
    else
    {
        setT1      = true;
        visibleT2  = mLine;
        getSelected()->setTrans2(tran);
    }
    emit sig_refreshMenu();
}

void TilingMaker::updateVisibleVectors()
{
    TilingPtr tp = getSelected();
    visibleT1 = QLineF(visibleT1.p1(), visibleT1.p1() + tp->getTrans1());
    visibleT2 = QLineF(visibleT2.p1(), visibleT2.p1() + tp->getTrans2());
    forceRedraw();
}

bool TilingMaker::isTranslationInvalid()
{
    TilingPtr tp = getSelected();
    if (tp->getTrans1().isNull() || tp->getTrans2().isNull())
    {
        return true;
    }
    return false;
}

void TilingMaker::toggleInclusion(TilingSelectorPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getPlacedFeature();
        if (!pf) return;
        if( in_tiling.contains(pf))
        {
            removeFromInTiling( pf );
        }
        else
        {
            addInTiling(pf);
        }
        sm_take(selectedTiling,SM_TILING_CHANGED);
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
    Q_ASSERT(poly_side_count > 2);

     FeaturePtr f = make_shared<Feature>(poly_side_count,poly_rotation);
     QTransform t;
     addNewPlacedFeature(make_shared<PlacedFeature>(f,t));
     forceRedraw();
     emit sig_buildMenu();
}

void TilingMaker::mirrorPolygonX(TilingSelectorPtr sel )
{
    if (sel)
    {
        PlacedFeaturePtr pfp = sel->getPlacedFeature();
        if (!pfp) return;
        EdgePoly & ep = pfp->getFeature()->getEdgePoly();
#if 1
        QTransform t = QTransform::fromScale(-1,1);
        ep.mapD(t);
#else
        QPolygonF pts = ep.getPoly();
        qreal x = Point::center(pts).x();
        for (auto edge : ep)
        {
            QPointF pos = edge->getV1()->getPosition();
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
        sm_take(selectedTiling,SM_FEATURE_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::mirrorPolygonY(TilingSelectorPtr sel )
{
    if (sel)
    {
        PlacedFeaturePtr pfp = sel->getPlacedFeature();
        EdgePoly & ep = pfp->getFeature()->getEdgePoly();
        QTransform t = QTransform::fromScale(1,-1);
        ep.mapD(t);
        sm_take(selectedTiling,SM_FEATURE_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::copyPolygon(TilingSelectorPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getPlacedFeature();
        addNewPlacedFeature(make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform()));
        forceRedraw();
        emit sig_buildMenu();
        sm_take(selectedTiling,SM_TILING_CHANGED);
    }
}

TilingSelectorPtr TilingMaker::findFeatureUnderMouse()
{
    return findFeature(sMousePos);
}

////////////////////////////////////////////////////////////////////////////
//
// Mouse events.
//
////////////////////////////////////////////////////////////////////////////

void  TilingMaker::drawMouseInteraction(GeoGraphics * g2d)
{
    if (mouse_interaction)
    {
        mouse_interaction->draw(g2d);
    }
}

void TilingMaker::slot_mouseMoved(QPointF spt)
{
    if (config->viewerType != VIEW_TILING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}

void TilingMaker::slot_mouseDragged(QPointF spt)
{
    if (config->viewerType != VIEW_TILING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToWorld(sMousePos)  << strMouseMode[tilingMakerMouseMode];

    updateUnderMouse(sMousePos);

    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
    case TM_COPY_MODE:
    case TM_TRANSLATION_VECTOR_MODE:
    case TM_DRAW_POLY_MODE:
    case TM_MEASURE_MODE:
    case TM_POSITION_MODE:
    case TM_BKGD_SKEW_MODE:
    case TM_EDIT_FEATURE_MODE:
    case TM_EDGE_CURVE_MODE:
    case TM_CONSTRUCTION_LINES:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case TM_DELETE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_INCLUSION_MODE:
        break;
    }
}

void TilingMaker::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (config->viewerType != VIEW_TILING_MAKER)
        return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "slot_mousePressed:" << sMousePos << screenToWorld(sMousePos);

    TilingSelectorPtr sel;

    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
        if (view->getMouseMode() == MOUSE_MODE_NONE)
        {
            startMouseInteraction(sMousePos,btn);
        }
        break;

    case TM_COPY_MODE:
        sel = findFeatureUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(this, sel, sMousePos);
        break;

    case TM_DELETE_MODE:
        deleteFeature( findFeatureUnderMouse() );
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
        toggleInclusion(findFeatureUnderMouse());
        break;

    case TM_MEASURE_MODE:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            TilingSelectorPtr nothingToIgnore;
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
        setCenterScreen(spt);
        break;

    case TM_BKGD_SKEW_MODE:
        if (mouse_interaction)
        {
            PerspectivePtr pp = std::dynamic_pointer_cast<Perspective>(mouse_interaction);
            if (pp)
            {
                pp->addPoint(sMousePos);
            }
        }
        else
        {
            mouse_interaction = make_shared<Perspective>(this, sMousePos);
        }
        break;

    case TM_EDIT_FEATURE_MODE:
        sel = findSelection(spt);
        if (sel)
        {
            if (!editPlacedFeature && sel->getType() == INTERIOR)
            {
                editPlacedFeature = sel->getPlacedFeature();
            }
            else if (editPlacedFeature && sel->getType() == VERTEX )
            {
                mouse_interaction = make_shared<EditFeature>(this, sel, editPlacedFeature, sMousePos);
            }
        }
        break;
    case TM_EDGE_CURVE_MODE:
        sel = findArcPoint(spt);
        if (sel)
        {
            featureSelector = sel;
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
                    PlacedFeaturePtr pfp = sel->getPlacedFeature();
                    FeaturePtr fp = pfp->getFeature();
                    fp->setRegular(false);
                    edge->calcArcCenter(true);  // default to convex
                    qDebug() << "edge converted to curve";
                }
                else if (edge->getType() == EDGETYPE_CURVE)
                {
                    featureSelector = sel;
                    QMenu myMenu;
                    myMenu.addAction("Flatten Edge",  this, SLOT(slot_flatenCurve()));
                    if (edge->isConvex())
                        myMenu.addAction("Make Concave",  this, SLOT(slot_makeConcave()));
                    else
                        myMenu.addAction("Make Convex",  this, SLOT(slot_makeConvex()));
                    myMenu.exec(view->mapToGlobal(spt.toPoint()));
                }
            }
        }
        break;

    case TM_MIRROR_X_MODE:
        mirrorPolygonX(findFeatureUnderMouse());
        break;

    case TM_MIRROR_Y_MODE:
        mirrorPolygonY(findFeatureUnderMouse());
        break;

    case TM_CONSTRUCTION_LINES:
        mouse_interaction = make_shared<TilingConstructionLine>(this, sel, sMousePos);
        break;
    }

    if (debugMouse)
    {
        if (mouse_interaction)
            qDebug().noquote() << "press end:"  << mouse_interaction->desc;
        else
            qDebug() << "press end: no mouse_interaction";
    }

    Layer::slot_mousePressed(spt,btn);  // handles centering
}

void TilingMaker::slot_mouseReleased(QPointF spt)
{

    if (config->viewerType != VIEW_TILING_MAKER)
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

void TilingMaker::updateUnderMouse(QPointF spt)
{
    switch (tilingMakerMouseMode)
    {
    case TM_NO_MOUSE_MODE:
    case TM_MEASURE_MODE:
    case TM_COPY_MODE:
    case TM_DELETE_MODE:
    case TM_INCLUSION_MODE:
    case TM_EDIT_FEATURE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
        featureSelector = findSelection(spt);
        if (featureSelector)
            forceRedraw();  // NOTE this triggers a lot of repainting
        break;

    case TM_TRANSLATION_VECTOR_MODE:
    case TM_CONSTRUCTION_LINES:
        featureSelector = findSelection(spt);
        if (featureSelector)
            forceRedraw();
        break;

    case TM_DRAW_POLY_MODE:
        featureSelector = findVertex(spt);
        if (!featureSelector)
        {
            featureSelector = findMidPoint(spt);
        }
        if (!featureSelector)
        {
            featureSelector = findNearGridPoint(spt);
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
        featureSelector = findArcPoint(spt);
        if (featureSelector)
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            featureSelector = findEdge(spt);
            if (featureSelector)
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                featureSelector.reset();
            }
        }
        forceRedraw();
        break;

    case TM_BKGD_SKEW_MODE:
        break;  // do nothing
    }
}

void TilingMaker::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    Q_ASSERT(tilingMakerMouseMode == TM_NO_MOUSE_MODE);

    TilingSelectorPtr sel = findSelection(spt);
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
                currentPlacedFeature = sel->getPlacedFeature();
                emit sig_current_feature(allPlacedFeatures.indexOf(currentPlacedFeature));
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
                PlacedFeaturePtr feature = sel->getPlacedFeature();
                QMenu myMenu;
                myMenu.addSection("Options");
                myMenu.addSeparator();
                myMenu.addAction("Copy/Move", this, &TilingMaker::slot_copyMoveFeature);
                myMenu.addAction("Edit Figure", this, &TilingMaker::slot_editFeature);
                if (isIncluded(feature))
                {
                    myMenu.addAction("Exclude", this, &TilingMaker::slot_excludeFeature);
                }
                else
                {
                    myMenu.addAction("Include", this, &TilingMaker::slot_includeFeature);
                }
                myMenu.addAction("Delete", this, &TilingMaker::slot_deleteFeature);
                myMenu.addAction("Uniquify", this, &TilingMaker::slot_uniquifyFeature);
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
        currentPlacedFeature.reset();
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

void TilingMaker::setFeatureEditPoint(QPointF pt)
{
    featureEditPoint = pt;
    qDebug() << "feature edit point =" << pt;
    forceRedraw();
}

bool TilingMaker::accumHasPoint(QPointF wpt)
{
    QPointF newpoint = worldToScreen(wpt);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        QPointF existing = worldToScreen(edge->getV1()->getPosition());
        if (Point::isNear(newpoint,existing))
        {
            return true;
        }
    }
    return false;
}

QString TilingMaker::getStatus()
{
    QString s = strMouseMode[tilingMakerMouseMode];
    if (mouse_interaction)
    {
        s += " ";
        s += mouse_interaction->desc;
    }
    return s;
}


void TilingMaker::slot_flatenCurve()
{
    EdgePtr ep = featureSelector->getModelEdge();
    ep->resetCurve();
    forceRedraw();
    qDebug() << "edge converted to LINE";
}

void TilingMaker::slot_makeConvex()
{
    EdgePtr ep = featureSelector->getModelEdge();
    ep->setConvex(true);
    forceRedraw();
}

void TilingMaker::slot_makeConcave()
{
    EdgePtr ep = featureSelector->getModelEdge();
    ep->setConvex(false);
    forceRedraw();
}

void TilingMaker::slot_moveArcCenter()
{
    mouse_interaction = make_shared<EditEdge>(this,featureSelector,QPointF());
}

void TilingMaker::slot_editMagnitude()
{
    mouse_interaction.reset();
    DlgMagnitude dlg(featureSelector);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();
}

void TilingMaker::slot_moveX(int amount)
{
    qDebug() << "TilingMaker::slot_moveX" << amount;
    switch (config->kbdMode)
    {
    case  KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaX(amount);
        break;

    case  KBD_MODE_XFORM_UNIQUE_FEATURE:
        // do nothing
        break;

    case  KBD_MODE_XFORM_TILING:
        tilingDeltaX(amount);
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_moveX(amount);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_moveY(int amount)
{
    qDebug() << "TilingMaker::slot_moveY" << amount;
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaY(amount);
        break;

    case KBD_MODE_XFORM_UNIQUE_FEATURE:
        // do nothing
        break;

    case KBD_MODE_XFORM_TILING:
        tilingDeltaY(amount);
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_moveY(amount);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_rotate(int amount)
{
    qDebug() << "TilingMaker::slot_rotate" << amount;
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaRotate(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_UNIQUE_FEATURE:
        uniqueFeatureDeltaRotate(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_TILING:
        tilingDeltaRotate(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_rotate(amount);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_scale(int amount)
{
    qDebug() << "TilingMaker::slot_scale" << amount;
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaScale(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_UNIQUE_FEATURE:
        uniqueFeatureDeltaScale(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_TILING:
        tilingDeltaScale(amount);
        emit sig_refreshMenu();
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_scale(amount);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_wheel_rotate(qreal delta)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
    {
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setCenter(getCenterModel());
        QTransform tr2 = xf.toQTransform(QTransform());
        qDebug() << Transform::toInfoString(tr);
        qDebug() << Transform::toInfoString(tr2);
        for (auto pfp : qAsConst(allPlacedFeatures))
        {
            QTransform t = pfp->getTransform();
            t *= tr2;
            pfp->setTransform(t);
        }

        sm_take(selectedTiling,SM_FEATURE_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
        break;

    case KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaRotate(0.5 * delta);
        break;

    case KBD_MODE_XFORM_UNIQUE_FEATURE:
        uniqueFeatureDeltaRotate(0.5 * delta);
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_wheel_rotate(delta);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_wheel_scale(qreal delta)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);
        for (auto pfp : qAsConst(allPlacedFeatures))
        {
            QTransform t = pfp->getTransform();
            t *= ts;
            pfp->setTransform(t);
        }

        sm_take(selectedTiling,SM_FEATURE_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
        break;

    case  KBD_MODE_XFORM_PLACED_FEATURE:
        placedFeatureDeltaScale(1.0 + delta);
        break;

    case  KBD_MODE_XFORM_UNIQUE_FEATURE:
        uniqueFeatureDeltaScale(delta);
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_wheel_scale(delta);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void TilingMaker::slot_mouseTranslate(QPointF spt)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
    {
        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
        for (auto pfp : qAsConst(allPlacedFeatures))
        {
            QTransform t = pfp->getTransform();
            t *= tt;
            pfp->setTransform(t);
        }

        sm_take(selectedTiling,SM_FEATURE_CHANGED);
        forceRedraw();
        emit sig_refreshMenu();
    }
        break;

    case KBD_MODE_XFORM_PLACED_FEATURE:
    {
        if (!currentPlacedFeature)
            return;

        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        QTransform t = currentPlacedFeature->getTransform();
        t *= tt;
        currentPlacedFeature->setTransform(t);

        forceRedraw();
        emit sig_refreshMenu();
    }
        break;

    case KBD_MODE_XFORM_UNIQUE_FEATURE:
        // do nothing
        break;

    case KBD_MODE_XFORM_VIEW:
        Layer::slot_mouseTranslate(spt);
        break;

    case KBD_MODE_XFORM_BKGD:
        break;

    default:
        qWarning() << "TilingMaker: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool TilingMaker::procKeyEvent(QKeyEvent * k)
{
    if (config->viewerType != VIEW_TILING_MAKER)
    {
        return false;
    }

    switch (k->key())
    {
        // actions
        case 'A': addRegularPolygon(); break;
        case 'C': addFeatureSelectionPointer(findFeatureUnderMouse()); break;
        case 'D': deleteFeature(findFeatureUnderMouse()); break;
        case 'E': excludeAll(); break;
        case 'F': fillUsingTranslations(); break;
        case 'I': toggleInclusion(findFeatureUnderMouse()); break;
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
        case Qt::Key_F10: setTilingMakerMouseMode(TM_BKGD_SKEW_MODE); break;
        case Qt::Key_F11: setTilingMakerMouseMode(TM_EDIT_FEATURE_MODE); break;
        case Qt::Key_F12: setTilingMakerMouseMode(TM_EDGE_CURVE_MODE); break;

        default: return false;
   }

   return true;
}
