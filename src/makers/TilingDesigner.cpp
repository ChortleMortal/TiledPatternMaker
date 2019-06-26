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

#include "TilingDesigner.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "base/shortcuts.h"
#include "makers/TilingSelection.h"

TilingDesigner * TilingDesigner::mpThis = nullptr;

static bool debugMouse = false;

static QString strMouseMode[] =
{
    Enum2Str(NO_MODE),
    Enum2Str(COPY_MODE),
    Enum2Str(DELETE_MODE),
    Enum2Str(TRANS_MODE),
    Enum2Str(DRAW_POLY_MODE),
    Enum2Str(INCLUSION_MODE),
    Enum2Str(TRANSFORM_MODE),
    Enum2Str(POSITION_MODE),
    Enum2Str(MEASURE_MODE)
};


TilingDesigner * TilingDesigner::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new TilingDesigner;
    }
    return mpThis;
}

TilingDesigner::TilingDesigner() : TilingDesignerView()
{
    qDebug() << "TilingDesigner::TilingDesigner";

    Bounds b(-5.0,5.0,10.0);
    setBounds(b);

    canvas = Canvas::getInstance();
    config = Configuration::getInstance();

    view = View::getInstance();
    connect(view, &View::sig_mousePressed,  this, &TilingDesigner::slot_mousePressed);
    connect(view, &View::sig_mouseDragged,  this, &TilingDesigner::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &TilingDesigner::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &TilingDesigner::slot_mouseMoved);
    connect(view, &View::keyEvent,          this, &TilingDesigner::procKey);

    mouse_mode      = NO_MODE;
    drawAllOverlaps = false;
}

TilingDesigner::~TilingDesigner()
{
    placed_features.clear();
    in_tiling.clear();
    overlapping.clear();
    touching.clear();
}

void TilingDesigner::setTiling(TilingPtr tiling)
{
    clearDesigner();

    setupDesigner(tiling);

    forceRedraw();
}

void TilingDesigner::clearDesigner()
{
    placed_features.clear();
    in_tiling.clear();
    overlapping.clear();
    touching.clear();
    accum.clear();
    measurements.clear();

    currentSelection.reset();
    mouse_interaction.reset();

    mouse_mode   = NO_MODE;
    trans1_start = QPointF();
    trans1_end   = QPointF();
    trans2_start = QPointF();
    trans2_end   = QPointF();
}

void TilingDesigner::setupDesigner(TilingPtr tiling)
{
    QList<PlacedFeaturePtr> & qlpf = tiling->getPlacedFeatures();
    QList<PlacedFeaturePtr>::const_iterator it;

    for(it = qlpf.begin(); it != qlpf.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        addFeaturePF(pf);
        in_tiling.push_back(pf);
    }

    QPointF trans_origin;
    if (placed_features.size() > 0 )
    {
        PlacedFeaturePtr pf = placed_features.first();
        Transform T         = pf->getTransform();
        trans_origin        = T.apply(pf->getFeature()->getCenter());
    }
    else
    {
        trans_origin = QPointF( 0, 0 );
    }

    trans1_start = trans_origin;
    trans1_end   = trans_origin + tiling->getTrans1();

    trans2_start = trans_origin;
    trans2_end   = trans_origin + tiling->getTrans2();

    Configuration * config = Configuration::getInstance();
    if (config->repeatMode != REPEAT_SINGLE)
    {
        createCopies();
    }

}

void TilingDesigner::updateTilingFromData(TilingPtr tiling)
{
    if ( placed_features.size() <= 0 || in_tiling.size() <= 0 )
    {
        return;
    }

    // need to remove what is there already
    tiling->getPlacedFeatures().clear();

    // We assume the data has already been verified (verifyTiling() == true).
    for (auto it = in_tiling.begin(); it != in_tiling.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        tiling->add(pf);
    }
}

bool TilingDesigner::verifyTiling()
{
    if ( placed_features.size() <= 0 )
    {
        qWarning("There are no polgons");
        return false;
    }

    if ( in_tiling.size() <= 0 )
    {
        qWarning("No selected tiles");
        return false;
    }

    if ( isTranslationInvalid() )
    {
        qWarning("Translation vectors not defined");
        return false;
    }
    return true;
}

void TilingDesigner::draw( GeoGraphics * g2d )
{
    updateOverlaps();

    for(auto it = placed_features.begin(); it != placed_features.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;
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

    if (currentSelection)
    {
        switch (currentSelection->type)
        {
        case NOTHING:
            break;
        case INTERIOR:
        {
            PlacedFeaturePtr pf = currentSelection->getFeature();
            drawFeature( g2d, pf, true, under_mouse_color );
            break;
        }
        case EDGE:
        {
            QLineF line  = currentSelection->getLine();
            g2d->setColor(under_mouse_color);
            qreal thickness = g2d->getTransform().distFromInvertedZero(5.0);
            g2d->drawThickLine(line, thickness);
            break;
        }
        case VERTEX:
        case MID_POINT:
        {
            qreal radius = g2d->getTransform().distFromInvertedZero(5.0);
            g2d->setColor(under_mouse_color);
            g2d->drawCircle( currentSelection->getPoint(), radius, true);
            break;
        }
        }
    }

    if ( accum.size() > 0)
    {
        g2d->setColor(construction_color);
        qreal radius = g2d->getTransform().distFromInvertedZero(3.0);
        for( int idx = 1; idx < accum.size(); ++idx )
        {
            QPointF p1 = accum[idx - 1];
            QPointF p2 = accum[idx];
            g2d->drawCircle( p1, radius, true );
            g2d->drawCircle( p2, radius, true );
            g2d->drawLine( p1, p2 );
        }
    }

    qreal arrow_length = g2d->getTransform().distFromInvertedZero(8.0);
    qreal arrow_width  = g2d->getTransform().distFromInvertedZero(4.0);

    if (trans1_start != trans1_end)
    {
        g2d->setColor(construction_color);
        g2d->drawLine(trans1_start, trans1_end);
        g2d->drawArrow(trans1_start, trans1_end, arrow_length, arrow_width, true );
        g2d->drawText(worldToScreen(trans1_end) + QPointF(10,0),"T1");
    }

    if (trans2_start != trans2_end)
    {
        g2d->setColor(construction_color);
        g2d->drawLine(trans2_start, trans2_end );
        g2d->drawArrow(trans2_start, trans2_end, arrow_length, arrow_width, true );
        g2d->drawText(worldToScreen(trans2_end) + QPointF(10,0),"T2");
    }

    if (mouse_interaction)
        mouse_interaction->draw(g2d);

    for (auto it = measurements.begin(); it != measurements.end(); it++)
    {
        Measurement & mm = *it;
        g2d->setColor(construction_color);
        g2d->drawLineS(mm.startS(), mm.endS());
        QString msg = QString("%1 (%2)").arg(QString::number(mm.lenS(),'f',2)).arg(QString::number(mm.len(),'f',8));
        g2d->drawText(mm.endS() + QPointF(10,0),msg);
    }
}

void TilingDesigner::setMouseMode(eMouseMode mode)
{
    mouse_mode = mode;

    if (mode == TRANSFORM_MODE)
    {
        canvas->setMode(MODE_TRANSFORM);
    }
    else
    {
        canvas->setMode(MODE_NONE);
    }

    forgetPolygon();

    forceRedraw();
    setFocus();
}

eMouseMode TilingDesigner::getMouseMode()
{
    return mouse_mode;
}

// Feature management.

int TilingDesigner::addFeaturePF(PlacedFeaturePtr pf)
{
    int add_pos = placed_features.size();
    placed_features.push_back(pf);

    forceRedraw();
    return add_pos;
}

TilingSelectionPtr TilingDesigner::addFeatureSP(TilingSelectionPtr sel)
{
    PlacedFeaturePtr pf    = sel->getFeature();
    PlacedFeaturePtr pfnew = make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform());
    addFeaturePF(pfnew);
    TilingSelectionPtr ret = make_shared<TilingSelection>(INTERIOR,pfnew);
    return ret;
}

void TilingDesigner::removeFeature(PlacedFeaturePtr pf )
{
    placed_features.removeOne(pf);
    in_tiling.removeOne( pf );
    forceRedraw();
}

void TilingDesigner::removeFeature(TilingSelectionPtr sel)
{
    if ( currentSelection && sel && currentSelection->getFeature() == sel->getFeature())
    {
        currentSelection.reset();
    }
    removeFeature(sel->getFeature());
}

void TilingDesigner::addInTiling(PlacedFeaturePtr pf)
{
    in_tiling.push_back(pf);
}

void TilingDesigner::removeFromTiling(PlacedFeaturePtr pf)
{
    in_tiling.removeAll(pf);
}

void TilingDesigner::fillUsingTranslations()
{
    if (!verifyTiling())
        return;

    removeExcluded();
    createCopies();
    emit hasChanged();
}

void TilingDesigner::removeExcluded()
{
    for ( int i = placed_features.size() - 1; i >= 0; --i )
    {
        PlacedFeaturePtr pf = placed_features[i];
        if ( ! in_tiling.contains( pf ) )
        {
            placed_features.remove(i);
        }
    }

    currentSelection.reset();

    forceRedraw();
    emit hasChanged();
}

void TilingDesigner::excludeAll()
{
    in_tiling.clear();
    forceRedraw();
    emit hasChanged();
}

void TilingDesigner::createCopies()
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
        Transform T = pf->getTransform();

        for( int y = -1; y <= 1; ++y )
        {
            for( int x = -1; x <= 1; ++x )
            {
                if ( y == 0 && x == 0 )
                    continue;

                Transform placement = Transform::translate( (t1 * x ) + (t2 * y) ).compose( T );
                addFeaturePF(make_shared<PlacedFeature>( f, placement ) );
            }
        }
    }
    forceRedraw();
}

////////////////////////////////////////////////////////////////////////////
//
// Validating that features don't overlap when tiled.
//
// Note: we're not very stringent to avoid flagging polygons that would
//       only slightly overlap. This is more about including completely
//       unnecessary polygons that cover area already covered by another
//       copy, or other gross errors that are hard to see (especially once
//       the plane if fully tiled!).
//
// casper - this is replaced with a more robust implementation which
// also distinguishes between touching and overlapping.  Also no attempt
// is made to deal with anythihng outside of in_tiling

void TilingDesigner::updateOverlaps()
{
    overlapping.clear();
    touching.clear();

    QVector<PlacedFeaturePtr> & pfeatures = (drawAllOverlaps) ? placed_features : in_tiling;

    for (auto it = pfeatures.begin(); it != pfeatures.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        Transform T = pf->getTransform();
        QPolygonF poly = T.apply(pf->getFeature()->getPolygon());
        for (auto it2 = pfeatures.begin(); it2 != pfeatures.end(); it2++)
        {
            PlacedFeaturePtr pf2 = *it2;
            if (pf2 == pf)
            {
                continue;
            }
            Transform T2 = pf2->getTransform();
            QPolygonF poly2 = T2.apply(pf2->getFeature()->getPolygon());
            if (poly2.intersects(poly))
            {
                QPolygonF p3 = poly2.intersected(poly);
                if (!p3.isEmpty())
                {
                    qDebug() << "overlapping";
                    if (!overlapping.contains(pf))
                        overlapping.push_back(pf);
                    if (!overlapping.contains(pf2))
                        overlapping.push_back(pf2);
                }
                else
                {
                    qDebug() << "touching";
                    if (!touching.contains(pf))
                        touching.push_back(pf);
                    if (!touching.contains(pf2))
                        touching.push_back(pf2);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Tiling translation vector.
//
// We treat the two vector as a circular buffer so that each one will
// get over-written in turn, so that both can be alternatively changed
// by the end-user.

void TilingDesigner::addToTranslate(QPointF pt, bool ending )
{
    if  (trans1_start.isNull())
    {
        trans1_start = pt;
    }
    else if (trans1_end.isNull())
    {
        if (ending && (pt != trans1_start))
        {
            trans1_end = pt;
        }
    }
    else if (!ending )
    {
        trans2_start = trans1_start;
        trans2_end   = trans1_end;
        trans1_start = pt;
        trans1_end   = QPointF();
    }
    forceRedraw();
}

bool TilingDesigner::isTranslationInvalid()
{
    if  (   (trans1_start == trans1_end)
         || (trans2_start == trans2_end))
        return true;
    else
        return false;
}

void TilingDesigner::updateUnderMouse(QPointF spt)
{
    switch (mouse_mode)
    {
    case NO_MODE:
    case MEASURE_MODE:
    case POSITION_MODE:
    case COPY_MODE:
    case DELETE_MODE:
    case INCLUSION_MODE:
        currentSelection = findSelection(spt);
        forceRedraw();
        if (currentSelection)
        {
            PlacedFeaturePtr currentFeature = currentSelection->getFeature();
            emit sig_current_feature(placed_features.indexOf(currentFeature));
        }
        break;
    case TRANS_MODE:
        currentSelection = findVertex(spt);
        if (!currentSelection)
        {
            currentSelection = findSelection(spt);
            if (currentSelection)
            {
                PlacedFeaturePtr currentFeature = currentSelection->getFeature();
                emit sig_current_feature(placed_features.indexOf(currentFeature));
            }
         }
         if (currentSelection)
            forceRedraw();
        break;
    case DRAW_POLY_MODE:
        currentSelection = findVertex(spt);
        if (currentSelection)
            forceRedraw();
        break;
    case TRANSFORM_MODE:
        currentSelection.reset();
        break;
    }
}

// Internal tiling creation.

QPointF TilingDesigner::getTrans1()
{
    if (trans1_start == trans1_end)
        return QPointF();
    else
        return trans1_end - trans1_start ;
}

QPointF TilingDesigner::getTrans2()
{
    if (trans2_start == trans2_end)
        return QPointF();
    else
        return trans2_end - trans2_start;
}

void TilingDesigner::toggleInclusion(TilingSelectionPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getFeature();
        if( in_tiling.contains(pf))
        {
            removeFromTiling( pf );
        }
        else
        {
            addInTiling( pf );
        }
        forceRedraw();
    }
}

void TilingDesigner::clearTranslation()
{
    trans1_start = QPointF();
    trans1_end   = QPointF();
    trans2_start = QPointF();
    trans2_end   = QPointF();
    forceRedraw();
    emit hasChanged();
}

void TilingDesigner::updatePolygonSides(int number)
{
    poly_side_count = number;
}

void TilingDesigner::addRegularPolygon()
{
    Q_ASSERT(poly_side_count > 2);

     FeaturePtr f = make_shared<Feature>(poly_side_count);
     Transform t;
     addFeaturePF(make_shared<PlacedFeature>(f,t));
     canvas->invalidate();
     emit hasChanged();
}

void TilingDesigner::addIrregularPolygon()
{
    if (accum.size() < 3)
        return;

     QPolygonF poly(accum);
     FeaturePtr f = make_shared<Feature>(poly);
     Transform t;
     addFeaturePF(make_shared<PlacedFeature>(f,t));
     accum.clear();
     canvas->invalidate();
     emit hasChanged();
}

void TilingDesigner::deletePolygon(TilingSelectionPtr sel )
{
    if (sel)
    {
        if (currentSelection)
        {
                currentSelection.reset();
        }
        removeFeature(sel);
    }
}

void TilingDesigner::copyPolygon(TilingSelectionPtr sel)
{
    if (sel)
    {
        PlacedFeaturePtr pf = sel->getFeature();
        addFeaturePF(make_shared<PlacedFeature>(pf->getFeature(), pf->getTransform()));
    }
}

void TilingDesigner::forgetPolygon()
{
    accum.clear();
    mouse_interaction.reset();
}

// Mouse tracking.

TilingSelectionPtr TilingDesigner::findFeatureUnderMouse()
{
    return findFeature(mousePos);
}


////////////////////////////////////////////////////////////////////////////
//
// Mouse events.
//
////////////////////////////////////////////////////////////////////////////

void TilingDesigner::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    TilingSelectionPtr sel = findSelection(spt);
    if(sel)
    {
        switch (mouseButton)
        {
        case Qt::LeftButton:
            if (std::dynamic_pointer_cast<DrawPolygon>(mouse_interaction))
            {
                mouse_interaction->updateDragging(spt);
                return;
            }

            switch (sel->type)
            {
            case VERTEX:
            case MID_POINT:
                mouse_interaction = make_shared<DrawPolygon>(this, sel, spt );
                return;
            case EDGE:
                mouse_interaction = make_shared<JoinEdge>(this, sel, spt );
                return;
            case INTERIOR:
                mouse_interaction = make_shared<MovePolygon>(this, sel, spt );
                return;
            case NOTHING:
                break;
            }
            break;
        case Qt::MiddleButton:
            mouse_interaction = make_shared<DrawTranslation>(this, sel, spt );
            return;
        case Qt::RightButton:
            switch (sel->type)
            {
            case EDGE:
                mouse_interaction = make_shared<CopyJoinEdge>(this, sel, spt );
                return;
            case VERTEX:
            case MID_POINT:
            case INTERIOR:
                mouse_interaction = make_shared<CopyMovePolygon>(this, sel, spt );
                return;
             case NOTHING:
                break;
            }
            break;
        default:
            break;
        }
    }

    forgetPolygon();
    mouse_interaction.reset();
}


void TilingDesigner::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    mousePos = spt;

    if (debugMouse && mouse_interaction)
        qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << mouse_interaction->desc;
    else
        qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << "no mouse interaction";

    TilingSelectionPtr sel;

    switch (mouse_mode)
    {
    case NO_MODE:
        startMouseInteraction(mousePos,btn);
        break;
    case COPY_MODE:
        sel = findFeatureUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(this, sel, mousePos);
        break;
    case DELETE_MODE:
        deletePolygon( findFeatureUnderMouse() );
        break;
    case TRANS_MODE:
        sel = findVertex(mousePos);
        if (sel)
            mouse_interaction = make_shared<DrawTranslation>(this, sel, mousePos);
        else
        {
            sel = findFeature(mousePos);
            if (sel)
                mouse_interaction = make_shared<DrawTranslation>(this, sel, mousePos);
        }
        break;
    case DRAW_POLY_MODE:
        sel = findSelection(mousePos);
        // always create the mouseAction
        mouse_interaction = make_shared<DrawPolygon>(this, sel, mousePos);
        if (!sel)
        {
            QPointF wpt = screenToWorld(spt);
            if (!accumHasPoint(spt))
            {
                qDebug() << "adding new point to accum" << wpt;
                accum.push_back(wpt);
                qDebug() << "accum=" << accum;
            }
            else
            {
                qDebug() << "point already exists" << wpt;
                QPointF zpos = worldToScreen(accum[0]);
                if ((accum.size() > 2) &&  Point::isNear(spt, zpos))
                {
                    qDebug("auto-complete the polygon");
                    QPolygonF pgon(accum);
                    Transform t;
                    addFeaturePF(make_shared<PlacedFeature>(make_shared<Feature>(pgon), t));
                    accum.clear();
                    mouse_interaction.reset();
                }
            }
            forceRedraw();
        }
        break;
    case INCLUSION_MODE:
        toggleInclusion(findFeatureUnderMouse());
        break;
    case TRANSFORM_MODE:
        break;
    case MEASURE_MODE:
    {
        TilingSelectionPtr sel;
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            TilingSelectionPtr nothing;
            sel = findEdge(spt,nothing);
            if (sel)
            {
                QLineF line = sel->getLine();
                qreal dist = Point::distToLine(screenToWorld(spt),line);
                qDebug() << "dist" << dist << "line" << line;
                mouse_interaction = make_shared<Measure>(this, mousePos,sel);
            }
        }
        else
        {
            mouse_interaction = make_shared<Measure>(this, mousePos,sel);
        }
        break;
    }
    case POSITION_MODE:
        mouse_interaction = make_shared<Position>(this, mousePos);
        break;
    }

    if (mouse_interaction)
    {
        qDebug().noquote() << "press end:"  << mouse_interaction->desc;
    }
    else
    {
        qDebug() << "press end: no mouse_interaction";
    }
}

void TilingDesigner::slot_mouseDragged(QPointF spt)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos)  << strMouseMode[mouse_mode];

    updateUnderMouse(mousePos);

    switch (mouse_mode)
    {
    case NO_MODE:
    case COPY_MODE:
    case TRANS_MODE:
    case DRAW_POLY_MODE:
    case MEASURE_MODE:
    case POSITION_MODE:
        if (mouse_interaction)
            mouse_interaction->updateDragging(mousePos);
        break;
    case DELETE_MODE:
    case INCLUSION_MODE:
    case TRANSFORM_MODE:
        break;
    }
}

void TilingDesigner::slot_mouseReleased(QPointF spt)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(mousePos);
        mouse_interaction.reset();
    }
    emit hasChanged();
}

void TilingDesigner::slot_mouseMoved(QPointF spt)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    updateUnderMouse(mousePos);
}


void TilingDesigner::setMousePos(QPointF spt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km == Qt::SHIFT)
    {
        mousePos.setX(spt.x());
    }
    else if (km == Qt::CTRL)
    {
        mousePos.setY(spt.y());
    }
    else
    {
        mousePos = spt;
    }
}

bool TilingDesigner::accumHasPoint(QPointF spt)
{
    for (auto it = accum.begin(); it != accum.end(); it++)
    {
        QPointF existing = *it;
        existing = worldToScreen(existing);
        if (Point::isNear(spt,existing))
        {
            return true;
        }
    }
    return false;
}

void TilingDesigner::updateStatus()
{
    QString s("Tiling Designer:: ");
    s += strMouseMode[mouse_mode];
    if (mouse_interaction)
    {
        s += "  ";
        s += mouse_interaction->desc;
    }
    view->setWindowTitle(s);
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

void TilingDesigner::procKey(QKeyEvent * k)
{
    if (config->viewerType != VIEW_TILIING_MAKER)
        return;

    switch (k->key())
    {
        // actions
        case 'A': addRegularPolygon(); break;
        case 'B': addRegularPolygon(); break;
        case 'C': addFeatureSP(findFeatureUnderMouse()); break;
        case 'D': deletePolygon(findFeatureUnderMouse()); break;
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
        case Qt::Key_Escape: setMouseMode(NO_MODE); canvas->procKeyEvent(k); break;
        case Qt::Key_F2: setMouseMode(TRANSFORM_MODE); break;
        case Qt::Key_F3: setMouseMode(TRANS_MODE); break;
        case Qt::Key_F4: setMouseMode(DRAW_POLY_MODE); break;
        case Qt::Key_F5: setMouseMode(COPY_MODE); break;
        case Qt::Key_F6: setMouseMode(DELETE_MODE); break;
        case Qt::Key_F7: setMouseMode(INCLUSION_MODE); break;
        case Qt::Key_F9: setMouseMode(MEASURE_MODE); break;

        default: canvas->procKeyEvent(k); break;
   }
}





