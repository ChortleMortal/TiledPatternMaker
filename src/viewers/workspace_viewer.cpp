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

#include "viewers/workspace_viewer.h"
#include "base/border.h"
#include "base/layer.h"
#include "base/tpmsplash.h"
#include "base/utilities.h"
#include "base/workspace.h"
#include "designs/patterns.h"
#include "panels/panel.h"
#include "style/style.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "viewers/prototype_view.h"
#include "viewers/placed_designelement_view.h"
#include "viewers/figure_view.h"
#include "viewers/tiling_view.h"
#include "viewers/faceset_view.h"

WorkspaceViewer::WorkspaceViewer()
{
}

void WorkspaceViewer::init()
{
    workspace       = Workspace::getInstance();
    config          = Configuration::getInstance();
    panel           = ControlPanel::getInstance();

    View::init();

    disableAll();
    viewEnable(config->viewerType,true);
}

void WorkspaceViewer::viewEnable(eViewType view, bool enable)
{
    enabledViews[view] = enable;
}

void WorkspaceViewer::disableAll()
{
    for (int i=0; i <= VIEW_MAX; i++)
    {
        enabledViews[i] = false;
    }
}

void WorkspaceViewer::slot_viewWorkspace()
{
    // prevent re-entrant call from the same thread

    static bool busy   = false;
    static bool review = false;

    if (busy)
    {
        qDebug() << "+ WorkspaceViewer::slot_viewWorkspace - BUSY IGNORED" ;
        review = true;
        return;
    }

    busy = true;
    viewWorkspace();
    busy = false;

    if (review)
    {
        review = false;
        qDebug() << "RE-VIEW";
        slot_viewWorkspace();
    }
}

void WorkspaceViewer::viewWorkspace()
{
    qDebug().noquote() << "+ WorkspaceViewer::slot_viewWorkspace type=" << sViewerType[config->viewerType];

    clearView();

    // add the viewers
    setupViewers();

    // resize
    resize(getActiveSize(config->viewerType));

    // big blue cross
    if (config->circleX)
    {
        MarkXPtr item = make_shared<MarkX>(rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        addLayer(item);
    }

    // trigger repaint
    update();

    emit sig_viewUpdated();
}

void WorkspaceViewer::setupViewers()
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

            case VIEW_FROTOTYPE_MAKER:
                viewPrototypeMaker();
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

void WorkspaceViewer::viewDesign()
{
    QVector<DesignPtr> &  designs = workspace->getDesigns();
    for (auto design :  designs)
    {
        QVector<PatternPtr> & pats = design->getPatterns();
        for (auto pat : pats)
        {
            addLayer(pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        WorkspaceSettings & settings = dp->getDesignInfo();
        setBackgroundColor(settings.getBackgroundColor());
        setBorder(settings.getBorder());
        setBackgroundImg(settings.getBkgdImage());
    }
    setWindowTitle(workspace->getDesignName());
}

void WorkspaceViewer::viewMosaic()
{
    MosaicPtr mosaic = workspace->getMosaic();
    QString name = mosaic->getName();
    qDebug() << "WorkspaceViewer::viewMosaic" << name;

    ControlPanel * panel  = ControlPanel::getInstance();
    QString astring = QString("Preparing design: %1").arg(name);
    panel->showPanelStatus(astring);

#ifdef TPMSPLASH
    panel->showSplash(astring);
#endif
    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        qDebug().noquote() << "Adding Style:" << Utils::addr(style.get()) << "  " << style->getDescription();
        style->createStyleRepresentation();   // important to do this here
        addLayer(style);
    }

    panel->hidePanelStatus();
#ifdef TPMSPLASH
    panel->hideSplash();
#endif

    WorkspaceSettings & settings = mosaic->getSettings();
    setBackgroundColor(settings.getBackgroundColor());
    setBorder(settings.getBorder());
    setBackgroundImg(settings.getBkgdImage());
    setWindowTitle(name);
}

void WorkspaceViewer::viewPrototype()
{
    qDebug() << "++WorkspaceViewer::viewPrototype";

    PrototypePtr pp = workspace->getSelectedPrototype();
    if (!pp)
    {
        qWarning() << "viewProtoFeature: no selected prototype";
        return;
    }

    setWindowTitle("WS Proto Feature");

    LayerPtr pfview = make_shared<PrototypeView>(pp,workspace->getProtoMode());

    WorkspaceSettings settings;
    Xform             xf;

    MosaicPtr mosaic = workspace->getMosaic();
    if (mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        xf = style->getCanvasXform();
        settings = mosaic->getSettings();
    }
    else
    {
        TilingPtr tiling = pp->getTiling();
        xf = tiling->getCanvasXform();

        settings = tiling->getSettings();

        setTitle(tiling);
    }

    addLayer(pfview);
    pfview->setCanvasXform(xf);

    setBackgroundColor(Qt::white);
    setBackgroundImg(settings.getBkgdImage());
}

void WorkspaceViewer::viewDesignElement()
{
    qDebug() << "++WorkspaceViewer::viewDesignElement";

    PrototypePtr pp = workspace->getSelectedPrototype();
    if (!pp)
    {
        qWarning("viewDesignElement - no seleected protype");
        return;
    }

    TilingPtr tiling = pp->getTiling();

    WorkspaceSettings settings;
    StylePtr style;
    MosaicPtr mosaic = workspace->getMosaic();
    if (mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        settings = mosaic->getSettings();
    }
    else
    {
        settings = tiling->getSettings();
    }

    const QVector<PlacedFeaturePtr> & placed = tiling->getPlacedFeatures();
    QVector<DesignElementPtr> &  dels      = pp->getDesignElements();
    QVector<QTransform>       & tforms     = pp->getLocations();

    qDebug() << "dels=" << dels.size() << "tforms="  << tforms.size();
    for (auto delp : dels)
    {
        FeaturePtr  feature = delp->getFeature();
        bool selected = (feature == workspace->getSelectedFeature());
        for (auto pfp : placed)
        {
            if (feature == pfp->getFeature())
            {
                QTransform tr                  = pfp->getTransform();
                PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                LayerPtr delView               = make_shared<PlacedDesignElementView>(pdel,selected);
                Xform xf;
                if (style)
                {
                    // use settings from design
                    Xform xf = style->getCanvasXform();
                }
                else
                {
                    // use settings from tiling maker
                    TilingPtr tiling = pp->getTiling();
                    xf = tiling->getCanvasXform();
                }
                delView->setCanvasXform(xf);

                if (selected)
                {
                    addLayer(delView);
                }
                else
                {
                    addTopLayer(delView);
                }
            }
        }
    }

    QString astring = QString("Design elements for tiling: %1").arg(tiling->getName());
    setWindowTitle(astring);

    setBackgroundImg(settings.getBkgdImage());
    setBackgroundColor(Qt::white);
}

void WorkspaceViewer::viewPrototypeMaker()
{
    // this is used by FigureMaker
    // this is what we want to see painted

    DesignElementPtr dep = workspace->getSelectedDesignElement();
    if (!dep)
    {
        return;
    }

    qDebug() << "++WorkspaceViewer::viewFigure";

    LayerPtr figView = make_shared<FigureView>(dep);
    addLayer(figView);

    QString astring = QString("WS Figure: dep=%1 fig=%2").arg(Utils::addr(dep.get())).arg(Utils::addr(dep->getFigure().get()));
    setWindowTitle(astring);

    QSize sz = getActiveSize(VIEW_FROTOTYPE_MAKER);
    setBackgroundColor(config->figureViewBkgdColor);
    figView->setCenterScreen(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewTiling()
{
    TilingPtr tiling = workspace->getCurrentTiling();

    qDebug() << "++WorkspaceViewer::viewTiling (tiling)"  << tiling->getName();

    TilingViewPtr tilingView = make_shared<TilingView>(tiling);


    WorkspaceSettings settings;
    Xform             xf;

    MosaicPtr mosaic = workspace->getMosaic();
    if (mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        StylePtr sp = mosaic->getFirstStyle();
        xf = sp->getCanvasXform();
        settings = mosaic->getSettings();
    }
    else
    {
        xf = tiling->getCanvasXform();
        settings = tiling->getSettings();
    }
    tilingView->setCanvasXform(xf);
    addLayer(tilingView);

    setBackgroundColor(Qt::white);
    setBackgroundImg(settings.getBkgdImage());
    setTitle(tiling);
}

void WorkspaceViewer::viewTilingMaker()
{
    TilingMakerPtr tilingMaker = TilingMaker::getSharedInstance();
    TilingPtr tiling           = tilingMaker->getTiling();
    qDebug() << "++WorkspaceViewer::viewTilingMaker";

    WorkspaceSettings settings;
    Xform             xf;

    MosaicPtr mosaic = workspace->getMosaic();
    if (mosaic->hasContent())
    {
        StylePtr style = mosaic->getFirstStyle();
        Q_ASSERT(style);
        xf = style->getCanvasXform();
        settings = mosaic->getSettings();
    }
    else if (tiling)
    {
        xf = tiling->getCanvasXform();
        settings = tiling->getSettings();
    }
    tilingMaker->setCanvasXform(xf);
    addLayer(tilingMaker);  // since this is a shared pointer it does not get deleted

    setBackgroundColor(Qt::white);
    setBackgroundImg(settings.getBkgdImage());
}

void WorkspaceViewer::viewMapEditor()
{
    qDebug() << "++WorkspaceViewer::viewFigMapEditor";

    MapEditorPtr ed  = MapEditor::getSharedInstance();

    addLayer(ed);           // since this is a shared pointer it does not get deleted

    ed->forceLayerRecalc();

    setWindowTitle("Map Editor");

    QSize sz = getActiveSize(VIEW_MAP_EDITOR);
    setBackgroundColor(config->figureViewBkgdColor);
    ed->setCenterScreen(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewFaceSet()
{
    qDebug() << "++WorkspaceViewer::viewFaceSet";
    LayerPtr fsView = make_shared<FaceSetView>(config->faces);
    addLayer(fsView);

    setBackgroundColor(Qt::black);
}

void WorkspaceViewer::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->viewerType]).arg(tp->getName()).arg(tp->getDescription()).arg(tp->getAuthor());
    setWindowTitle(str);
}

void  WorkspaceViewer::setBackgroundImg(BkgdImgPtr bkgd)
{
    if (bkgd)
    {
        if (bkgd->isLoaded() && bkgd->bShowBkgd)
        {
            addLayer(bkgd);
        }
    }
}

void  WorkspaceViewer::setBorder(BorderPtr bp)
{
    if (bp)
    {
        addLayer(bp);
    }
}
