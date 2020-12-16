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

#include "viewers/viewcontrol.h"
#include "base/border.h"
#include "base/layer.h"
#include "base/mosaic.h"
#include "base/tpmsplash.h"
#include "base/utilities.h"
#include "designs/design.h"
#include "designs/design_maker.h"
#include "designs/patterns.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"
#include "style/style.h"
#include "tile/grid.h"
#include "tile/backgroundimage.h"
#include "viewers/view.h"
#include "viewers/faceset_view.h"
#include "viewers/figure_view.h"
#include "viewers/placed_designelement_view.h"
#include "viewers/prototype_view.h"
#include "viewers/tiling_view.h"

ViewControl * ViewControl::mpThis = nullptr;

ViewControl * ViewControl::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new ViewControl();
    }
    return mpThis;
}

void ViewControl::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

ViewControl::ViewControl()
{
    protoViewMode = PROTO_DRAW_FIGURES | PROTO_DRAW_FEATURES;
}

void ViewControl::init()
{
    config          = Configuration::getInstance();
    panel           = ControlPanel::getInstance();
    designMaker     = DesignMaker::getInstance();
    decorationMaker = DecorationMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    view            = View::getInstance();

    disableAllViews();
    viewEnable(config->viewerType,true);
}

ViewControl::~ViewControl()
{
    resetAllMakers();
}

void ViewControl::slot_clearView()
{
    // remove from scene but does not delete
    view->clearView();
}

void ViewControl::slot_clearMakers()
{
    view->dump(true);

    resetAllMakers();

    view->dump(true);
}

void ViewControl::selectFeature(WeakFeaturePtr wfp)
{
    selectedFeature = wfp;
}

FeaturePtr ViewControl::getSelectedFeature()
{
    return selectedFeature.lock();
}

// figures
void  ViewControl::setProtoViewMode(eProtoViewMode mode, bool enb)
{
    if (enb)
       protoViewMode |= mode;
    else
       protoViewMode &= ~mode;
}

void ViewControl::resetAllMakers()
{
    DesignMaker * designMaker = DesignMaker::getInstance();
    designMaker->clearDesigns();

    DecorationMaker * decorationMaker = DecorationMaker::getInstance();
    decorationMaker->resetMosaic();

    MotifMaker * motifMaker = MotifMaker::getInstance();
    motifMaker->erasePrototypes();

    TilingMaker * tilingMaker = TilingMaker::getInstance();
    tilingMaker->unload();

    selectedFeature.reset();

    view->clearView();
    view->reInitFrameSettings();
}

void ViewControl::viewEnable(eViewType view, bool enable)
{
    enabledViews[view] = enable;
}

void ViewControl::disableAllViews()
{
    for (int i=0; i <= VIEW_MAX; i++)
    {
        enabledViews[i] = false;
    }
}

void ViewControl::slot_refreshView()
{
    // prevent re-entrant call from the same thread

    static volatile bool busy   = false;
    static volatile bool review = false;

    if (busy)
    {
        qDebug() << "+ ViewController::slot_refreshView - BUSY IGNORED" ;
        review = true;
    }
    else
    {
        busy = true;
        refreshView();
        busy = false;

        if (review)
        {
            review = false;
            qDebug() << "RE-VIEW";
            slot_refreshView();
        }
    }
}

void ViewControl::refreshView()
{
    qDebug().noquote() << "+ ViewController::refreshView type=" << sViewerType[config->viewerType];

    view->paintEnable(false);

    view->clearView();

    // add the viewers
    setupViewers();

    // grid
    if (config->showGrid)
    {
        GridPtr grid  = make_shared<Grid>();
        view->addLayer(grid);
    }

    // resize
    view->resize(view->getActiveFrameSize(config->viewerType));

    // big blue cross
    if (config->circleX)
    {
        MarkXPtr item = make_shared<MarkX>(view->rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        view->addLayer(item);
    }

    // trigger repaint

    view->paintEnable(true);

    view->update();

    emit sig_viewUpdated();
}

void ViewControl::setupViewers()
{
    eViewType vtype = config->viewerType;
    panel->reflectCurrentView(vtype);

    for (int i=0; i <= VIEW_MAX; i++)
    {
        if (enabledViews[i])
        {
            switch (i)
            {
            case VIEW_DESIGN:
                viewDesign();
                break;

            case VIEW_MOSAIC:
                viewMosaic();
                break;

            case VIEW_PROTOTYPE:
                viewPrototype();
                break;

            case VIEW_DESIGN_ELEMENT:
                viewDesignElement();
                break;

            case VIEW_MOTIF_MAKER:
                viewMotifMaker();
                break;

            case VIEW_TILING:
                viewTiling();
                break;

            case VIEW_TILING_MAKER:
                viewTilingMaker();
                break;

            case VIEW_MAP_EDITOR:
                viewMapEditor();
                break;

            case VIEW_FACE_SET:
                viewFaceSet();
                break;

            case VIEW_UNDEFINED:
                break;
            }
        }
    }
}

void ViewControl::viewDesign()
{
    QVector<DesignPtr> &  designs = designMaker->getDesigns();
    for (auto design :  designs)
    {
        QVector<PatternPtr> & pats = design->getPatterns();
        for (auto& pat : pats)
        {
            view->addLayer(pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        ModelSettingsPtr settings = dp->getDesignInfo();
        view->setBackgroundColor(settings->getBackgroundColor());
        setBorder(settings->getBorder());
        setBackgroundImg(settings->getBkgdImage());
    }
    view->setWindowTitle(designMaker->getDesignName());
}

void ViewControl::viewMosaic()
{
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic)
    {
        QString name = mosaic->getName();
        qDebug() << "ViewController::viewMosaic" << name;

        ControlPanel * panel  = ControlPanel::getInstance();
        QString astring = QString("Preparing design: %1").arg(name);
        panel->showPanelStatus(astring);

#ifdef TPMSPLASH
        panel->showSplash(astring);
#endif
        const StyleSet & sset = mosaic->getStyleSet();
        for (auto style : sset)
        {
            qDebug().noquote() << "Adding Style:" << style.get() << "  " << style->getDescription();
            style->createStyleRepresentation();   // important to do this here
            view->addLayer(style);
        }

        panel->hidePanelStatus();
#ifdef TPMSPLASH
        panel->hideSplash();
#endif
        view->setWindowTitle(name);
    }
    else
    {
        qDebug() << "ViewController::viewMosaic - no mosaic";
        view->setWindowTitle("");
    }

    ModelSettingsPtr settings = decorationMaker->getMosaicSettings();
    view->setBackgroundColor(settings->getBackgroundColor());
    setBorder(settings->getBorder());
    setBackgroundImg(settings->getBkgdImage());
}

void ViewControl::viewPrototype()
{
    qDebug() << "++ViewController::viewPrototype";

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    if (!pp)
    {
        qWarning() << "viewProtoFeature: no selected prototype";
        return;
    }

    view->setWindowTitle("WS Proto Feature");

    LayerPtr pfview = make_shared<PrototypeView>(pp,getProtoViewMode());

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        const Xform & xf = style->getCanvasXform();
        pfview->setCanvasXform(xf);
    }
    else
    {
        TilingPtr tiling = pp->getTiling();
        const Xform & xf = tiling->getCanvasXform();
        pfview->setCanvasXform(xf);

        setTitle(tiling);
    }

    view->addLayer(pfview);

    view->setBackgroundColor(Qt::white);

    ModelSettingsPtr settings = ViewControl::getMosaicOrTilingSettings();
    setBackgroundImg(settings->getBkgdImage());
}

void ViewControl::viewDesignElement()
{
    qDebug() << "++ViewController::viewDesignElement";

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    if (!pp)
    {
        qWarning("viewDesignElement - no seleected protype");
        return;
    }

    StylePtr style;
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        style = mosaic->getFirstStyle();
        Q_ASSERT(style);
    }

    TilingPtr tiling                         = pp->getTiling();
    const QVector<PlacedFeaturePtr> & placed = tiling->getPlacedFeatures();
    QVector<DesignElementPtr> &  dels        = pp->getDesignElements();
    QVector<QTransform>       & tforms       = pp->getLocations();

    qDebug() << "dels=" << dels.size() << "tforms="  << tforms.size();
    for (auto delp : dels)
    {
        FeaturePtr  feature = delp->getFeature();
        bool selected = (feature == getSelectedFeature());
        for (auto pfp : placed)
        {
            if (feature == pfp->getFeature())
            {
                QTransform tr                  = pfp->getTransform();
                PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                LayerPtr delView               = make_shared<PlacedDesignElementView>(pdel,selected);
                if (style)
                {
                    // use settings from design
                    const Xform & xf = style->getCanvasXform();
                    delView->setCanvasXform(xf);
                }
                else
                {
                    // use settings from prototypes tiling
                    const Xform & xf = tiling->getCanvasXform();
                    delView->setCanvasXform(xf);
                }

                if (selected)
                {
                    view->addLayer(delView);
                }
                else
                {
                    view->addTopLayer(delView);
                }
            }
        }
    }

    QString astring = QString("Design elements for tiling: %1").arg(tiling->getName());
    view->setWindowTitle(astring);

    view->setBackgroundColor(Qt::white);

    ModelSettingsPtr settings = ViewControl::getMosaicOrTilingSettings();
    setBackgroundImg(settings->getBkgdImage());
}

void ViewControl::viewMotifMaker()
{
    DesignElementPtr dep = motifMaker->getSelectedDesignElement();
    if (!dep)
    {
        return;
    }

    qDebug() << "++ViewController::viewFigure";

    LayerPtr figView = make_shared<FigureView>(dep);
    view->addLayer(figView);

    QString astring = QString("WS Figure: dep=%1 fig=%2 tiling=%3").arg(Utils::addr(dep.get()),Utils::addr(dep->getFigure().get()),motifMaker->getSelectedPrototype()->getTiling()->getName());
    view->setWindowTitle(astring);

    QSize sz = view->getActiveFrameSize(VIEW_MOTIF_MAKER);
    figView->setCenterScreen(QRect(QPoint(0,0),sz).center());

    view->setBackgroundColor(config->figureViewBkgdColor);
#if 1
    ModelSettingsPtr settings = ViewControl::getMosaicOrTilingSettings();
    setBackgroundImg(settings->getBkgdImage());
#endif
}

void ViewControl::viewTiling()
{
    TilingMakerPtr tilingMaker = TilingMaker::getSharedInstance();
    TilingPtr tiling           = tilingMaker->getSelected();

    qDebug() << "++ViewController::viewTiling (tiling)"  << tiling->getName();

    TilingViewPtr tilingView = make_shared<TilingView>(tiling);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic  && mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        StylePtr sp = mosaic->getFirstStyle();
        const Xform & xf = sp->getCanvasXform();
        tilingView->setCanvasXform(xf);
    }
    else
    {
        const Xform & xf = tiling->getCanvasXform();
        tilingView->setCanvasXform(xf);
    }
    view->addLayer(tilingView);

    view->setBackgroundColor(Qt::white);
    setTitle(tiling);

    QSize sz = view->getActiveFrameSize(VIEW_TILING);
    tilingView->setCenterScreen(QRect(QPoint(0,0),sz).center());

    ModelSettingsPtr settings = ViewControl::getMosaicOrTilingSettings();
    setBackgroundImg(settings->getBkgdImage());
}

void ViewControl::viewTilingMaker()
{
    TilingMakerPtr tilingMaker = TilingMaker::getSharedInstance();
    TilingPtr tiling           = tilingMaker->getSelected();
    qDebug() << "++ViewController::viewTilingMaker";

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic  && mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        const Xform & xf = style->getCanvasXform();
        tilingMaker->setCanvasXform(xf);
    }
    else if (tiling)
    {
        const Xform & xf = tiling->getCanvasXform();
        tilingMaker->setCanvasXform(xf);
    }

    view->addLayer(tilingMaker);  // since this is a shared pointer it does not get deleted

    view->setBackgroundColor(Qt::white);

    QSize sz = view->getActiveFrameSize(VIEW_TILING_MAKER);
    tilingMaker->setCenterScreen(QRectF(QPoint(0,0),sz).center());

    ModelSettingsPtr settings = ViewControl::getMosaicOrTilingSettings();
    setBackgroundImg(settings->getBkgdImage());
}

void ViewControl::viewMapEditor()
{
    qDebug() << "++ViewController::viewFigMapEditor";

    MapEditorPtr ed  = MapEditor::getSharedInstance();

    view->addLayer(ed);           // since this is a shared pointer it does not get deleted

    ed->forceLayerRecalc();

    view->setWindowTitle("Map Editor");

    view->setBackgroundColor(config->figureViewBkgdColor);

    QSize sz = view->getActiveFrameSize(VIEW_MAP_EDITOR);
    ed->setCenterScreen(QRect(QPoint(0,0),sz).center());
}

void ViewControl::viewFaceSet()
{
    qDebug() << "++ViewController::viewFaceSet";
    LayerPtr fsView = make_shared<FaceSetView>(config->faces);
    view->addLayer(fsView);

    view->setBackgroundColor(Qt::black);
}

void ViewControl::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->viewerType],tp->getName(),tp->getDescription(),tp->getAuthor());
    view->setWindowTitle(str);
}

void  ViewControl::setBackgroundImg(BkgdImgPtr bkgd)
{
    if (bkgd)
    {
        if (bkgd->isLoaded() && bkgd->bShowBkgd)
        {
            view->addLayer(bkgd);
        }
    }
}

void  ViewControl::setBorder(BorderPtr bp)
{
    if (bp)
    {
        view->addLayer(bp);
    }
}

ModelSettingsPtr ViewControl::getMosaicOrTilingSettings()
{
    ModelSettingsPtr settings;
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        settings = mosaic->getSettings();
    }
    else
    {
        TilingPtr tp = tilingMaker->getSelected();
        Q_ASSERT(tp);
        settings = tp->getSettings();
    }
    return settings;
}


