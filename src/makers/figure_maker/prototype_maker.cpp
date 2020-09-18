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
#include "base/workspace.h"
#include "makers/figure_maker/prototype_maker.h"
#include "makers/figure_maker/master_figure_editor.h"
#include "makers/figure_maker/feature_button.h"
#include "makers/figure_maker/feature_launcher.h"
#include "style/interlace.h"
#include "style/plain.h"
#include "tapp/infer.h"
#include "viewers/workspace_viewer.h"

PrototypeMaker * PrototypeMaker::mpThis = nullptr;

PrototypeMaker * PrototypeMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new PrototypeMaker;
    }
    return mpThis;
}

PrototypeMaker::PrototypeMaker()
{}

void PrototypeMaker::init(TiledPatternMaker * maker, page_prototype_maker * menu)
{
    this->maker = maker;
    this->menu  = menu;

    config    = Configuration::getInstance();
    workspace = Workspace::getInstance();

    launcher    = new FeatureLauncher();
    viewerBtn   = make_shared<FeatureButton>(-1);
    masterEdit  = new MasterFigureEditor(this);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addLayout(launcher);
    hbox->addWidget(viewerBtn.get());

    addLayout(hbox);
    addWidget(masterEdit);

    Workspace * workspace = Workspace::getInstance();

    setObjectName("FigureMaker");
    connect(this,      &PrototypeMaker::sig_viewWS,          workspace, &WorkspaceViewer::slot_viewWorkspace);
    connect(launcher,  &FeatureLauncher::sig_launcherButton, this,      &PrototypeMaker::slot_launcherButton);
    connect(workspace, &View::sig_figure_changed,            this,      &PrototypeMaker::slot_launcherButton);
}

void PrototypeMaker::slot_launcherButton()
{
    qDebug() << "slot_launcherButton";
    setActiveFeature(launcher->getCurrentButton());
}

void PrototypeMaker::slot_figureChanged()
{
    FigurePtr fp = masterEdit->getFigureFromEditor();
    if (!fp)
    {
        qWarning() << "FigureMaker::slot_figureChanged figure is null - returning";
        return;
    }
    masterEdit->setMasterFigure(fp);

    qDebug() << "FigureMaker::slot_figureChanged fig=" << fp.get();
    DesignElementPtr dep = viewerBtn->getDesignElement();
    dep->setFigure(fp);
    viewerBtn->designElementChanged();
    launcher->getCurrentButton()->designElementChanged();
    menu->setupFigure(fp->isRadial());

    workspace->setSelectedDesignElement(dep);

    emit sig_viewWS();
}

// called from page_figure_editor
void PrototypeMaker::setupFigures(PrototypePtr prototype)
{
    _prototype = prototype;

    masterEdit->reset();

    launcher->launch(prototype, prototype->getTiling());
}

void PrototypeMaker::setActiveFeature(FeatureBtnPtr fb)
{
    qDebug() << "FigureMaker::setActiveFeature" << Utils::addr(fb.get());
    //verify();

    if (fb)
    {
        DesignElementPtr dep = fb->getDesignElement(); // DAC taprats cloned here
        Q_ASSERT(dep);
        FigurePtr figure   = dep->getFigure();
        Q_ASSERT(figure);
        qDebug() << "Active feature: index=" << fb->getIndex() << "del=" << dep.get()  << "fig=" << figure.get() << " " << figure->getFigureDesc();
#if 0
        // Here is where I default the explicit type to infer
        if (figure->getFigType() == FIG_TYPE_EXPLICIT)
        {
            figure->setFigType(FIG_TYPE_INFER);
        }
#endif
        viewerBtn->setDesignElement(dep);
        masterEdit->MasterResetWithFigure(figure,fb);
        menu->setupFigure(figure->isRadial());
    }
}

FeaturePtr PrototypeMaker::getActiveFeature()
{
    FeaturePtr a;
    FeatureBtnPtr fb = launcher->getCurrentButton();
    if (!fb)
    {
        return a;
    }
    DesignElementPtr dep = fb->getDesignElement();
    if (!dep)
    {
        return a;
    }
    return dep->getFeature();
}


void PrototypeMaker::duplicateActiveFeature()
{
    FeaturePtr fp = getActiveFeature();
    if (!fp) return;

    // create new feature
    FeaturePtr fp2;
    if (fp->isRegular())
    {
        fp2 = make_shared<Feature>(fp->numPoints(),fp->getRotation());
    }
    else
    {
        fp2 = make_shared<Feature>(fp->getEdgePoly(),fp->getRotation());
    }

    TilingPtr tiling = workspace->getCurrentTiling();
    const QVector<PlacedFeaturePtr> & pfps = tiling->getPlacedFeatures();
    for (auto pfp : pfps)
    {
        if (pfp->getFeature() == fp)
        {
            PlacedFeaturePtr pfp2 = make_shared<PlacedFeature>(fp2,pfp->getTransform());
            // put placed feature in the tiling
            tiling->add(pfp2);
            if (_prototype)
            {
                DesignElementPtr dep = make_shared<DesignElement>(fp2);
                // put design element in the prototype
                _prototype->addElement(dep);
                _prototype->resetProtoMap();
            }
        }
    }
}


QPolygonF PrototypeMaker::boundingRect()
{
    QPolygonF p;
#if 1
    //p << QPointF(-50,-50) << QPointF(-50,50) << QPointF(50,50) << QPointF(50,-50);
    p  << QPointF(-10,-10) << QPointF(-10,10) << QPointF(10,10) << QPointF(10,-10);
    //p < QPointF(-20,-20) << QPointF(-20,20) << QPointF(20,20) << QPointF(20,-20);
#else
    QRectF boundary;
    for( int idx = 0; idx < launcher->numFeatureButtons(); ++idx )
    {
        FeatureButton * fb   = launcher->getFeatureButton(idx);
        DesignElementPtr del = fb->getDesignElement();
        FeaturePtr       fp  = del->getFeature();
        QRectF b0            = fp->boundingRect();
        boundary             = boundary.united(b0);    // assembles super-set
    }
    p = boundary;
#endif
    return p;
}

StylePtr  PrototypeMaker::createDefaultStyleFromPrototype()
{
    PrototypePtr pp = getPrototype();
    launcher->populateProtoWithDELsFromButtons(pp);

    //Plain * st      = new Plain(getPrototype(),make_shared<QPolygonF>(dummyBoundary));
    //Interlace * st  = new Interlace(getPrototype(),make_shared<QPolygonF>(boundingRect()));
    StylePtr thick    = make_shared<Thick>(pp,make_shared<QPolygonF>(boundingRect()));
    return thick;
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


PrototypePtr PrototypeMaker::getPrototype()
{
    return _prototype;
}

#if 0
// removed 26APR2020
PrototypePtr FigureMaker::createPrototypeFromLauncherButtons()
{
    // this is the taprats way
    qDebug() << "FigureMaker::createPrototype()";
    PrototypePtr proto;
    if (tiling)
    {
        proto = make_shared<Prototype>(tiling);
        populateProtoWithDELsFromLauncherButtons(proto);
        proto->createProtoMap();
    }
    return proto;
}
#endif



bool PrototypeMaker::verify()
{
    bool rv = true;
    DesignElementPtr dep = viewerBtn->getDesignElement();
    if (dep)
    {
        FigurePtr        fp = dep->getFigure();
        if (!fp) rv = false;
        qDebug() << "FigureMaker::verify feature=" << fp.get();
    }
    else
        qDebug() << "FigureMaker::verify -  No design element";

    if (!launcher->verify())
    {
        rv = false;
    }

    if (!rv)
        qCritical("FigureMaker::verify - FAIL");
    return rv;
}

////////////////////////////////////////////////////////////////////////////
//
// Explicit figure inferring.
//
// Infer a map for the currently selected feature, using
// the app.Infer algorithm.  Absolutely definitely
// guaranteed to not necessarily work or produce satisfactory
// results.

MapPtr PrototypeMaker::createExplicitInferredMap()
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
        return inf->infer( getActiveFeature() );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr PrototypeMaker::createExplicitFeatureMap()
{
    PrototypePtr proto = getPrototype();
    InferPtr inf = make_shared<Infer>(proto);
    return inf->inferFeature(getActiveFeature());
}

MapPtr PrototypeMaker::createExplicitStarMap( qreal d, int s )
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
        return inf->inferStar( getActiveFeature(), d, s );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr PrototypeMaker::createExplicitHourglassMap( qreal d, int s )
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
        return inf->inferHourglass( getActiveFeature(), d, s );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr PrototypeMaker::createExplicitGirihMap( int starSides, qreal starSkip )
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
        return inf->inferGirih( getActiveFeature(), starSides, starSkip );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

MapPtr PrototypeMaker::createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive )
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
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

MapPtr PrototypeMaker::createExplicitRosetteMap( qreal q, int s, qreal r )
{
    PrototypePtr proto = getPrototype();
    try
    {
        InferPtr inf = make_shared<Infer>(proto);
        return inf->inferRosette( getActiveFeature(), q, s, r );
    }
    catch(...)
    {
        qWarning("Woops - exception");
        return nullMap;
    }
}

void PrototypeMaker::unload()
{
    _prototype.reset();
    launcher->clear();
    masterEdit->reset();
    masterEdit->MasterResetWithFigure(nullptr,nullptr);
    viewerBtn->setDesignElement(nullptr);
}
