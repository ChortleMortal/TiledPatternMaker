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

#include "figure_maker.h"
#include "base/canvas.h"
#include "base/workspace.h"
#include "master_figure_editor.h"
#include "FeatureButton.h"
#include "FeatureLauncher.h"
#include "style/Interlace.h"
#include "style/Plain.h"
#include "tapp/Infer.h"
#include "viewers/workspaceviewer.h"
#include "base/utilities.h"
#include "base/tiledpatternmaker.h"

FigureMaker::FigureMaker(TiledPatternMaker * maker)
{
    this->maker = maker;
    launcher    = new FeatureLauncher();
    viewerBtn   = make_shared<FeatureButton>(-1);
    masterEdit  = new MasterFigureEditor(this);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addLayout(launcher);
    hbox->addWidget(viewerBtn.get());

    addLayout(hbox);
    addWidget(masterEdit);

    Canvas          * canvas = Canvas::getInstance();
    WorkspaceViewer * viewer = WorkspaceViewer::getInstance();

    setObjectName("FigureMaker");
    connect(this,     &FigureMaker::sig_viewWS,            viewer, &WorkspaceViewer::slot_viewWorkspace);
    connect(this,     &FigureMaker::sig_render,            maker,  &TiledPatternMaker::slot_render);
    connect(launcher, &FeatureLauncher::sig_launcherButton, this,   &FigureMaker::slot_launcherButton);
    connect(canvas,   &Canvas::sig_figure_changed,          this,   &FigureMaker::slot_launcherButton);
}

void FigureMaker::slot_launcherButton()
{
    qDebug() << "slot_launcherButton";
    setActiveFeature(launcher->getCurrentButton());
}

void FigureMaker::slot_figureChanged()
{
    FigurePtr fp = masterEdit->getFigureFromEditor();
    if (!fp)
    {
        qDebug() << "FigureMaker::slot_figureChanged figure is null - returning";
        return;
    }
    masterEdit->setMasterFigure(fp);

    qDebug() << "FigureMaker::slot_figureChanged fig=" << fp.get();
    DesignElementPtr dep = viewerBtn->getDesignElement();
    dep->setFigure(fp);
    viewerBtn->designElementChanged();
    launcher->getCurrentButton()->designElementChanged();
    Workspace * workspace = Workspace::getInstance();
    workspace->setWSDesignElement(dep);
    emit sig_viewWS();
}

// called from page_figure_editor
void FigureMaker::setNewTiling(TilingPtr tiling)
{
    this->tiling = tiling;

    designPrototype = createPrototype(); //make a new prototype

    masterEdit->reset();

    FeatureBtnPtr btn = launcher->launchFromTiling(tiling, designPrototype);
    launcher->setCurrentButton(btn);

    Workspace * workspace = Workspace::getInstance();
    workspace->setWSPrototype(designPrototype);
}

// called from page_figure_editor
void FigureMaker::setNewStyle(StylePtr style)
{
    designPrototype.reset();
    tiling.reset();

    if (style)
    {
        designPrototype =  style->getPrototype();

        tiling = designPrototype->getTiling();
    }

    masterEdit->reset();

    FeatureBtnPtr btn = launcher->launchFromStyle(style);
    launcher->setCurrentButton(btn);

    Workspace * workspace = Workspace::getInstance();
    workspace->setWSPrototype(designPrototype);
}

// called from page_figure_editor when tiling has changed
void FigureMaker::setTilingChanged()
{
    designPrototype = createPrototype();

    masterEdit->reset();

    FeatureBtnPtr btn = launcher->launchFromTiling(tiling, designPrototype);
    launcher->setCurrentButton(btn);

    Workspace * workspace = Workspace::getInstance();
    workspace->setWSPrototype(designPrototype);
}


void FigureMaker::setActiveFeature(FeatureBtnPtr fb)
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
    }
}

FeaturePtr FigureMaker::getActiveFeature()
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

QPolygonF FigureMaker::boundingRect()
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

StylePtr  FigureMaker::createDefaultStyleFromPrototype()
{
    PrototypePtr pp = getPrototype();
    populateProtoWithDELs(pp);
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


PrototypePtr FigureMaker::makePrototype()
{
    designPrototype = createPrototype();
    return designPrototype;
}

PrototypePtr FigureMaker::getPrototype()
{
    if (!designPrototype)
    {
        designPrototype = createPrototype();
    }
    return designPrototype;
}

PrototypePtr FigureMaker::createPrototype()
{
    // this is the taprats way
    qDebug() << "FigureMaker::createPrototype()";
    PrototypePtr proto;
    if (tiling)
    {
        proto = make_shared<Prototype>(tiling);
        populateProtoWithDELs(proto);
    }
    return proto;
}

void FigureMaker::populateProtoWithDELs(PrototypePtr proto)
{
    proto->getDesignElements().clear();
    for(auto it = launcher->buttons.begin(); it != launcher->buttons.end(); it++)
    {
        FeatureBtnPtr     fb = *it;
        DesignElementPtr dep = fb->getDesignElement();
        FigurePtr        fp  = dep->getFigure();
        Q_ASSERT(fp);
        proto->addElement(dep);
    }
}

bool FigureMaker::verify()
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

    for (auto it = launcher->buttons.begin(); it != launcher->buttons.end(); it++)
    {
        FeatureBtnPtr     fb = *it;
        DesignElementPtr del = fb->getDesignElement();
        FigurePtr        fp  = del->getFigure();
        qDebug() << "FigureMaker::verify figure =" << fp.get();
        if (!fp) rv = false;
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

MapPtr FigureMaker::createExplicitInferredMap()
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

MapPtr FigureMaker::createExplicitFeatureMap()
{
    PrototypePtr proto = getPrototype();
    InferPtr inf = make_shared<Infer>(proto);
    return inf->inferFeature(getActiveFeature());
}

MapPtr FigureMaker::createExplicitStarMap( qreal d, int s )
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

MapPtr FigureMaker::createExplicitHourglassMap( qreal d, int s )
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

MapPtr FigureMaker::createExplicitGirihMap( int starSides, qreal starSkip )
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

MapPtr FigureMaker::createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive )
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

MapPtr FigureMaker::createExplicitRosetteMap( qreal q, int s, qreal r )
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

void FigureMaker::unload()
{
    setNewTiling(nullptr);
    setNewStyle(nullptr);
    masterEdit->MasterResetWithFigure(nullptr,nullptr);
    viewerBtn->setDesignElement(nullptr);
}
