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

#include "base/configuration.h"
#include "base/canvas.h"
#include "base/shortcuts.h"
#include "base/workspace.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_selection.h"
#include "geometry/point.h"
#include "viewers/workspace_viewer.h"
#include "style/style.h"
#include "panels/dlg_magnitude.h"

TilingMaker *  TilingMaker::mpThis = nullptr;
TilingMakerPtr TilingMaker::spThis;

const bool debugMouse = false;

static QString strMouseMode[] =
{
    Enum2Str(NO_MOUSE_MODE),
    Enum2Str(COPY_MODE),
    Enum2Str(DELETE_MODE),
    Enum2Str(TRANSLATION_VECTOR_MODE),
    Enum2Str(DRAW_POLY_MODE),
    Enum2Str(INCLUSION_MODE),
    Enum2Str(POSITION_MODE),
    Enum2Str(MEASURE_MODE),
    Enum2Str(BKGD_SKEW_MODE),
    Enum2Str(EDIT_FEATURE_MODE),
    Enum2Str(CURVE_EDGE_MODE),
    Enum2Str(FLATTEN_EDGE_MODE),
    Enum2Str(MIRROR_X_MODE),
    Enum2Str(MIRROR_Y_MODE)
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


TilingMaker::TilingMaker() : TilingMakerView()
{
    qDebug() << "TilingMaker::TilingMaker";

    canvas    = Canvas::getInstance();
    view      = View::getInstance();
    workspace = Workspace::getInstance();

    connect(view, &View::sig_mouseDragged,  this, &TilingMaker::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &TilingMaker::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &TilingMaker::slot_mouseMoved);

    mouse_mode      = NO_MOUSE_MODE;
    poly_side_count = config->polySides;
    poly_rotation   = 0.0;
}

void TilingMaker::slot_setTiling()
{
    TilingPtr tp = workspace->getTiling();

    clearMakerData();

    setupMaker();

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        forceUpdateLayer();
    }

    currentTiling = tp;
}

void TilingMaker::slot_unload()
{
    clearMakerData();
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

    currentTiling.reset();
    currentSelection.reset();
    currentFeature.reset();
    editFeature.reset();
    mouse_interaction.reset();
    menuSelection.reset();

    mouse_mode    = NO_MOUSE_MODE;

    wTrans1_start = QPointF();
    wTrans1_end   = QPointF();
    wTrans2_start = QPointF();
    wTrans2_end   = QPointF();
}

void TilingMaker::setupMaker()
{
    TilingPtr tiling = workspace->getTiling();
    QList<PlacedFeaturePtr> & qlpf = tiling->getPlacedFeatures();
    for(auto it = qlpf.begin(); it != qlpf.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        addToAllPlacedFeatures(pf);
        in_tiling.push_back(pf);
    }

    QPointF trans_origin;
    if (allPlacedFeatures.size() > 0)
    {
        PlacedFeaturePtr pf = allPlacedFeatures.first();  // at this time allPlacedFeature and in_tiling are the same
        QTransform T        = pf->getTransform();
        trans_origin        = T.map(pf->getFeature()->getCenter());
    }
    else
    {
        trans_origin = QPointF( 0, 0 );
    }

    wTrans1_start = trans_origin;
    wTrans1_end   = trans_origin + tiling->getTrans1();

    wTrans2_start = trans_origin;
    wTrans2_end   = trans_origin + tiling->getTrans2();

    // set the layer transform and the view size
    wsViewer->setViewSize(VIEW_TILING_MAKER,tiling->getCanvasSize());
    Layer::setCanvasXform(tiling->getCanvasXform());

    qtr_layer.reset();     // forced recompute of layer transform
}

void TilingMaker::pushTiling()
{
    TilingPtr tiling = workspace->getTiling();
    tiling->setDirty(true);

    MosaicPtr mosaic  = workspace->getMosaic();
    QVector<PrototypePtr> prototypes = mosaic->getUniquePrototypes();
    for (auto prototype : prototypes)
    {
        prototype->setTiling(tiling);
        prototype->createProtoMap();
    }

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->resetStyleRepresentation();
        style->createStyleRepresentation();
    }
}

void TilingMaker::hide(bool state)
{
    _hide = state;
    forceRedraw();
}

void TilingMaker::updatePlacedFeaturesFromData()
{
    if (in_tiling.size() <= 0)
    {
        return;
    }

    // need to remove what is there already
    TilingPtr tiling = workspace->getTiling();
    tiling->getPlacedFeatures().clear();

    // We assume the data has already been verified (verifyTiling() == true).
    for (auto placedFeature : in_tiling)
    {
        tiling->add(placedFeature);
    }
    tiling->setDirty(true);
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


/////////////////////////////////////////////////////////
///
/// Draw
///
/////////////////////////////////////////////////////////


void TilingMaker::draw( GeoGraphics * g2d )
{
    if (!_hide && !editFeature)
    {
        drawTiling(g2d);

        drawTranslationVectors(g2d,wTrans1_start,wTrans1_end,wTrans2_start,wTrans2_end);
    }

    if (currentSelection && !editFeature)
    {
        qDebug() << "current selection:"  << strTiliingSelection[currentSelection->getType()];
        switch (currentSelection->getType())
        {
        case NOTHING:
            if (mouse_mode == DRAW_POLY_MODE || mouse_mode == BKGD_SKEW_MODE)
            {
                g2d->drawLine(wAccum.last()->getV2()->getPosition(),screenToWorld(sMousePos), QPen(drag_color,1));
                g2d->drawCircle(screenToWorld(sMousePos),10,  QPen(drag_color,1), QBrush(drag_color));
            }
            break;

        case INTERIOR:
            // nothing - handled by currentFeature
            break;

        case EDGE:
            g2d->drawEdge(currentSelection->getPlacedEdge(), QPen(QColor(Qt::green),5));
            break;

        case VERTEX:
        case MID_POINT:
        case FEAT_CENTER:
            g2d->drawCircle(currentSelection->getPlacedPoint(), 10, QPen(under_mouse_color),QBrush(under_mouse_color));
            break;

        case ARC_POINT:
            g2d->drawCircle(currentSelection->getPlacedPoint(), 10, QPen(under_mouse_color), QBrush(under_mouse_color));
            g2d->drawEdge(currentSelection->getPlacedEdge(), QPen(QColor(Qt::red),5));
            break;

        case SCREEN_POINT:
            g2d->drawCircle(currentSelection->getModelPoint(), 14, QPen(Qt::red), QBrush(Qt::red));
            break;
        }
    }



    if (getMouseMode() == EDIT_FEATURE_MODE)
    {
        QPolygonF p;
        if (editFeature)
        {
             p = editFeature->getPlacedPolygon();
             drawFeature(g2d, editFeature, true, normal_color);
        }
        else if (currentSelection && currentSelection->getType() == INTERIOR)
        {
             p = currentSelection->getPlacedPolygon();
        }
        if (p.size())
        {
            QPen pen(Qt::blue);
            QBrush brush(Qt::blue);
            for (auto it = p.begin(); it != p.end(); it++)
            {
                g2d->drawCircle(*it,5,pen,brush);
            }
        }
    }

    drawMeasurements(g2d);

    drawAccum(g2d);

    if (mouse_interaction)
    {
        mouse_interaction->draw(g2d);
    }

    if (!featureEditPoint.isNull())
    {
        g2d->drawCircle(featureEditPoint,10,QPen(Qt::red),QBrush());
    }
}

void TilingMaker::drawTiling( GeoGraphics * g2d )
{
    determineOverlapsAndTouching();

    for(auto pf : allPlacedFeatures)
    {
        if (pf == currentFeature)
        {
            drawFeature(g2d, pf, true, under_mouse_color);
        }
        else if(overlapping.contains(pf))
        {
            drawFeature(g2d, pf, true, overlapping_color);
        }
        else if (touching.contains(pf))
        {
            drawFeature(g2d, pf, true, touching_color);
        }
        else if (in_tiling.contains(pf))
        {
            drawFeature(g2d, pf, true, in_tiling_color);
        }
        else
        {
            drawFeature(g2d, pf, true, normal_color);
        }
    }
}

void TilingMaker::setMouseMode(eMouseMode mode)
{
    mouse_mode = mode;

    editFeature.reset();
    wAccum.clear();

    forceRedraw();
}

eMouseMode TilingMaker::getMouseMode()
{
    return mouse_mode;
}

// Feature management.

int TilingMaker::addToAllPlacedFeatures(PlacedFeaturePtr pf)
{
    int add_pos = allPlacedFeatures.size();
    allPlacedFeatures.push_front(pf);   // push_front so it (the new) becomes selected for move

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        forceRedraw();
    }
    return add_pos;
}

TilingSelectionPtr TilingMaker::addFeatureSelectionPointer(TilingSelectionPtr sel)
{
    PlacedFeaturePtr pf    = sel->getPlacedFeature();
    PlacedFeaturePtr pfnew = make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform());
    addToAllPlacedFeatures(pfnew);

    TilingSelectionPtr ret;
    switch (sel->getType())
    {
    case NOTHING:
    case ARC_POINT:
    case FEAT_CENTER:
    case SCREEN_POINT:
        break;
    case INTERIOR:
        ret = make_shared<TilingSelection>(INTERIOR,pfnew);
        break;
    case EDGE:
        ret = make_shared<TilingSelection>(EDGE,pfnew,sel->getModelEdge());
        break;
    case VERTEX:
        ret = make_shared<TilingSelection>(VERTEX,pfnew,sel->getModelPoint());
        break;
    case MID_POINT:
        ret = make_shared<TilingSelection>(MID_POINT,pfnew,sel->getModelPoint());
        break;
    }

    emit sig_buildMenu();

    return ret;
}

void TilingMaker::removeFeature(PlacedFeaturePtr pf )
{
    allPlacedFeatures.removeOne(pf);
    in_tiling.removeOne(pf);
    forceRedraw();
}

void TilingMaker::removeFeature(TilingSelectionPtr sel)
{
    if ( currentSelection && sel && currentSelection->getPlacedFeature() == sel->getPlacedFeature())
    {
        currentSelection.reset();
        currentFeature.reset();
    }
    removeFeature(sel->getPlacedFeature());
}

void TilingMaker::addInTiling(PlacedFeaturePtr pf)
{
    in_tiling.push_back(pf);
}

void TilingMaker::removeFromInTiling(PlacedFeaturePtr pf)
{
    in_tiling.removeOne(pf);
}

void TilingMaker::fillUsingTranslations()
{
    if (!verifyTiling())
        return;

    removeExcluded();
    createFillCopies();

    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::removeExcluded()
{
    QVector<PlacedFeaturePtr> toRemove;

    for (auto pf : allPlacedFeatures)
    {
        if (!in_tiling.contains(pf))
        {
            toRemove.push_back(pf);
        }
    }

    for (auto pf : toRemove)
    {
        allPlacedFeatures.removeAll(pf);
    }

    currentSelection.reset();
    currentFeature.reset();

    forceRedraw();
    emit sig_buildMenu();
}

void TilingMaker::excludeAll()
{
    in_tiling.clear();
    forceRedraw();
    emit sig_buildMenu();
}


void TilingMaker::slot_deleteFeature()
{
    if (menuSelection)
    {
        deleteFeature(menuSelection);
        menuSelection.reset();
        //emit sig_buildMenu();
    }
}

void TilingMaker::slot_includeFeature()
{
    if (menuSelection)
    {
        PlacedFeaturePtr pf = menuSelection->getPlacedFeature();
        if(!in_tiling.contains(pf))
        {
            addInTiling( pf );
            emit sig_buildMenu();
            forceRedraw();
            menuSelection.reset();
        }
    }
}

void TilingMaker::slot_excludeFeature()
{
    if (menuSelection)
    {
        PlacedFeaturePtr pf = menuSelection->getPlacedFeature();
        if (in_tiling.contains(pf))
        {
            removeFromInTiling(pf);
            emit sig_buildMenu();
            forceRedraw();
            menuSelection.reset();
        }
    }
}

void TilingMaker::slot_copyMoveFeature()
{
    if (menuSelection)
    {
        mouse_interaction = make_shared<CopyMovePolygon>(this, menuSelection, menuSpt);
        menuSelection.reset();
        menuSpt = QPointF();
    }
}

void TilingMaker::slot_uniquifyFeature()
{
    if (menuSelection)
    {
        PlacedFeaturePtr pf = menuSelection->getPlacedFeature();
        FeaturePtr fp = pf->getFeature();
        FeaturePtr fp2 = fp->recreate();  // creates a new feature same as other
        pf->setFeature(fp2);
        emit sig_buildMenu();
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

    QPointF t1 = getTrans1();
    QPointF t2 = getTrans2();

    for (auto pf : in_tiling)
    {
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
                addToAllPlacedFeatures(make_shared<PlacedFeature>( f, placement ) );
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
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setTransform(t);
    }

    if (config->viewerType != VIEW_TILING_MAKER  &&  config->viewerType != VIEW_TILING)
    {

        pushTiling();
        forceRedraw();
    }
    else
    {
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::tilingDeltaY(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setTransform(t);
    }

    if (config->viewerType != VIEW_TILING_MAKER  &&  config->viewerType != VIEW_TILING)
    {

        pushTiling();
        forceRedraw();
    }
    else
    {
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::tilingDeltaScale(int delta)
{
    qreal scale = 1.0 + (0.01 * delta);
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        qDebug() << "t0" << Transform::toInfoString(t);
        QTransform t1 = t.scale(scale,scale);

        t = pfp->getTransform();
        QTransform t2 = t *QTransform::fromScale(scale,scale);

        qDebug() << "t1" << Transform::toInfoString(t1);
        qDebug() << "t2" << Transform::toInfoString(t2);

        t = pfp->getTransform();
        if (config->kbdMode == KBD_MODE_XFORM_FEATURE)
        {
            // does not change position
            t = t.scale(scale,scale);
            pfp->setTransform(t);
        }
        else if (config->kbdMode == KBD_MODE_XFORM_TILING)
        {
            // scales position too
            t *= QTransform::fromScale(scale,scale);
            pfp->setTransform(t);
        }
    }

    if (config->viewerType != VIEW_TILING_MAKER  &&  config->viewerType != VIEW_TILING)
    {

        pushTiling();
        forceRedraw();
    }
    else
    {
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::tilingDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setTransform(t);
    }

    if (config->viewerType != VIEW_TILING_MAKER  &&  config->viewerType != VIEW_TILING)
    {

        pushTiling();
        forceRedraw();
    }
    else
    {
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::featureDeltaX(int delta)
{
    if (!currentFeature)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentFeature->getTransform();
    t *= QTransform::fromTranslate(qdelta,0.0);
    currentFeature->setTransform(t);

    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::featureDeltaY(int delta)
{
    if (!currentFeature)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentFeature->getTransform();
    t *= QTransform::fromTranslate(0.0,qdelta);
    currentFeature->setTransform(t);

    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::featureDeltaScale(int delta)
{
    if (!currentFeature)
        return;

    qreal scale = 1.0 + (0.01 * delta);
    QTransform t = currentFeature->getTransform();
    qDebug() << "t0" << Transform::toInfoString(t);
    QTransform t1 = t.scale(scale,scale);

    t = currentFeature->getTransform();
    QTransform t2 = t *QTransform::fromScale(scale,scale);

    qDebug() << "t1" << Transform::toInfoString(t1);
    qDebug() << "t2" << Transform::toInfoString(t2);

    t = currentFeature->getTransform();
    if (config->kbdMode == KBD_MODE_XFORM_FEATURE)
    {
        // does not change position
        t = t.scale(scale,scale);
        currentFeature->setTransform(t);
    }
    else if (config->kbdMode == KBD_MODE_XFORM_TILING)
    {
        // scales position too
        t *= QTransform::fromScale(scale,scale);
        currentFeature->setTransform(t);
    }

    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::featureDeltaRotate(int delta)
{
    if (!currentFeature)
        return;

    qreal qdelta = 0.01 * delta;
    QTransform t = currentFeature->getTransform();
    t *= QTransform().rotateRadians(qdelta);
    currentFeature->setTransform(t);

    forceRedraw();
    emit sig_refreshMenu();
}

////////////////////////////////////////////////////////////////////////////
//
// Tiling translation vector.
//
// We treat the two vector as a circular buffer so that each one will
// get over-written in turn, so that both can be alternatively changed
// by the end-user.

void TilingMaker::addToTranslate(QPointF wpt, bool ending )
{
    if  (wTrans1_start.isNull())
    {
        wTrans1_start = wpt;
    }
    else if (wTrans1_end.isNull())
    {
        if (ending && (wpt != wTrans1_start))
        {
            wTrans1_end = wpt;
        }
    }
    else if (!ending )
    {
        wTrans2_start = wTrans1_start;
        wTrans2_end   = wTrans1_end;
        wTrans1_start = wpt;
        wTrans1_end   = QPointF();
    }

    emit sig_refreshMenu();
}

void TilingMaker::fixupTranslate(TilingPtr tiling)
{
    Q_ASSERT(tiling);
    wTrans1_end   = wTrans1_start + tiling->getTrans1();
    wTrans2_end   = wTrans2_start + tiling->getTrans2();
    forceRedraw();
}

bool TilingMaker::isTranslationInvalid()
{
    if (wTrans1_start.isNull()  || wTrans1_end.isNull() || wTrans2_start.isNull() || wTrans2_end.isNull())
        return true;

    if  ((wTrans1_start == wTrans1_end) || (wTrans2_start == wTrans2_end))
        return true;

    return false;
}


// Internal tiling creation.

QPointF TilingMaker::getTrans1()
{
    if (wTrans1_start.isNull() || wTrans1_end.isNull())
           return QPointF( );
    else
        return wTrans1_end - wTrans1_start ;
}

QPointF TilingMaker::getTrans2()
{
    if (wTrans2_start.isNull() || wTrans2_end.isNull())
           return QPointF( );
    else
        return wTrans2_end - wTrans2_start ;
}

void TilingMaker::toggleInclusion(TilingSelectionPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getPlacedFeature();
        if( in_tiling.contains(pf))
        {
            removeFromInTiling( pf );
        }
        else
        {
            addInTiling( pf );
        }
        forceRedraw();
        emit sig_buildMenu();
    }
}

void TilingMaker::clearTranslationVectors()
{
    wTrans1_start = QPointF();
    wTrans1_end   = QPointF();
    wTrans2_start = QPointF();
    wTrans2_end   = QPointF();
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
     addToAllPlacedFeatures(make_shared<PlacedFeature>(f,t));
     forceRedraw();
     emit sig_buildMenu();
}

void TilingMaker::deleteFeature(TilingSelectionPtr sel )
{
    if (sel)
    {
        removeFeature(sel);
        emit sig_buildMenu();
        forceRedraw();
    }
}

void TilingMaker::mirrorPolygonX(TilingSelectionPtr sel )
{
    if (sel)
    {
        PlacedFeaturePtr pfp = sel->getPlacedFeature();
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
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::mirrorPolygonY(TilingSelectionPtr sel )
{
    if (sel)
    {
        PlacedFeaturePtr pfp = sel->getPlacedFeature();
        EdgePoly & ep = pfp->getFeature()->getEdgePoly();
        QTransform t = QTransform::fromScale(1,-1);
        ep.mapD(t);
        forceRedraw();
        emit sig_refreshMenu();
    }
}

void TilingMaker::copyPolygon(TilingSelectionPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getPlacedFeature();
        addToAllPlacedFeatures(make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform()));
        forceRedraw();
        emit sig_buildMenu();
    }
}

TilingSelectionPtr TilingMaker::findFeatureUnderMouse()
{
    return findFeature(sMousePos);
}

////////////////////////////////////////////////////////////////////////////
//
// Mouse events.
//
////////////////////////////////////////////////////////////////////////////

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

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToWorld(sMousePos)  << strMouseMode[mouse_mode];

    updateUnderMouse(sMousePos);

    switch (mouse_mode)
    {
    case NO_MOUSE_MODE:
    case COPY_MODE:
    case TRANSLATION_VECTOR_MODE:
    case DRAW_POLY_MODE:
    case MEASURE_MODE:
    case POSITION_MODE:
    case BKGD_SKEW_MODE:
    case EDIT_FEATURE_MODE:
    case EDGE_CURVE_MODE:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case DELETE_MODE:
    case MIRROR_X_MODE:
    case MIRROR_Y_MODE:
    case INCLUSION_MODE:
        break;
    }
}

void TilingMaker::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (config->viewerType != VIEW_TILING_MAKER)
        return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "press start:" << sMousePos << screenToWorld(sMousePos);

    TilingSelectionPtr sel;

    switch (mouse_mode)
    {
    case NO_MOUSE_MODE:
        startMouseInteraction(sMousePos,btn);
        break;

    case COPY_MODE:
        sel = findFeatureUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(this, sel, sMousePos);
        break;

    case DELETE_MODE:
        deleteFeature( findFeatureUnderMouse() );
        setMouseMode(NO_MOUSE_MODE);
        break;

    case TRANSLATION_VECTOR_MODE:
        sel = findSelection(sMousePos);
        if (sel)
            mouse_interaction = make_shared<DrawTranslation>(this, sel, sMousePos);
        break;

    case DRAW_POLY_MODE:
        mouse_interaction = make_shared<CreatePolygon>(this, sMousePos);
        break;

    case INCLUSION_MODE:
        toggleInclusion(findFeatureUnderMouse());
        break;

    case MEASURE_MODE:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            TilingSelectionPtr nothing;
            TilingSelectionPtr sel2 = findEdge(spt,nothing);
            if (sel2)
            {
                QLineF line = sel2->getPlacedLine();
                qreal dist = Point::distToLine(screenToWorld(spt),line);
                qDebug() << "dist" << dist << "line" << line;
                mouse_interaction = make_shared<Measure>(this, sMousePos,sel2);
            }
        }
        else
        {
            mouse_interaction = make_shared<Measure>(this, sMousePos,sel);
        }
        break;
    }

    case POSITION_MODE:
        break;

    case BKGD_SKEW_MODE:
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

    case EDIT_FEATURE_MODE:
        sel = findSelection(spt);
        if (sel)
        {
            if (!editFeature && sel->getType() == INTERIOR)
            {
                editFeature = sel->getPlacedFeature();
            }
            else if (editFeature && sel->getType() == VERTEX )
            {
                mouse_interaction = make_shared<EditFeature>(this, sel, editFeature, sMousePos);
            }
        }
        break;
    case EDGE_CURVE_MODE:
        sel = findArcPoint(spt);
        if (sel)
        {
            currentSelection = sel;
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
                    edge->calcArcCenter(true);  // default to convex
                    sel->getFeature()->setRegular(false);
                    qDebug() << "edge converted to curve";
                }
                else if (edge->getType() == EDGETYPE_CURVE)
                {
                    currentSelection = sel;
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

    case MIRROR_X_MODE:
        mirrorPolygonX(findFeatureUnderMouse());
        break;

    case MIRROR_Y_MODE:
        mirrorPolygonY(findFeatureUnderMouse());
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
    switch (mouse_mode)
    {
    case NO_MOUSE_MODE:
    case MEASURE_MODE:
    case COPY_MODE:
    case DELETE_MODE:
    case INCLUSION_MODE:
    case EDIT_FEATURE_MODE:
    case MIRROR_X_MODE:
    case MIRROR_Y_MODE:
        currentSelection = findSelection(spt);
        forceRedraw();
        if (currentSelection && currentSelection->getType() == INTERIOR)
        {
            currentFeature = currentSelection->getPlacedFeature();
            emit sig_current_feature(allPlacedFeatures.indexOf(currentFeature));
        }
        else
        {
            currentFeature.reset();
        }
        break;

    case TRANSLATION_VECTOR_MODE:
        currentSelection = findSelection(spt);
        if (currentSelection)
            forceRedraw();
        break;

    case DRAW_POLY_MODE:
        currentSelection = findVertex(spt);
        if (!currentSelection)
        {
            currentSelection = findNearGridPoint(spt);
        }
        forceRedraw();
        break;

    case POSITION_MODE:
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

    case EDGE_CURVE_MODE:
        currentSelection = findArcPoint(spt);
        if (currentSelection)
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            currentSelection = findEdge(spt);
            if (currentSelection)
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                currentSelection.reset();
            }
        }
        forceRedraw();
        break;

    case BKGD_SKEW_MODE:

        break;  // do nothing

    }
}

void TilingMaker::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    Q_ASSERT(mouse_mode == NO_MOUSE_MODE);

    TilingSelectionPtr sel = findSelection(spt);
    if(sel)
    {
        // do this first
        wAccum.clear();
        mouse_interaction.reset();

        switch (mouseButton)
        {
        case Qt::LeftButton:
            switch (sel->getType())
            {
            case VERTEX:
                mouse_interaction = make_shared<JoinPoint>(this,sel, spt);
                break;
            case MID_POINT:
                mouse_interaction = make_shared<JoinMidPoint>(this,sel, spt);
                break;
            case EDGE:
                mouse_interaction = make_shared<JoinEdge>(this, sel, spt);
                break;
            case INTERIOR:
                mouse_interaction = make_shared<MovePolygon>(this, sel, spt);
                break;
            case NOTHING:
            case ARC_POINT:
            case FEAT_CENTER:
                if (config->kbdMode == KBD_MODE_CENTER)
                {
                    xf_canvas.setCenter(spt);
                }
                break;
            case SCREEN_POINT:
                break;
            }
            break;

        case Qt::MiddleButton:
            mouse_interaction = make_shared<DrawTranslation>(this, sel, spt);
            break;

        case Qt::RightButton:
            switch (sel->getType())
            {
            case VERTEX:
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
                menuSelection = sel;    // save
                menuSpt       = spt;    // save
                QMenu myMenu;
                myMenu.addSection("Options");
                myMenu.addSeparator();
                myMenu.addAction("Copy/Move", this, SLOT(slot_copyMoveFeature()));
                myMenu.addAction("Include", this, SLOT(slot_includeFeature()));
                myMenu.addAction("Exclude", this, SLOT(slot_excludeFeature()));
                myMenu.addAction("Delete", this, SLOT(slot_deleteFeature()));
                myMenu.addAction("Uniquify", this, SLOT(slot_uniquifyFeature()));
                myMenu.exec(view->mapToGlobal(spt.toPoint()));
            }
                break;

            case NOTHING:
            case ARC_POINT:
            case FEAT_CENTER:
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
        currentFeature.reset();
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
    QString s = strMouseMode[mouse_mode];
    if (mouse_interaction)
    {
        s += " ";
        s += mouse_interaction->desc;
    }
    return s;
}


void TilingMaker::slot_flatenCurve()
{
    EdgePtr ep = currentSelection->getModelEdge();
    ep->resetCurve();
    forceRedraw();
    qDebug() << "edge converted to LINE";
}

void TilingMaker::slot_makeConvex()
{
    EdgePtr ep = currentSelection->getModelEdge();
    ep->setConvex(true);
    forceRedraw();
}

void TilingMaker::slot_makeConcave()
{
    EdgePtr ep = currentSelection->getModelEdge();
    ep->setConvex(false);
    forceRedraw();
}

void TilingMaker::slot_moveArcCenter()
{
    mouse_interaction = make_shared<EditEdge>(this,currentSelection,QPointF());
}

void TilingMaker::slot_editMagnitude()
{
    mouse_interaction.reset();
    DlgMagnitude dlg(currentSelection);
    connect(&dlg, &DlgMagnitude::sig_magnitudeChanged, this, &TilingMaker::forceRedraw);
    dlg.exec();
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
        case 'M': emit canvas->sig_raiseMenu(); break;
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
        case Qt::Key_Escape: setMouseMode(NO_MOUSE_MODE); return false;     // propagate
        case Qt::Key_F3: setMouseMode(TRANSLATION_VECTOR_MODE); break;
        case Qt::Key_F4: setMouseMode(DRAW_POLY_MODE); break;
        case Qt::Key_F5: setMouseMode(COPY_MODE); break;
        case Qt::Key_F6: setMouseMode(DELETE_MODE); break;
        case Qt::Key_F7: setMouseMode(INCLUSION_MODE); break;
        case Qt::Key_F9: setMouseMode(MEASURE_MODE); break;
        case Qt::Key_F10: setMouseMode(BKGD_SKEW_MODE); break;
        case Qt::Key_F11: setMouseMode(EDIT_FEATURE_MODE); break;
        case Qt::Key_F12: setMouseMode(EDGE_CURVE_MODE); break;

        default: return false;
   }

   return true;
}
