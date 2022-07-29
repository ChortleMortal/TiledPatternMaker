#include <QMessageBox>

#include "makers/motif_maker/motif_maker.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "figures/explicit_figure.h"
#include "figures/infer.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "panels/page_motif_maker.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

typedef std::shared_ptr<class ExplicitFigure>   ExplicitPtr;
typedef std::shared_ptr<Infer>                  InferPtr;

MotifMaker * MotifMaker::mpThis = nullptr;

#define E2STR(x) #x

static QString mm_states[]
{
    E2STR(MM_EMPTY),
    E2STR(MM_SINGLE),
    E2STR(MM_MULTI)
 };

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
}

void MotifMaker::init()
{
    config          = Configuration::getInstance();
    vcontrol        = ViewControl::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    mosaicMaker     = MosaicMaker::getInstance();
}

void MotifMaker::unload()
{
    _activeFeature.reset();
    _selectedPrototype.reset();
    _selectedDesignElement.reset();
    _prototypes.clear();
    _selectedDesignElements.clear();
}

PrototypePtr MotifMaker::createPrototype(const TilingPtr &tiling)
{
    PrototypePtr prototype = make_shared<Prototype>(tiling);

    QVector<FeaturePtr> uniqueFeatures = tiling->getUniqueFeatures();
    qDebug() << "Create new design elements";
    int count = 0;
    for (auto feature : uniqueFeatures)
    {
        // NOTE this takes order from order of unique features
        DesignElementPtr dep = make_shared<DesignElement>(feature);
        prototype->addElement(dep);
        if (++count > MAX_UNIQUE_FEATURE_INDEX)
            qWarning() << "Large number of unique features in tiling count:" << count;
    }
    return prototype;
}

void MotifMaker::recreatePrototype(const TilingPtr &tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    QVector<FeaturePtr> uniqueFeatures = tiling->getUniqueFeatures();
    int count = 0;
    for (auto feature : uniqueFeatures)
    {
        if (++count > MAX_UNIQUE_FEATURE_INDEX)
            qWarning() << "Large number of unique features in tiling count:" << count;

        bool found = false;
        for (auto del : qAsConst(prototype->getDesignElements()))
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

    QVector<DesignElementPtr> forDeletion;
    for (auto del : qAsConst(prototype->getDesignElements()))
    {
        FeaturePtr feature = del->getFeature();
        if (!uniqueFeatures.contains(feature))
        {
            forDeletion.push_back(del);
        }
    }
    for (const auto & del : forDeletion)
    {
        prototype->removeElement(del);
    }

    prototype->wipeoutProtoMap();
}

void MotifMaker::recreateFigures(const TilingPtr &tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    for (auto del : qAsConst(prototype->getDesignElements()))
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
                }
                //figure->buildExtBoundary();
                //figure->buildMaps();
                figure->resetMaps();
            }
            else
            {
                ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figure);
                Q_ASSERT(ep);
                if (feature->numSides() != ep->getN())
                {
                    ep->setN(feature->numSides());
                }
                //ep->buildExtBoundary();
                //ep->buildMaps();
                figure->resetMaps();
            }
        }
        else
        {
            del->createFigure();
            figure  = del->getFigure();
            //figure->buildExtBoundary();
            //figure->buildMaps();
            figure->resetMaps();
        }
    }
}

void MotifMaker::takeDown(const PrototypePtr & prototype)
{
    qDebug() << "MotifMaker::takeDown()";

    _prototypes.push_front(prototype);

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

void MotifMaker::sm_eraseAllCreateAdd(const TilingPtr &tiling)
{
    erasePrototypes();
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_eraseCurrentCreateAdd(const TilingPtr &tiling)
{
    _prototypes.removeOne(getSelectedPrototype());
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_createAdd(const TilingPtr &tiling)
{
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_replaceTiling(const PrototypePtr &prototype, const TilingPtr &tiling)
{
    prototype->setTiling(tiling);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_resetMaps()
{
    for (auto prototype : qAsConst(_prototypes))
    {
        prototype->wipeoutProtoMap();
    }
}

void  MotifMaker::sm_take(const TilingPtr & tiling, eSM_Event event)
{
    eMMState state = sm_getState();
    qDebug().noquote() << "MotifMaker::sm_take() state:" << mm_states[state] << "event:" << sSM_Events[event];

    QVector<PrototypePtr> protos;
    switch (event)
    {
    case SM_LOAD_EMPTY:
        sm_eraseAllCreateAdd(tiling);
        protos.push_back(getSelectedPrototype());
        mosaicMaker->sm_takeUp(protos,event);
        emit sig_tilingChoicesChanged();
        break;

    case SM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            sm_eraseAllCreateAdd(tiling);
            //mosaicMaker->sm_takeUp(selectedPrototype,event);
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
                     mosaicMaker->sm_takeUp(selectedPrototype,event);
                 }
                 else
                 {
                     sm_replaceTiling(selectedPrototype, tiling);
                     mosaicMaker->sm_takeUp(selectedPrototype,SM_RELOAD_SINGLE);
                 }
#else
                 sm_eraseAllCreateAdd(tiling);
                 protos.push_back(getSelectedPrototype());
                 mosaicMaker->sm_takeUp(protos,event);
#endif
             }
             else if (state == MM_MULTI)
             {
                 bool createNew = askNewProto();
                 if (createNew)
                 {
                     sm_eraseCurrentCreateAdd(tiling);
                     protos.push_back(getSelectedPrototype());
                     mosaicMaker->sm_takeUp(protos,SM_LOAD_MULTI);
                 }
                 else
                 {
                     auto proto = getSelectedPrototype();
                     sm_replaceTiling(proto, tiling);
                     protos.push_back(proto);
                     mosaicMaker->sm_takeUp(protos,SM_RELOAD_MULTI);
                 }
             }
        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,event);
        }
        else
        {
            Q_ASSERT(state == MM_EMPTY);
            qWarning("MotifMaker ; invalid state");
        }
        emit sig_tilingChoicesChanged();
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
            mosaicMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            sm_createAdd(tiling);
            protos.push_back(getSelectedPrototype());
            mosaicMaker->sm_takeUp(protos,event);

        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_RELOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == MM_SINGLE)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,event);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,event);
        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_FIGURE_CHANGED:
        sm_resetMaps();
        mosaicMaker->sm_takeUp(protos,event);
        break;

    case SM_FEATURE_CHANGED:
        Q_ASSERT(tiling == getSelectedPrototype()->getTiling());
        recreateFigures(tiling);
        mosaicMaker->sm_takeUp(protos,event);
        emit sig_featureChanged();
        break;

    case SM_TILING_CHANGED:
        if (state == MM_EMPTY)
        {
            sm_eraseAllCreateAdd(tiling);
        }
        else
        {
            recreatePrototype(tiling);
            emit sig_tilingChanged();
            mosaicMaker->sm_takeUp(protos,event);
        }
        break;

    case SM_RENDER:
        sm_resetMaps();
        for (auto& prototype : _prototypes )
        {
            protos.push_back(prototype);
        }
        mosaicMaker->sm_takeUp(protos,event);
        break;

    case SM_LOAD_FROM_MOSAIC:
        qWarning("Invalid mode");
        break;
    }
    if (vcontrol->isEnabled(VIEW_MOTIF_MAKER))
        vcontrol->update();
}

const QVector<PrototypePtr> & MotifMaker::getPrototypes()
{
    return _prototypes;
}

void MotifMaker::removePrototype(TilingPtr tiling)
{
    QVector<PrototypePtr> forRemoval;

    for (auto prototype : qAsConst(_prototypes))
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval.push_back(prototype);
        }
    }

    for (auto& prototype : forRemoval)
    {
        _prototypes.removeAll(prototype);
        if (getSelectedPrototype() == prototype)
        {
            resetSelectedPrototype();
        }
    }
}

void MotifMaker::erasePrototypes()
{
    _prototypes.clear();
    resetSelectedPrototype();
}

eMMState MotifMaker::sm_getState()
{
    if (_prototypes.size() > 1)
    {
        return  MM_MULTI;
    }
    else if (_prototypes.size() == 0)
    {
        return MM_EMPTY;
    }
    else
    {
        Q_ASSERT(getSelectedPrototype());
        Q_ASSERT(_prototypes.size() == 1);
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
            getSelectedPrototype()->wipeoutProtoMap();
            return;
        }
    }
}

void MotifMaker::deleteActiveFeature()
{
    FeaturePtr fp = getActiveFeature();
    if (!fp)
        return;

    auto pp = getSelectedPrototype();
    if (!pp)
        return;

    QVector<PlacedFeaturePtr> deletions;
    TilingPtr tiling = pp->getTiling();
    const QVector<PlacedFeaturePtr> & pfps = tiling->getPlacedFeatures();
    for (auto pfp : pfps)
    {
        if (pfp->getFeature() == fp)
        {
            deletions.push_back(pfp);
        }
    }
    for (auto pfp : deletions)
    {
    	tiling->remove(pfp);
    }
    pp->removeElement(getSelectedDesignElement());
    pp->wipeoutProtoMap();
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

void MotifMaker::setSelectedPrototype(const PrototypePtr &pp)
{
    if (_selectedPrototype == pp)
    {
        return;
    }

    _selectedPrototype = pp;
    if (pp && pp->hasContent())
    {
        _selectedDesignElement = pp->getDesignElement(0);  // first
        _selectedDesignElements.push_back(_selectedDesignElement);
        _activeFeature         = _selectedDesignElement->getFeature();
    }
    else
        _selectedDesignElement.reset();
}

PrototypePtr MotifMaker::getSelectedPrototype()
{
    return _selectedPrototype;
}

void MotifMaker::setSelectedDesignElement(const DesignElementPtr &del)
{
    _selectedDesignElement = del;
    if (!config->motifMultiView)
    {
        _selectedDesignElements.clear();
    }
    _selectedDesignElements.push_back(del);
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

PrototypePtr MotifMaker::findPrototypeByName(const TilingPtr &tiling)
{
    PrototypePtr pp;
    for (auto prototype : qAsConst(_prototypes))
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
