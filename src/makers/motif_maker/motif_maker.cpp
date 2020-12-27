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

#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "style/interlace.h"
#include "style/plain.h"
#include "tapp/explicit_figure.h"
#include "tapp/infer.h"
#include "panels/panel.h"

MotifMaker * MotifMaker::mpThis = nullptr;

MotifMaker * MotifMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new MotifMaker;
    }
    return mpThis;
}

MotifMaker::MotifMaker()
{
    menu = nullptr;
}

void MotifMaker::init()
{
    config          = Configuration::getInstance();
    vcontrol        = ViewControl::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    decorationMaker = DecorationMaker::getInstance();
}

PrototypePtr MotifMaker::createPrototype(TilingPtr tiling)
{
    PrototypePtr prototype = make_shared<Prototype>(tiling);

    QVector<FeaturePtr> uniqueFeatures = tiling->getUniqueFeatures();
    qDebug() << "Create new design elements";
    int count = 0;
    for (auto feature : uniqueFeatures)
    {
        // NOTE this takes order from order of unique features
        if (++count <= MAX_UNIQUE_FEATURE_INDEX)
        {
            DesignElementPtr dep = make_shared<DesignElement>(feature);
            prototype->addElement(dep);
        }
        else
        {
            qWarning() << "Too many unique features in tiling. count:" << count;
        }
    }
    return prototype;
}

void MotifMaker::recreatePrototype(TilingPtr tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    QVector<FeaturePtr> uniqueFeatures = tiling->getUniqueFeatures();
    int count = 0;
    for (auto feature : uniqueFeatures)
    {
        if (++count <= MAX_UNIQUE_FEATURE_INDEX)
        {
            bool found = false;
            for (auto del : prototype->getDesignElements())
            {
                if (del->getFeature() == feature)
                {
                    found = true;
                    if (!del->validFigure())
                    {
                       del->createFigure();
                    }
                    break;
                }
            }
            if (!found)
            {
                DesignElementPtr dep = make_shared<DesignElement>(feature);
                prototype->addElement(dep);
            }
        }
        else
        {
            qWarning() << "Too many unique features in tiling. count:" << count;
        }
    }
    QVector<DesignElementPtr> forDeletion;
    for (auto del : prototype->getDesignElements())
    {
        FeaturePtr feature = del->getFeature();
        if (!uniqueFeatures.contains(feature))
        {
            forDeletion.push_back(del);
        }
    }
    for (auto del : forDeletion)
    {
        prototype->removeElement(del);
    }
}

void MotifMaker::recreateFigures(TilingPtr tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    for (auto del : prototype->getDesignElements())
    {
        FeaturePtr feature = del->getFeature();
        FigurePtr  figure  = del->getFigure();
        if (del->validFigure())
        {
            if (feature->isRegular())
            {
                if (feature->numSides() != figure->getN())
                {
                    figure->setN(feature->numSides());
                    figure->buildExtBoundary();
                    figure->buildMaps();
                }
            }
            else
            {
                ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figure);
                if (feature->numSides() != ep->getN())
                {
                    ep->setN(feature->numSides());
                    ep->buildExtBoundary();
                    ep->buildMaps();
                }
            }
        }
        else
        {
            del->createFigure();
            figure  = del->getFigure();
            figure->buildExtBoundary();
            figure->buildMaps();
        }
    }
}

void MotifMaker::takeDown(PrototypePtr prototype)
{
    qDebug() << "MotifMaker::takeDown()";

    prototypes.push_front(prototype);

    setSelectedPrototype(prototype);

    tilingMaker->sm_take(prototype->getTiling(), SM_LOAD_FROM_MOSAIC);
}

/* There are 5 possible actions
   1. erase all proto, create new proto, and add
   2. erase current proto, create new proto, and add
   3. create new proto using tiling and add
   4. replace tiling in current proto
   5. reset prototype maps
*/

void MotifMaker::sm_eraseAllCreateAdd(TilingPtr tiling)
{
    erasePrototypes();
    PrototypePtr prototype = createPrototype(tiling);
    prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_eraseCurrentCreateAdd(TilingPtr tiling)
{
    prototypes.removeOne(getSelectedPrototype());
    PrototypePtr prototype = createPrototype(tiling);
    prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_createAdd(TilingPtr tiling)
{
    PrototypePtr prototype = createPrototype(tiling);
    prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_replaceTiling(PrototypePtr prototype, TilingPtr tiling)
{
    prototype->setTiling(tiling);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_resetMaps()
{
    for (auto prototype : qAsConst(prototypes))
    {
        prototype->resetProtoMap();
    }
}

void  MotifMaker::sm_take(TilingPtr tiling, eSM_Event event)
{
    eMMState state = sm_getState();
    qDebug().noquote() << "MotifMaker::takeUp() state:" << mm_states[state] << "event" << sSM_Events[event];

    QVector<PrototypePtr> protos;
    switch (event)
    {
    case SM_LOAD_EMPTY:
        sm_eraseAllCreateAdd(tiling);
        protos.push_back(getSelectedPrototype());
        decorationMaker->sm_takeUp(protos,event);
        menu->tilingChoicesChanged();
        break;

    case SM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            sm_eraseAllCreateAdd(tiling);
            //decorationMaker->sm_takeUp(selectedPrototype,event);
        }
        else
        {
             if (state == MM_SINGLE)
             {
#if 0
                 bool createNew = askNewProto();
                 if (createNew)
                 {
                     sm_eraseAllCreateAdd(tiling);
                     decorationMaker->sm_takeUp(selectedPrototype,event);
                 }
                 else
                 {
                     sm_replaceTiling(selectedPrototype, tiling);
                     decorationMaker->sm_takeUp(selectedPrototype,SM_RELOAD_SINGLE);
                 }
#else
                 sm_eraseAllCreateAdd(tiling);
                 protos.push_back(getSelectedPrototype());
                 decorationMaker->sm_takeUp(protos,event);
#endif
             }
             else if (state == MM_MULTI)
             {
                 bool createNew = askNewProto();
                 if (createNew)
                 {
                     sm_eraseCurrentCreateAdd(tiling);
                     protos.push_back(getSelectedPrototype());
                     decorationMaker->sm_takeUp(protos,SM_LOAD_MULTI);
                 }
                 else
                 {
                     sm_replaceTiling(getSelectedPrototype(), tiling);
                     protos.push_back(getSelectedPrototype());
                     decorationMaker->sm_takeUp(protos,SM_RELOAD_MULTI);
                 }
             }
        }
        menu->tilingChoicesChanged();
        break;

    case SM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
            sm_replaceTiling(getSelectedPrototype(),tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            sm_replaceTiling(getSelectedPrototype(),tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);
        }
        else
        {
            Q_ASSERT(state == MM_EMPTY);
            qWarning("MotifMaker ; invalid state");
        }
        menu->tilingChoicesChanged();
        break;

    case SM_LOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == MM_SINGLE)
        {
            sm_createAdd(tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            sm_createAdd(tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);

        }
        menu->tilingChoicesChanged();
        break;

    case SM_RELOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == MM_SINGLE)
        {
            sm_replaceTiling(getSelectedPrototype(),tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            sm_replaceTiling(getSelectedPrototype(),tiling);
            protos.push_back(getSelectedPrototype());
            decorationMaker->sm_takeUp(protos,event);
        }
        menu->tilingChoicesChanged();
        break;

    case SM_FIGURE_CHANGED:
        sm_resetMaps();
        decorationMaker->sm_takeUp(protos,event);
        break;

    case SM_FEATURE_CHANGED:
        Q_ASSERT(tiling == getSelectedPrototype()->getTiling());
        recreateFigures(tiling);
        decorationMaker->sm_takeUp(protos,event);
        menu->featureChanged();
        break;

    case SM_TILING_CHANGED:
        recreatePrototype(tiling);
        menu->tilingChanged();
        decorationMaker->sm_takeUp(protos,event);
        break;

    case SM_RENDER:
        sm_resetMaps();
        for (auto& prototype : prototypes )
        {
            protos.push_back(prototype);
        }
        decorationMaker->sm_takeUp(protos,event);
        break;

    case SM_LOAD_FROM_MOSAIC:
        qWarning("Invalid mode");
        break;
    }
}

const QVector<PrototypePtr> & MotifMaker::getPrototypes()
{
    return prototypes;
}

void MotifMaker::removePrototype(TilingPtr tiling)
{
    QVector<PrototypePtr> forRemoval;

    for (auto prototype : qAsConst(prototypes))
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval.push_back(prototype);
        }
    }

    for (auto& prototype : forRemoval)
    {
        prototypes.removeAll(prototype);
        if (getSelectedPrototype() == prototype)
        {
            resetSelectedPrototype();
        }
    }
}

void MotifMaker::erasePrototypes()
{
    prototypes.clear();
    resetSelectedPrototype();
}

eMMState MotifMaker::sm_getState()
{
    if (prototypes.size() > 1)
    {
        return  MM_MULTI;
    }
    else if (prototypes.size() == 0)
    {
        return MM_EMPTY;
    }
    else
    {
        Q_ASSERT(getSelectedPrototype());
        Q_ASSERT(prototypes.size() == 1);
        if (getSelectedPrototype()->hasContent())
        {
            return MM_SINGLE;
        }
        else
        {
            return MM_EMPTY;
        }
    }
}

// duplication is used to superimpose a second feature in the same figure
// so it has exactly the same placements
void MotifMaker::duplicateActiveFeature()
{
    FeaturePtr fp = getActiveFeature();
    if (!fp) return;
    if (!getSelectedPrototype()) return;

    // create new feature
    FeaturePtr newFeature;
    if (fp->isRegular())
    {
        newFeature = make_shared<Feature>(fp->numPoints(),fp->getRotation());
    }
    else
    {
        newFeature = make_shared<Feature>(fp->getEdgePoly(),fp->getRotation());
    }

    TilingPtr tiling = getSelectedPrototype()->getTiling();
    const QVector<PlacedFeaturePtr> & pfps = tiling->getPlacedFeatures();
    for (auto pfp : pfps)
    {
        if (pfp->getFeature() == fp)
        {
            PlacedFeaturePtr newPlacedFeature = make_shared<PlacedFeature>(newFeature,pfp->getTransform());
            // put placed feature in the tiling
            tiling->add(newPlacedFeature);

            DesignElementPtr newDesignElement = make_shared<DesignElement>(newFeature);
            // put design element in the prototype
            getSelectedPrototype()->addElement(newDesignElement);
            getSelectedPrototype()->resetProtoMap();
            return;
        }
    }
}

void MotifMaker::deleteActiveFeature()
{
    FeaturePtr fp = getActiveFeature();
    if (!fp) return;
    if (!getSelectedPrototype()) return;

    TilingPtr tiling = getSelectedPrototype()->getTiling();
    const QVector<PlacedFeaturePtr> & pfps = tiling->getPlacedFeatures();
    for (auto pfp : pfps)
    {
        if (pfp->getFeature() == fp)
        {
            tiling->remove(pfp);
        }
    }
    getSelectedPrototype()->removeElement(getSelectedDesignElement());
    getSelectedPrototype()->resetProtoMap();
}

////////////////////////////////////////////////////////////////////////////
//
// Build a Prototype containing all the information about the
// design in its current state.  This will then be used to construct
// the final map.  It's also used to infer maps for features.
//
// The prototype clones the design elements stored in the editor,
// so multiple prototypes can be fired off for different purposes and
// those prototypes can be consumed if needed.  This follows nicely
// in my philosophy of "fire and forget" UIs -- the different
// phases of a process should be independent.

void MotifMaker::setSelectedPrototype(PrototypePtr pp)
{
    if (_selectedPrototype == pp)
    {
        return;
    }

    _selectedPrototype = pp;
    if (pp && pp->hasContent())
    {
        _selectedDesignElement = pp->getDesignElement(0);  // first
        // this is often ovverwritten by MotifDisplayWidget
    }
    else
        _selectedDesignElement.reset();
}

PrototypePtr MotifMaker::getSelectedPrototype()
{
    return _selectedPrototype;
}

////////////////////////////////////////////////////////////////////////////
//
// Explicit figure inferring.
//
// Infer a map for the currently selected feature, using
// the app.Infer algorithm.  Absolutely definitely
// guaranteed to not necessarily work or produce satisfactory
// results.

MapPtr MotifMaker::createExplicitInferredMap()
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        return inf->infer( getActiveFeature() );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr MotifMaker::createExplicitFeatureMap()
{
    InferPtr inf = make_shared<Infer>(getSelectedPrototype());
    return inf->inferFeature(getActiveFeature());
}

MapPtr MotifMaker::createExplicitStarMap( qreal d, int s )
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        return inf->inferStar( getActiveFeature(), d, s );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr MotifMaker::createExplicitHourglassMap( qreal d, int s )
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        return inf->inferHourglass( getActiveFeature(), d, s );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr MotifMaker::createExplicitGirihMap( int starSides, qreal starSkip )
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        return inf->inferGirih( getActiveFeature(), starSides, starSkip );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr MotifMaker::createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive )
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        if ( progressive )
        {
            return inf->inferIntersectProgressive( getActiveFeature(), starSides, starSkip, s );
        }
        else
        {
            return inf->inferIntersect( getActiveFeature(), starSides, starSkip, s );
        }
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr MotifMaker::createExplicitRosetteMap( qreal q, int s, qreal r )
{
    try
    {
        InferPtr inf = make_shared<Infer>(getSelectedPrototype());
        return inf->inferRosette( getActiveFeature(), q, s, r );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

PrototypePtr MotifMaker::findPrototypeByName(TilingPtr tiling)
{
    PrototypePtr pp;
    for (auto prototype : qAsConst(prototypes))
    {
        if (prototype->getTiling()->getName() == tiling->getName())
        {
            return prototype;
        }
    }
    return pp;
}

bool MotifMaker::askNewProto()
{
    QMessageBox box(ControlPanel::getInstance());
    box.setIcon(QMessageBox::Question);
    QPushButton *createNewButton  = box.addButton("Create new Prototype", QMessageBox::ActionRole);
    QPushButton *replaceButton    = box.addButton("Replace tiling in existing Prototype", QMessageBox::ActionRole);
    Q_UNUSED(replaceButton);
    box.exec();

    bool createNew = (box.clickedButton() == createNewButton);
    return createNew;
}
