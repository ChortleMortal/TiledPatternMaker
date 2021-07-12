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
#include "settings/model_settings.h"
#include "style/style.h"
#include "tapp/prototype.h"
#include "tile/backgroundimage.h"
#include "viewers/figure_view.h"
#include "viewers/grid.h"
#include "viewers/grid.h"
#include "viewers/prototype_view.h"
#include "viewers/tiling_view.h"
#include "viewers/view.h"
#include "base/transparentwidget.h"

typedef std::shared_ptr<Grid>            GridPtr;
typedef std::shared_ptr<MarkX>           MarkXPtr;

using std::make_shared;


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
}

void ViewControl::init()
{
    config          = Configuration::getInstance();
    panel           = ControlPanel::getInstance();
    designMaker     = DesignMaker::getInstance();
    decorationMaker = DecorationMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    view            = View::getInstance();

    mapEditor       = MapEditor::getSharedInstance();
    pfview          = PrototypeView::getSharedInstance();
    figView         = FigureView::getSharedInstance();
    tilingView      = TilingView::getSharedInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    gridView        = Grid::getSharedInstance();
    imageView       = BackgroundImage::getSharedInstance();

    disableAllViews();
    viewEnable(config->getViewerType(),true);
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

void ViewControl::resetAllMakers()
{
    DesignMaker * designMaker = DesignMaker::getInstance();
    designMaker->clearDesigns();

    DecorationMaker * decorationMaker = DecorationMaker::getInstance();
    decorationMaker->resetMosaic();

    MotifMaker * motifMaker = MotifMaker::getInstance();
    motifMaker->erasePrototypes();

    mapEditor->unload();
    tilingMaker->unload();

    selectedFeature.reset();

    view->clearView();
    view->frameSettings.reInit();
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

void ViewControl::slot_updateView()
{
    view->update();
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
    qDebug().noquote() << "+ ViewController::refreshView type=" << sViewerType[config->getViewerType()];

    view->paintEnable(false);

    view->clearView();

    // viewers
    setupViewers();

    // background image
    BkgdImgPtr bip = BackgroundImage::getSharedInstance();
    if (imageView->isLoaded())
    {
        qDebug() << "adding image" << imageView->getName();
        view->addLayer(imageView);
    }

    // other images
    for (ImgLayerPtr ilp : images)
    {
        view->addLayer(ilp);
    }

    // grid
    if (config->showGrid)
    {
        Grid::getSharedInstance()->create();    // re-create
        view->addLayer(gridView);
    }

    // resize
    QSize size = view->frameSettings.getCropSize(config->getViewerType());
    if (size != view->size())
    {
        view->resize(size);
    }

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
    eViewType vtype = config->getViewerType();
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
        for (PatternPtr & pat : pats)
        {
            view->addLayer(pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        ModelSettingsPtr settings = dp->getDesignInfo();
        view->setBackgroundColor(settings->getBackgroundColor());
        setBorder(dp->border);
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

        QString astring = QString("Preparing design: %1").arg(name);
        panel->showPanelStatus(astring);

#ifdef TPMSPLASH
        panel->splashMosiac(astring);
#endif

        const StyleSet & sset = mosaic->getStyleSet();
        for (StylePtr style : sset)
        {
            qDebug().noquote() << "Adding Style:" << style.get() << "  " << style->getDescription();
            if (view->frameSettings.getModelAlignment() != M_ALIGN_MOSAIC)
            {
                style->setCanvasXform(getCurrentXform());
            }
            style->createStyleRepresentation();   // important to do this here
            view->addLayer(style);
        }
        view->setWindowTitle(name);

        ModelSettingsPtr settings = decorationMaker->getMosaicSettings();
        if (settings)
        {
            view->setBackgroundColor(settings->getBackgroundColor());
        }
        setBorder(mosaic->getBorder());

        panel->hidePanelStatus();
#ifdef TPMSPLASH
        panel->removeSplashMosaic();
#endif
    }
    else
    {
        qDebug() << "ViewController::viewMosaic - no mosaic";
        view->setWindowTitle("");
    }
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

    pfview->setPrototype(pp);
    pfview->setCanvasXform(getCurrentXform());
    view->addLayer(pfview);

    view->setBackgroundColor(Qt::white);
}

void ViewControl::viewMotifMaker()
{
    qDebug() << "++ViewController::viewFigure";

    // dont set canvas xform
    view->addLayer(figView);

    QString astring = QString("Motif Maker Figure");
    view->setWindowTitle(astring);

    QSize sz = view->frameSettings.getCropSize(VIEW_MOTIF_MAKER);
    figView->setCenterScreenUnits(QRect(QPoint(0,0),sz).center());

    view->setBackgroundColor(config->figureViewBkgdColor);
}

void ViewControl::viewTiling()
{
    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
        qDebug() << "++ViewController::viewTiling (tiling)"  << tiling->getName();

    tilingView->setTiling(tiling);
    view->addLayer(tilingView);

    view->setBackgroundColor(Qt::white);
    setTitle(tiling);

    QSize sz = view->frameSettings.getCropSize(VIEW_TILING);
    tilingView->setCenterScreenUnits(QRect(QPoint(0,0),sz).center());
}

void ViewControl::viewTilingMaker()
{
    qDebug() << "++ViewController::viewTilingMaker";

    // dont set canvas xform
    view->addLayer(tilingMaker);

    view->setBackgroundColor(Qt::white);

    QSize sz = view->frameSettings.getCropSize(VIEW_TILING_MAKER);
    tilingMaker->setCenterScreenUnits(QRectF(QPoint(0,0),sz).center());
}

void ViewControl::viewMapEditor()
{
    qDebug() << "++ViewController::viewMapEditor";

    mapEditor->setCanvasXform(getCurrentXform());
    view->addLayer(mapEditor);

    mapEditor->forceLayerRecalc();

    view->setWindowTitle("Map Editor");

    view->setBackgroundColor(config->figureViewBkgdColor);

    QSize sz = view->frameSettings.getCropSize(VIEW_MAP_EDITOR);
    mapEditor->setCenterScreenUnits(QRect(QPoint(0,0),sz).center());
}

void ViewControl::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->getViewerType()]).arg(tp->getName()).arg(tp->getDescription()).arg(tp->getAuthor());
    view->setWindowTitle(str);
}

void  ViewControl::setBorder(BorderPtr bp)
{
    if (bp)
    {
        OuterBorder * ob = dynamic_cast<OuterBorder*>(bp.get());
        if (ob)
        {
            view->addLayer(bp);
        }
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
        if (tp)
        {
            settings = tp->getSettings();
        }
    }
    return settings;
}

const Xform &ViewControl::getCurrentXform()
{
    switch (view->frameSettings.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
    {
        MosaicPtr mosaic = decorationMaker->getMosaic();
        if (mosaic)
        {
            StylePtr style = mosaic->getFirstStyle();
            if (style)
            {
                return style->getCanvasXform();
            }
        }
        return unityXform;
    }
    case M_ALIGN_TILING:
    {
        PrototypePtr pp = motifMaker->getSelectedPrototype();
        if (pp)
        {
            TilingPtr tiling = pp->getTiling();
            if (tiling)
            {
                return tiling->getCanvasXform();
            }
        }
        return unityXform;
    }
    default:
    case M_ALIGN_NONE:
        return unityXform;
    }
}

void  ViewControl::removeImage(ImageLayer * img)
{
    for (ImgLayerPtr ilp : images)
    {
        ImageLayer * il = ilp.get();
        if (il == img)
        {
            images.removeAll(ilp);
            return;
        }
    }
}
