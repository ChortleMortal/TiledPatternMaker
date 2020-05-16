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
#include "makers/figure_maker/figure_maker.h"
#include "viewers/placeddesignelementview.h"
#include "viewers/facesetview.h"
#include "viewers/figureview.h"
#include "viewers/PrototypeView.h"
#include "viewers/ProtoFeatureView.h"
#include "viewers/TilingView.h"
#include "viewers/workspaceviewer.h"
#include "style/Style.h"
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
    eViewType evt = VIEW_DESIGN;
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
}

void WorkspaceViewer::clear()
{
    qDebug() << "WorkspaceViewer::clear";
    qDeleteAll(mViewers);
    mViewers.clear();
    mStyles.clear();
    mDesigns.clear();
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
    qDebug() << "+ WorkspaceViewer::slot_viewWorkspace  type= " << sViewerType[config->viewerType];

    if (config->autoClear)
    {
        clear();
    }

    // prepare a fresh scene
    Scene * scene = canvas->swapScenes();
    if (config->autoClear)
    {
        canvas->clearScene();
    }

    // setup initial canvas settings according to view
    eViewType vtype = config->viewerType;

    switch (vtype)
    {
    case VIEW_DESIGN:
        viewDesign();
        break;

    case VIEW_PROTO:
        viewProto();
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
    }

    updateScene(scene);

    View * view = View::getInstance();
    view->setScene(scene);
    view->show();

    emit sig_viewUpdated();
}

void WorkspaceViewer::updateScene(Scene * scene)
{
    // TODO - border should be treated like background image and included in mStyles
    bp = canvasSettings.getBorder();
    if (bp)
    {
        // when border cleared from scene, its children become disconnected
        // this reconnects them by re-parenting
        bp->reconnectChildren();
        // adds to the scene
        scene->addItem(bp.get());
    }

    scene->setSceneRect(canvasSettings.getCanvasRect());
    scene->setBackgroundBrush(QBrush(canvasSettings.getBackgroundColor()));


    // now add to the scene
    bip = canvasSettings.getBkgdImage();
    if (bip)
    {
        if (bip->isLoaded() && bip->bShowBkgd)
        {
            BackgroundImage * bi = bip.get();
            mStyles.push_back(bi);
        }
    }

    for (auto it = mDesigns.begin(); it != mDesigns.end(); it++)
    {
        DesignPtr dp = *it;
        canvas->addDesign(dp.get());
    }

    //qDebug() << "mViewers count =" << mViewers.size();
    for (auto layer : mViewers)
    {
        //qDebug() << "layer" << Utils::addr(layer)  << layer->getName();
        scene->addItem(layer);    // this does it
    }

    //qDebug() << "mStyles count =" << mStyles.size();
    for (auto layer : mStyles)
    {
        //qDebug() << "layer" << Utils::addr(layer)  << layer->getName();
        scene->addItem(layer);    // this does it
    }

    if (config->circleX)
    {
        MarkX * item = new MarkX(scene->sceneRect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        scene->addItem(item);
    }
}

void WorkspaceViewer::viewDesign()
{
    if (config->designViewer == DV_SHAPES)
    {
        mDesigns = workspace->getDesigns();
        if (mDesigns.count())
        {
            DesignPtr dp = mDesigns.first();
            canvasSettings = dp->getDesignInfo();
        }
        emit sig_title(workspace->getDesignName());
    }
    else
    {
        eWsData wsdata = (config->designViewer == DV_LOADED_STYLE) ? WS_LOADED : WS_TILING;
        StyledDesign   & sd = workspace->getStyledDesign(wsdata);
        if (sd.hasContent())
        {
            // copy canvas settings to view settings
            canvasSettings = sd.getCanvasSettings();

            emit sig_title(sd.getName());

            ControlPanel * panel  = ControlPanel::getInstance();
            QString astring = QString("<span style=\"color:rgb(0,240,0)\">Preparing design: %1</span>").arg(sd.getName());
            panel->showStatus(astring);

#ifdef TPMSPLASH
            astring = QString("Preparing design: %1").arg(sd.getName());
            panel->showSplash(astring);
#endif
            qDebug() << "Viewing styled design" << sd.getName();
            const StyleSet & sset = sd.getStyleSet();
            for (auto it = sset.begin(); it != sset.end(); it++)
            {
                StylePtr  style = *it;
                Style *  pStyle = style.get();
                qDebug().noquote() << "Adding Style:" << Utils::addr(style.get()) << "  " << style->getDescription();
                style->createStyleRepresentation();   // important to do this here
                mStyles.push_back(pStyle);
            }

            panel->hideStatus();
#ifdef TPMSPLASH
            panel->hideSplash();
#endif
            emit sig_title(sd.getName());
        }
        else
        {
            qWarning() <<  "Empty StyleSet - nothing to display";
        }
    }


}

void WorkspaceViewer::viewProto()
{
    qDebug() << "++WorkspaceViewer::viewProto";
    eWsData wsdata = (config->protoViewer == PV_STYLE) ? WS_LOADED : WS_TILING;
    PrototypePtr pp = workspace->getPrototype(wsdata);
    if (pp)
    {
        emit sig_title("Prototype");
        TilingPtr tp = workspace->getTiling(wsdata);
        Q_ASSERT(tp);
        canvasSettings.setBkgdImage(tp->getBackground());

        ProtoView * protoView = new ProtoView(pp);
        mViewers.push_back(protoView);
    }
    else
    {
        qWarning() << "VIEW_PROTO: no prototype";
    }

    if (config->protoViewer == PV_WS)
    {
        canvasSettings.setBackgroundColor(TileBlack);
    }
    else
    {
        canvasSettings.setBackgroundColor(Qt::white);
    }
    canvasSettings.setCanvasSize(getViewSize(VIEW_PROTO));
}

void WorkspaceViewer::viewProtoFeature()
{
    qDebug() << "++WorkspaceViewer::viewProtoFeature";
    eWsData wsdata = (config->protoFeatureViewer == PVF_STYLE) ? WS_LOADED : WS_TILING;
    PrototypePtr pp = workspace->getPrototype(wsdata);
    if (pp)
    {
        TilingPtr tp = workspace->getTiling(wsdata);
        Q_ASSERT(tp);
        canvasSettings.setBkgdImage(tp->getBackground());

        ProtoFeatureView * pfv = new ProtoFeatureView(pp);
        mViewers.push_back(pfv);
        emit sig_title("WS Proto Feature");
    }
    else
    {
        qWarning() << "VIEW_PROTO_FEATURE: no prototype";
    }

    if (config->protoFeatureViewer == PVF_WS)
    {
        canvasSettings.setBackgroundColor(TileBlack);
    }
    else
    {
        canvasSettings.setBackgroundColor(Qt::white);
    }
    canvasSettings.setCanvasSize(getViewSize(VIEW_PROTO_FEATURE));
}

void WorkspaceViewer::viewDesignElement()
{
    qDebug() << "++WorkspaceViewer::viewDesignElement";
    eWsData wsdata = (config->delViewer == DEL_STYLES) ? WS_LOADED : WS_TILING;
    PrototypePtr pp = workspace->getPrototype(wsdata);
    if (pp)
    {
        TilingPtr   tiling                 = workspace->getTiling(wsdata);
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
                    PlacedDesignElementView * delView = new  PlacedDesignElementView(pdel);
                    mViewers.push_back(delView);
                }
            }
        }

        canvasSettings.setBkgdImage(tiling->getBackground());
        emit sig_title("Design element");
    }
    else
    {
        emit sig_title("Design element - not found");
    }

    canvasSettings.setBackgroundColor(Qt::white);
    canvasSettings.setCanvasSize(getViewSize(VIEW_DEL));
}

void WorkspaceViewer::viewFigureMaker()
{
    // this is used by FigureMaker
    // this is what we want to see painted

    eWsData dataset = (config->figureViewer == FV_STYLE) ? WS_LOADED : WS_TILING;
    DesignElementPtr dep = workspace->getSelectedDesignElement(dataset);
    if (!dep)
    {
        return;
    }

    qDebug() << "++WorkspaceViewer::viewFigure";

    FigureView * figView = new FigureView(dep);
    mViewers.push_back(figView);
    figView->setPos(canvasSettings.getStartTile());

    QString astring = QString("WS Figure: dep=%1 fig=%2").arg(Utils::addr(dep.get())).arg(Utils::addr(dep->getFigure().get()));
    emit sig_title(astring);

    QSize sz = getViewSize(VIEW_FIGURE_MAKER);
    canvasSettings.setCanvasSize(sz);
    canvasSettings.setBackgroundColor(config->figureViewBkgdColor);
    figView->setCenter(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewTiling()
{
    eWsData wsdata   = (config->tilingViewer == TV_STYLE) ? WS_LOADED : WS_TILING;
    TilingPtr tiling = workspace->getTiling(wsdata);
    if (tiling)
    {
        canvasSettings.setBkgdImage(tiling->getBackground());
        setTitle(tiling);
        qDebug() << "++WorkspaceViewer::viewTiling (tiling)";
        TilingView * tilingView = new TilingView(tiling);
        mViewers.push_back(tilingView);
    }
    canvasSettings.setBackgroundColor(Qt::white);
    canvasSettings.setCanvasSize(getViewSize(VIEW_TILING));

}

void WorkspaceViewer::viewTilingMaker()
{
    qDebug() << "++WorkspaceViewer::viewTilingMaker";

    TilingMaker * tilingMaker  = TilingMaker::getInstance();
    mStyles.push_back(tilingMaker);  // add to non-deletable styles not viewers

    eWsData input_wsdata = (config->tilingMakerViewer == TMV_STYLE) ? WS_LOADED : WS_TILING;
    TilingPtr tiling     = workspace->getTiling(input_wsdata);
    if (tiling)
    {
        setTitle(tiling);
        canvasSettings.setBkgdImage(tiling->getBackground());
    }
    canvasSettings.setBackgroundColor(Qt::white);
    canvasSettings.setCanvasSize(getViewSize(VIEW_TILING_MAKER));

}

void WorkspaceViewer::viewMapEditor()
{
    qDebug() << "++WorkspaceViewer::viewFigMapEditor";

    MapEditor * ed  = MapEditor::getInstance();

    mStyles.push_back(ed);  // add to non-deletable styles not viewers

    ed->setPos(canvasSettings.getStartTile());
    ed->forceUpdateLayer();

    emit sig_title("Map Editor");

    QSize sz = getViewSize(VIEW_MAP_EDITOR);
    canvasSettings.setCanvasSize(sz);
    canvasSettings.setBackgroundColor(config->figureViewBkgdColor);
    ed->setCenter(QRect(QPoint(0,0),sz).center());
}

void WorkspaceViewer::viewFaceSet()
{
    if (config->faceSet)
    {
        qDebug() << "++WorkspaceViewer::viewFaceSet";
        FaceSetView * fsView = new FaceSetView(config->faceSet);
        mViewers.push_back(fsView);
    }
    canvasSettings.setBackgroundColor(Qt::black);
    canvasSettings.setCanvasSize(getViewSize(VIEW_FACE_SET));
}

void WorkspaceViewer::slot_update()
{
    for (auto  layer : mViewers)
    {
        layer->update();
    }
    for (auto layer : mStyles)
    {
        layer->update();
    }
}

QVector<Layer*> WorkspaceViewer::getActiveLayers()
{
    QVector<Layer*> layers;
    layers  = mStyles;
    layers += mViewers;
    return layers;
}

bool WorkspaceViewer::hasLayers()
{
    return (mStyles.size() + mViewers.size());
}

void WorkspaceViewer::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->viewerType]).arg(tp->getName()).arg(tp->getDescription()).arg(tp->getAuthor());
    emit sig_title(str);
}

ViewSettings & WorkspaceViewer::getViewSettings(eViewType e)
{
    return viewSettings[e];
}

CanvasSettings & WorkspaceViewer::GetCanvasSettings()
{
    return canvasSettings;
}

QTransform WorkspaceViewer::getViewTransform(eViewType e)
{
    return viewSettings[e].getViewTransform();
}

QSizeF WorkspaceViewer::getSceneSize()
{
    return canvasSettings.getCanvasSize();
}

void WorkspaceViewer::setSceneSize(QSizeF sz)
{
    canvasSettings.setCanvasSize(sz);
}

QSize WorkspaceViewer::getViewSize(eViewType e)
{
    return viewSettings[e].getViewSize();
}

void WorkspaceViewer::setViewSize(eViewType e, QSize sz)
{
    viewSettings[e].setViewSize(sz);
}

QRect WorkspaceViewer::getViewRect(eViewType e)
{
    return QRect(QPoint(0,0),getViewSize(e));
}
