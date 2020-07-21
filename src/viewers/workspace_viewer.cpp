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

#include "base/canvas.h"
#include "base/border.h"
#include "base/tpmsplash.h"
#include "base/view.h"
#include "base/utilities.h"
#include "base/workspace.h"
#include "designs/patterns.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/figure_maker/prototype_maker.h"
#include "viewers/placed_designelement_view.h"
#include "viewers/faceset_view.h"
#include "viewers/figure_view.h"
#include "viewers/prototype_view.h"
#include "viewers/prototype_feature_view.h"
#include "viewers/tiling_view.h"
#include "viewers/workspace_viewer.h"
#include "style/style.h"
#include "panels/panel_status.h"
#include "panels/panel.h"

WorkspaceViewer * WorkspaceViewer::mpThis = nullptr;

WorkspaceViewer * WorkspaceViewer::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new WorkspaceViewer;
    }
    return mpThis;
}

WorkspaceViewer::WorkspaceViewer()
{
    view = View::getInstance();

    eViewType evt = VIEW_MOSAIC;
    ViewSettings * vs = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_PROTO;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_PROTO_FEATURE;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_DEL;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_TILING;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_TILING_MAKER;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_FACE_SET;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_FIGURE_MAKER;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize( 900, 900));

    evt = VIEW_MAP_EDITOR;
    vs  = &viewSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize( 900, 900));
}

void WorkspaceViewer::init()
{
    workspace       = Workspace::getInstance();
    config          = Configuration::getInstance();
    canvas          = Canvas::getInstance();

    disableAll();
    viewEnable(config->viewerType,true);
}

void WorkspaceViewer::clear()
{
    qDebug() << "WorkspaceViewer::clear";
    mViewers.clear();
    mDesigns.clear();
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

    clear();
    view->clearView();

    setupViews();

    // now add everything from the canvas settings to the view

    view->resize(currentCanvasSettings.getCanvasSize());
    view->setBackgroundColor(currentCanvasSettings.getBackgroundColor());

    BorderPtr bp = currentCanvasSettings.getBorder();
    if (bp)
    {
        view->addLayer(bp);
    }

    BkgdImgPtr bip = currentCanvasSettings.getBkgdImage();
    if (bip)
    {
        if (bip->isLoaded() && bip->bShowBkgd)
        {
            view->addLayer(bip);
        }
    }

    for (auto design :  mDesigns)
    {
        QVector<PatternPtr> & pats = design->getPatterns();
        for (auto pat : pats)
        {
            view->addLayer(pat);
        }
    }

    for (auto layer : mViewers)
    {
        view->addLayer(layer);
    }

    if (config->circleX)
    {
        MarkXPtr item = make_shared<MarkX>(view->rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        view->addLayer(item);
    }

    view->update();

    emit sig_viewUpdated();
}

void WorkspaceViewer::setupViews()
{
    //eViewType vtype = config->viewerType;

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

            case VIEW_PROTO:
                viewPrototype();
                break;

            case VIEW_PROTO_FEATURE:
                viewProtoFeature();
                break;

            case VIEW_DEL:
                viewDesignElement();
                break;

            case VIEW_FIGURE_MAKER:
                viewFigureMaker();
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
    mDesigns = workspace->getDesigns();
    if (mDesigns.count())
    {
        DesignPtr dp = mDesigns.first();
        currentCanvasSettings = dp->getDesignInfo();
    }
    emit sig_title(workspace->getDesignName());
}

void WorkspaceViewer::viewMosaic()
{
    if (MosaicPtr mosaic = setCanvasFromDesign())
    {
        QString name = mosaic->getName();
        qDebug() << "WorkspaceViewer::viewMosaic" << name;

        ControlPanel * panel  = ControlPanel::getInstance();
        QString astring = QString("<span style=\"color:rgb(0,240,0)\">Preparing design: %1</span>").arg(name);
        panel->showStatus(astring);

#ifdef TPMSPLASH
        astring = QString("Preparing design: %1").arg(name);
        panel->showSplash(astring);
#endif
        const StyleSet & sset = mosaic->getStyleSet();
        for (auto style : sset)
        {
            qDebug().noquote() << "Adding Style:" << Utils::addr(style.get()) << "  " << style->getDescription();
            style->createStyleRepresentation();   // important to do this here
            mViewers.push_back(style);
        }

        panel->hideStatus();
#ifdef TPMSPLASH
        panel->hideSplash();
#endif
        emit sig_title(name);
    }
    else
    {
        qWarning() <<  "Empty StyleSet - nothing to display";
    }
}

void WorkspaceViewer::viewPrototype()
{
    qDebug() << "++WorkspaceViewer::viewPrototype";

    StylePtr sp;
    MosaicPtr mosaic = setCanvasFromDesign();
    if (mosaic)
    {
        sp = mosaic->getFirstStyle();
    }

    PrototypePtr pp = workspace->getSelectedPrototype();
    if (pp)
    {
        emit sig_title("Prototype");
        TilingPtr tiling = workspace->getTiling();
        Q_ASSERT(tiling);

        LayerPtr protoView = make_shared<ProtoView>(pp);
        if (sp)
        {
            // use settings from design
            Xform xf = sp->getCanvasXform();
            protoView->setCanvasXform(xf);
        }
        else
        {
            // use settings from tiling maker
            setCanvasFromTiling(tiling,protoView);
        }
        mViewers.push_back(protoView);

        currentCanvasSettings.setBkgdImage(tiling->getBackground());
    }
    else
    {
        qWarning() << "viewProto: no  selected prototype";
    }

    currentCanvasSettings.setBackgroundColor(TileBlack);
    currentCanvasSettings.setCanvasSize(getViewSize(VIEW_PROTO));
}

void WorkspaceViewer::viewProtoFeature()
{
    qDebug() << "++WorkspaceViewer::viewProtoFeature";

    StylePtr sp;
    MosaicPtr mosaic = setCanvasFromDesign();
    if (mosaic)
    {
        sp = mosaic->getFirstStyle();
    }

    PrototypePtr pp = workspace->getSelectedPrototype();
    if (pp)
    {
        TilingPtr tiling = workspace->getTiling();
        Q_ASSERT(tiling);

        LayerPtr pfview = make_shared<ProtoFeatureView>(pp);
        if (sp)
        {
            // use settings from design
            Xform xf = sp->getCanvasXform();
            pfview->setCanvasXform(xf);
        }
        else
        {
            // use settings from tiling maker
            setCanvasFromTiling(tiling,pfview);
        }
        mViewers.push_back(pfview);

        currentCanvasSettings.setBkgdImage(tiling->getBackground());
        emit sig_title("WS Proto Feature");
    }
    else
    {
        qWarning() << "viewProtoFeature: no selected prototype";
    }

    currentCanvasSettings.setBackgroundColor(TileBlack);
    currentCanvasSettings.setCanvasSize(getViewSize(VIEW_PROTO_FEATURE));
}

void WorkspaceViewer::viewDesignElement()
{
    qDebug() << "++WorkspaceViewer::viewDesignElement";

    StylePtr sp;
    MosaicPtr mosaic = setCanvasFromDesign();
    if (mosaic)
    {
        sp = mosaic->getFirstStyle();
    }

    PrototypePtr pp = workspace->getSelectedPrototype();
    if (pp)
    {
        TilingPtr   tiling                 = workspace->getTiling();
        Q_ASSERT(tiling);
        QList<PlacedFeaturePtr>	& placed   = tiling->getPlacedFeatures();
        QVector<DesignElementPtr> &  dels  = pp->getDesignElements();
        QVector<QTransform>       & tforms = pp->getLocations();

        qDebug() << "dels=" << dels.size() << "tforms="  << tforms.size();
        for (auto delp : dels)
        {
            FeaturePtr  feap = delp->getFeature();
            for (auto pfp : placed)
            {
                if (feap == pfp->getFeature())
                {
                    QTransform tr                  = pfp->getTransform();
                    PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                    LayerPtr delView               = make_shared<PlacedDesignElementView>(pdel);
                    if (sp)
                    {
                        // use settings from design
                        Xform xf = sp->getCanvasXform();
                        delView->setCanvasXform(xf);
                    }
                    else
                    {
                        // use settings from tiling maker
                        setCanvasFromTiling(tiling,delView);
                    }
                    mViewers.push_back(delView);
                }
            }
        }

        currentCanvasSettings.setBkgdImage(tiling->getBackground());
        emit sig_title("Design element");
    }
    else
    {
        emit sig_title("viewDesignElement - no seleected protype");
    }

    currentCanvasSettings.setBackgroundColor(Qt::white);
    currentCanvasSettings.setCanvasSize(getViewSize(VIEW_DEL));
}

void WorkspaceViewer::viewFigureMaker()
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
    mViewers.push_back(figView);

    QString astring = QString("WS Figure: dep=%1 fig=%2").arg(Utils::addr(dep.get())).arg(Utils::addr(dep->getFigure().get()));
    emit sig_title(astring);

    QSize sz = getViewSize(VIEW_FIGURE_MAKER);
    currentCanvasSettings.setCanvasSize(sz);
    currentCanvasSettings.setBackgroundColor(config->figureViewBkgdColor);
    figView->setCenter(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewTiling()
{
    TilingPtr tiling = workspace->getTiling();
    if (!tiling)
    {
        currentCanvasSettings.setBackgroundColor(Qt::white);
        currentCanvasSettings.setCanvasSize(getViewSize(VIEW_TILING));
        return;
    }

    qDebug() << "++WorkspaceViewer::viewTiling (tiling)"  << tiling->getName();

    TilingViewPtr tilingView = make_shared<TilingView>(tiling);

    if (MosaicPtr mosaic = setCanvasFromDesign())
    {
        // use settings from design
        StylePtr sp = mosaic->getFirstStyle();
        Xform xf = sp->getCanvasXform();
        tilingView->setCanvasXform(xf);
    }
    else
    {
        // use settings from tiling maker
        setCanvasFromTiling(tiling,tilingView);
    }

    currentCanvasSettings.setBackgroundColor(Qt::white);
    currentCanvasSettings.setBkgdImage(tiling->getBackground());
    setTitle(tiling);

    mViewers.push_back(tilingView);
}

void WorkspaceViewer::viewTilingMaker()
{
    TilingPtr tiling     = workspace->getTiling();
    if (!tiling)
    {
        currentCanvasSettings.setBackgroundColor(Qt::white);
        currentCanvasSettings.setCanvasSize(getViewSize(VIEW_TILING_MAKER));
        return;
    }

    qDebug() << "++WorkspaceViewer::viewTilingMaker" << tiling->getName();

    TilingMakerPtr tilingMaker = TilingMaker::getInstance();
    if (MosaicPtr mosaic = setCanvasFromDesign())
    {
        // use settings from design
        StylePtr sp = mosaic->getFirstStyle();
        Xform xf    = sp->getCanvasXform();
        tilingMaker->setCanvasXform(xf);
    }
    else
    {
        setCanvasFromTiling(tiling,tilingMaker);
    }

    currentCanvasSettings.setBackgroundColor(Qt::white);

    mViewers.push_back(tilingMaker);  // add to non-deletable styles not viewers
}

void WorkspaceViewer::viewMapEditor()
{
    qDebug() << "++WorkspaceViewer::viewFigMapEditor";

    MapEditorPtr ed  = MapEditor::getInstance();

    mViewers.push_back(ed);  // add to non-deletable styles not viewers

    ed->forceUpdateLayer();

    emit sig_title("Map Editor");

    QSize sz = getViewSize(VIEW_MAP_EDITOR);
    currentCanvasSettings.setCanvasSize(sz);
    currentCanvasSettings.setBackgroundColor(config->figureViewBkgdColor);
    ed->setCenter(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewFaceSet()
{
    if (config->faceSet)
    {
        qDebug() << "++WorkspaceViewer::viewFaceSet";
        LayerPtr fsView = make_shared<FaceSetView>(config->faceSet);
        mViewers.push_back(fsView);
    }
    currentCanvasSettings.setBackgroundColor(Qt::black);
    currentCanvasSettings.setCanvasSize(getViewSize(VIEW_FACE_SET));
}

void WorkspaceViewer::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->viewerType]).arg(tp->getName()).arg(tp->getDescription()).arg(tp->getAuthor());
    emit sig_title(str);
}

//
// canvas
//

CanvasSettings & WorkspaceViewer::getCurrentCanvasSettings()
{
    return currentCanvasSettings;
}

QSize WorkspaceViewer::getCurrentCanvasSize()
{
    QSize sz =  currentCanvasSettings.getCanvasSize();
    qDebug().noquote() << "WorkspaceViewer::getCurrentCanvasSize()" << sz;
    return sz;
}

void WorkspaceViewer::setCurrentCanvasSize(QSize sz)
{
    qDebug().noquote() << "WorkspaceViewer::setCurrentCanvasSize()" << sz;
    currentCanvasSettings.setCanvasSize(sz);
}

//
// view
//

QTransform WorkspaceViewer::getViewTransform(eViewType e)
{
    QTransform t = viewSettings[e].getViewTransform();
    //qDebug().noquote() << "WorkspaceViewer::getViewTransform" << sViewerType[e] << Transform::toInfoString(t);
    return t;
}

ViewSettings & WorkspaceViewer::getViewSettings(eViewType e)
{
    return viewSettings[e];
}

QSize WorkspaceViewer::getViewSize(eViewType e)
{
    QSize sz =  viewSettings[e].getViewSize();
    qDebug().noquote() << "WorkspaceViewer::getViewSize()" << sViewerType[e] << sz;
    return sz;
}

void WorkspaceViewer::setViewSize(eViewType e, QSize sz)
{
    qDebug().noquote() << "WorkspaceViewer::setViewSize()" << sViewerType[e] << sz;
    viewSettings[e].setViewSize(sz);
}

//
// setters
//

MosaicPtr WorkspaceViewer::setCanvasFromDesign()
{
    MosaicPtr mosaic = workspace->getMosaic();
    if (mosaic && mosaic->hasContent())
    {
        // copy canvas settings to view settings
        currentCanvasSettings = mosaic->getCanvasSettings();
        return mosaic;
    }
    mosaic.reset();
    return mosaic;
}

void WorkspaceViewer::setCanvasFromTiling(TilingPtr tiling, LayerPtr layer)
{
    Xform xf = tiling->getCanvasXform();
    layer->setCanvasXform(xf);

    QSize size = tiling->getCanvasSize();
    currentCanvasSettings.setCanvasSize(size);

    setTitle(tiling);
}

