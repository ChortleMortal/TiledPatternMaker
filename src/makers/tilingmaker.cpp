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
#include "makers/tilingmaker.h"
#include "makers/TilingSelection.h"
#include "geometry/Point.h"
#include "viewers/workspaceviewer.h"

TilingMaker * TilingMaker::mpThis = nullptr;

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
    Enum2Str(MIRROR_T_MODE)
};


TilingMaker * TilingMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new TilingMaker;
    }
    return mpThis;
}

TilingMaker::TilingMaker() : TilingMakerView()
{
    qDebug() << "TilingMaker::TilingMaker";

    canvas = Canvas::getInstance();

    view = View::getInstance();
    connect(view, &View::sig_mousePressed,  this, &TilingMaker::slot_mousePressed);
    connect(view, &View::sig_mouseDragged,  this, &TilingMaker::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &TilingMaker::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &TilingMaker::slot_mouseMoved);
    connect(view, &View::sig_procKeyEvent,  this, &TilingMaker::slot_procKeyEvent);

    mouse_mode = NO_MOUSE_MODE;
}

TilingMaker::~TilingMaker()
{}

void TilingMaker::setTiling(TilingPtr tiling)
{
    clearDesignerData();

    _tiling = tiling;

    setupDesigner(tiling);

    if (config->viewerType == VIEW_TILIING_MAKER)
    {
        forceRedraw();
    }
}

void TilingMaker::clearDesignerData()
{
    allPlacedFeatures.clear();
    in_tiling.clear();
    wAccum.clear();
    wMeasurements.clear();

    currentSelection.reset();
    currentFeature.reset();
    editFeature.reset();
    mouse_interaction.reset();

    mouse_mode    = NO_MOUSE_MODE;

    wTrans1_start = QPointF();
    wTrans1_end   = QPointF();
    wTrans2_start = QPointF();
    wTrans2_end   = QPointF();

    _tiling.reset();

    convex = true;
}

void TilingMaker::setupDesigner(TilingPtr tiling)
{
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

    Xform xf = tiling->getViewTransform();
    setDeltas(xf);
}

void TilingMaker::viewRectChanged()
{
    QRect rect = view->rect();
    QPointF pt = view->sceneRect().center();
    setRotateCenter(pt);

    int height = rect.height();
    int width  = rect.width();
    SizeAndBounds sab = WorkspaceViewer::viewDimensions[VIEW_TILIING_MAKER];
    sab.viewSize = QSize(width,height);
    QTransform t0 = WorkspaceViewer::calculateViewTransform(sab);
    qDebug().noquote() << Transform::toInfoString(layerT) << "-" << Transform::toInfoString(t0);

    deltaTrans = Transform::trans(t0) - Transform::trans(baseT);
    qDebug() << "deltaTrans=" << deltaTrans;

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
    _tiling->getPlacedFeatures().clear();

    // We assume the data has already been verified (verifyTiling() == true).
    for (auto placedFeature :  in_tiling)
    {
        _tiling->add(placedFeature);
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
        switch (currentSelection->getType())
        {
        case NOTHING:
            switch(mouse_mode)
            {
            case DRAW_POLY_MODE:
            case BKGD_SKEW_MODE:
            {
                QPen pen(drag_color,1);
                g2d->drawLine(wAccum.last()->getV2()->getPosition(),screenToWorld(sMousePos), pen);
                g2d->drawCircle(screenToWorld(sMousePos),10, pen, QBrush(drag_color));
            }
                break;

            default:
                break;
            }
            break;
        case INTERIOR:
            // nothing - handled by currentFeature
            break;

        case EDGE:
        {
            QLineF line  = currentSelection->getPlacedLine();
            qreal thickness = Transform::distFromInvertedZero(g2d->getTransform(),5.0);
            g2d->drawThickLine(line, thickness, QPen(under_mouse_color));
            break;
        }
        case VERTEX:
        case MID_POINT:
        {
            g2d->drawCircle(currentSelection->getPlacedPoint(), 10, QPen(under_mouse_color),QBrush(under_mouse_color));
            break;
        }
        case ARC_POINT:
            g2d->drawCircle(currentSelection->getPlacedPoint(), 10, QPen(under_mouse_color), QBrush(under_mouse_color));
            break;
        }
    }

    if (currentFeature)
    {
        drawFeature( g2d, currentFeature, true, under_mouse_color );
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

    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        if (pf == currentFeature)
        {
            continue;
        }
        if(overlapping.contains(pf))
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
    allPlacedFeatures.push_back(pf);

    if (config->viewerType == VIEW_TILIING_MAKER)
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

    for (auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        if (!in_tiling.contains(pf))
        {
            toRemove.push_back(pf);
        }
    }

    for (auto it = toRemove.begin(); it != toRemove.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
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
        emit sig_buildMenu();
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


void TilingMaker::createFillCopies()
{
    // Create copies of the feature to help visualise the result in the panel.

    if ( isTranslationInvalid() )
        return;

    QPointF t1 = getTrans1();
    QPointF t2 = getTrans2();
    QVector<PlacedFeaturePtr>::iterator it;
    for (it = in_tiling.begin(); it != in_tiling.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
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
    _showOverlaps = checked;
    forceRedraw();
}

void TilingMaker::allDeltaX(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(qdelta,0.0);
        pfp->setTransform(t);
    }
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::allDeltaY(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromTranslate(0.0,qdelta);
        pfp->setTransform(t);
    }
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::allDeltaScale(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform::fromScale(1.0+qdelta,1.0+qdelta);
        pfp->setTransform(t);
    }
    forceRedraw();
    emit sig_refreshMenu();
}

void TilingMaker::allDeltaRotate(int delta)
{
    qreal qdelta = 0.01 * delta;
    for (auto pfp : allPlacedFeatures)
    {
        QTransform t = pfp->getTransform();
        t *= QTransform().rotateRadians(qdelta);
        pfp->setTransform(t);
    }
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

void TilingMaker::fixupTranslate()
{
    wTrans1_end   = wTrans1_start + _tiling->getTrans1();
    wTrans2_end   = wTrans2_start + _tiling->getTrans2();
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

void TilingMaker::clearTranslation()
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
    poly_side_count = number;
}

void TilingMaker::addRegularPolygon()
{
    Q_ASSERT(poly_side_count > 2);

     FeaturePtr f = make_shared<Feature>(poly_side_count);
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
        QTransform t = QTransform::fromScale(-1,1);
        ep.mapD(t);
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

void TilingMaker::setConvexEdge(bool convex)
{
    this->convex = convex;
}


////////////////////////////////////////////////////////////////////////////
//
// Mouse events.
//
////////////////////////////////////////////////////////////////////////////

void TilingMaker::slot_mouseMoved(QPointF spt)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    spt -= deltaTrans;
    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}

void TilingMaker::slot_mouseDragged(QPointF spt)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    spt -= deltaTrans;
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
    case CURVE_EDGE_MODE:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case DELETE_MODE:
    case MIRROR_X_MODE:
    case MIRROR_Y_MODE:
    case INCLUSION_MODE:
    case FLATTEN_EDGE_MODE:
        break;
    }
}

void TilingMaker::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    spt -= deltaTrans;
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
        sel = findVertex(sMousePos);
        if (sel)
            mouse_interaction = make_shared<DrawTranslation>(this, sel, sMousePos);
        else
        {
            sel = findFeature(sMousePos);
            if (sel)
                mouse_interaction = make_shared<DrawTranslation>(this, sel, sMousePos);
        }
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

    case CURVE_EDGE_MODE:
        sel = findEdge(spt);
        if (sel)
        {
            EdgePtr ep = sel->getModelEdge();
            QPointF pt = ep->calcDefaultArcCenter(convex);
            ep->setArcCenter(pt,convex);   // default to start with
            sel->getFeature()->setRegular(false);
            qDebug() << "edge converted to curve";
        }
        else
        {
            sel = findArcPoint(spt);
            if (sel)
            {
                mouse_interaction = make_shared<EditEdge>(this,sel,spt);
            }
        }
        break;

    case FLATTEN_EDGE_MODE:
        sel = findEdge(spt);
        if (sel)
        {
            EdgePtr ep = sel->getModelEdge();
            ep->resetCurve();
            forceRedraw();
            qDebug() << "edge converted to LINE";
        }
        setMouseMode(NO_MOUSE_MODE);
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

    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    spt -= deltaTrans;
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
        currentSelection = findSelection(spt);
        forceRedraw();
        if (currentSelection && currentSelection->getType() == INTERIOR)
        {
            currentFeature = currentSelection->getPlacedFeature();
            emit sig_current_feature(allPlacedFeatures.indexOf(currentFeature));
        }
        break;

    case TRANSLATION_VECTOR_MODE:
        currentSelection = findPoint(spt);
        if (currentSelection)
        {
            forceRedraw();
        }
        else
        {
            currentSelection = findFeature(spt);
            if (currentSelection)
            {
                PlacedFeaturePtr currentFeature = currentSelection->getPlacedFeature();
                emit sig_current_feature(allPlacedFeatures.indexOf(currentFeature));
            }
        }
        break;

    case DRAW_POLY_MODE:
        currentSelection = findVertex(spt);
        if (currentSelection)
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

    case CURVE_EDGE_MODE:
        currentSelection = findEdge(spt);
        if (!currentSelection)
            currentSelection = findArcPoint(spt);
        if (currentSelection)
            forceRedraw();
        break;

    case FLATTEN_EDGE_MODE:
        currentSelection = findEdge(spt);
        break;

    case BKGD_SKEW_MODE:
    case MIRROR_X_MODE:
    case MIRROR_Y_MODE:
        break;  // do nothing

    }
}

void TilingMaker::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
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
                myMenu.addAction("Copy/Move", this, SLOT(slot_copyMoveFeature()));
                myMenu.addAction("Include", this, SLOT(slot_includeFeature()));
                myMenu.addAction("Exclude", this, SLOT(slot_excludeFeature()));
                myMenu.addAction("Delete", this, SLOT(slot_deleteFeature()));
                myMenu.exec(view->mapToGlobal(spt.toPoint()));
             }
                break;

             case NOTHING:
             case ARC_POINT:
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

void TilingMaker::slot_xformMode_changed(int row)
{
    qDebug() << "slot_xformMode_changed"  << row;
    eTileMakerXform tmx = static_cast<eTileMakerXform>(row);
    switch (tmx)
    {
    case XF_VIEW:
        qDebug() << "view";
        canvas->setKbdMode(KBD_MODE_TRANSFORM);
        break;

    case XF_TILING:
        qDebug() << "tiling data";
        canvas->setKbdMode(KBD_MODE_DATA);
        break;

    case XF_BKGD:
        qDebug() << "background";
        canvas->setKbdMode(KBD_MODE_BKGD);
        break;
    }
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

void TilingMaker::slot_procKeyEvent(QKeyEvent * k)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    bool consumed = true;
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
        case 'V': clearTranslation(); break;
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
        case Qt::Key_Escape: setMouseMode(NO_MOUSE_MODE); consumed = false; break;
        case Qt::Key_F3: setMouseMode(TRANSLATION_VECTOR_MODE); break;
        case Qt::Key_F4: setMouseMode(DRAW_POLY_MODE); break;
        case Qt::Key_F5: setMouseMode(COPY_MODE); break;
        case Qt::Key_F6: setMouseMode(DELETE_MODE); break;
        case Qt::Key_F7: setMouseMode(INCLUSION_MODE); break;
        case Qt::Key_F9: setMouseMode(MEASURE_MODE); break;
        case Qt::Key_F10: setMouseMode(BKGD_SKEW_MODE); break;
        case Qt::Key_F11: setMouseMode(EDIT_FEATURE_MODE); break;
        case Qt::Key_F12: setMouseMode(CURVE_EDGE_MODE); break;

        default: consumed = false; break;
   }

   if (!consumed)
        canvas->procKeyEvent(k);
}





