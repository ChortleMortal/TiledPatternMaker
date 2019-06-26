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

#include "base/configuration.h"
#include "base/canvas.h"
#include "base/misc.h"
#include "base/patterns.h"
#include "base/tpmsplash.h"
#include "base/utilities.h"
#include "base/workspace.h"
#include "makers/TilingDesigner.h"
#include "makers/mapeditor.h"
#include "viewers/designelementview.h"
#include "viewers/facesetview.h"
#include "viewers/figureview.h"
#include "viewers/PrototypeView.h"
#include "viewers/ProtoFeatureView.h"
#include "viewers/TilingView.h"
#include "viewers/workspaceviewer.h"
#include "style/Style.h"

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
    workspace       = Workspace::getInstance();
    config          = Configuration::getInstance();
    canvas          = nullptr;
}

void WorkspaceViewer::setCanvas(Canvas * canvas)
{
    this->canvas = canvas;
}

void WorkspaceViewer::clear()
{
    qDeleteAll(mViewers);
    mViewers.clear();
    mStyles.clear();
}

void WorkspaceViewer::slot_viewWorkspace()
{
    // prevent re-entrant call from the same thread

    static bool busy   = false;
    static bool review = false;

    if (busy)
    {
        qDebug() << "+ DesignViewer::slot_viewWorkspace - BUSY IGNORED" ;
        review = true;
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
    qDebug() << "+ DesignViewer::slot_viewWorkspace";

    if (config->autoClear)
    {
        canvas->clearCanvas();
        clear();
    }

    if (config->viewerType == VIEW_DESIGN)
    {
        if (config->designViewer == DV_SHAPES)
        {
            QVector<DesignPtr> & designs = workspace->getDesigns();
            for (auto it = designs.begin(); it != designs.end(); it++)
            {
                DesignPtr dp = *it;
                canvas->addDesign(dp.get());
            }
            emit sig_title(workspace->getDesignName());
        }
        else
        {
            StyledDesign   & sd = (config->designViewer == DV_LOADED_STYLE) ? workspace->getLoadedStyles() : workspace->getWsStyles();
            viewStyledDesign(sd);

            emit sig_title(sd.getName());
        }
    }
    else if (config->viewerType == VIEW_PROTO)
    {
        if (config->protoViewer == PV_STYLE)    // style
        {
            StyledDesign   & sd = workspace->getLoadedStyles();
            const StyleSet & sset = sd.getStyleSet();
            for (auto it = sset.begin(); it != sset.end(); it++)
            {
                StylePtr  pStyle = *it;
                qDebug().noquote() << "Style:" << pStyle->getDescription();
                qDebug().noquote() << "Add style to  viewProto:" << pStyle->getInfo();
                ProtoView * pview = viewProto(pStyle);
                Bounds b(-10.0,10.0,20.0);
                pview->setBounds(b);
            }
            emit sig_title(workspace->getLoadedStyles().getName());
        }
        else if (config->protoViewer == PV_WS)
        {
            PrototypePtr pp = workspace->getWSPrototype();
            if (pp)
            {
                ProtoView * pview = viewProto(pp);
                Bounds b(-10.0,10.0,20.0);
                pview->setBounds(b);
            }
        }
        emit sig_title("WS Prototype");
    }
    else if (config->viewerType == VIEW_PROTO_FEATURE)
    {
        if (config->protoFeatureViewer == PVF_STYLE)
        {
            StyledDesign   & sd = workspace->getLoadedStyles();
            const StyleSet & sset = sd.getStyleSet();
            for (auto it = sset.begin(); it != sset.end(); it++)
            {
                StylePtr  pStyle = *it;
                qDebug().noquote() << pStyle->getDescription();
                qDebug().noquote() << "Add style to  DesignViewer:" << pStyle->getInfo();
                ProtoFeatureView * pfv = viewProtoFeature(pStyle);
                Bounds b(-10.0,10.0,20.0);
                pfv->setBounds(b);
            }
            emit sig_title(workspace->getLoadedStyles().getName());
        }
        else if (config->protoFeatureViewer == PVF_WS)
        {
            PrototypePtr pp = workspace->getWSPrototype();
            if (pp)
            {
                ProtoFeatureView * pfv = viewProtoFeature(pp);
                Bounds b(-10.0,10.0,20.0);
                pfv->setBounds(b);
            }
            emit sig_title("WS Proto Feature");
        }
    }
    else if (config->viewerType == VIEW_DEL)
    {
        StyledDesign & sd  = (config->delViewer == DEL_STYLES) ? workspace->getLoadedStyles() : workspace->getWsStyles();

        StylePtr st = sd.getFirstStyle();
        if (!st) return;

        PrototypePtr pp                    = st->getPrototype();
        TilingPtr   tiling                 = pp->getTiling();
        QList<PlacedFeaturePtr>	& placed   = tiling->getPlacedFeatures();
        QVector<DesignElementPtr> &  dels  = pp->getDesignElements();
        QVector<Transform>        & tforms = pp->getLocations();
        qDebug() << "dels=" << dels.size() << "tforms="  << tforms.size();
        for (auto it = dels.begin(); it != dels.end(); it++)
        {
            DesignElementPtr delp = *it;
            FeaturePtr       feap = delp->getFeature();
            for (auto it2 = placed.begin(); it2 != placed.end(); it2++)
            {
                PlacedFeaturePtr pfp = *it2;
                if (feap == pfp->getFeature())
                {
                    Transform tr                   = pfp->getTransform();
                    PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                    PlacedDesignElementView * delv = viewPlacedDesignElement(pdel);
                    Bounds b(-10.0,10.0,20.0);
                    delv->setBounds(b);
                }
            }
        }
        emit sig_title(sd.getName());
    }
    else if (config->viewerType == VIEW_FIGURE_MAKER)
    {
        // this is used by FigureMaker
        // this is what we want to see painted
        DesignElementPtr dep = workspace->getWSDesignElement();
        if (!dep)
        {
            return;
        }

        CanvasSettings di = canvas->getCanvasSettings();

        FigureView * figView = viewFigure(dep, di.getCenter());
        figView->setPen(QPen(Qt::green,2));
        Bounds b(-10.0,10.0,20.0);
        figView->setBounds(b);
        figView->setPos(di.getStartTile());

        QString astring = QString("WS Figure: dep=%1 fig=%2").arg(Utils::addr(dep.get())).arg(Utils::addr(dep->getFigure().get()));
        emit sig_title(astring);
    }
    else if (config->viewerType == VIEW_TILING)
    {
        TilingPtr tp;
        switch (config->tilingViewer)
        {
        case TV_STYLE:
            tp = workspace->getLoadedStyles().getTiling();
            if (tp)
                emit sig_title(tp->getName());  // DAC FIXME - this is where a style tiling name would be used
            break;
        case TV_WORKSPACE:
            tp = workspace->getTiling();
            if (tp)
                emit sig_title("Tiling");
            break;
        }
        if (!tp) return;

        TilingView * tv = viewTiling(tp);
        Bounds b(-10.0,10.0,20.0);
        tv->setBounds(b);
    }
    else if (config->viewerType == VIEW_TILIING_MAKER)
    {
        TilingDesigner * td = viewTilingDesigner();
        Bounds b(-5.0,5.0,10.0);
        td->setBounds(b);

        emit sig_title("Tiling Designer");
    }
    else if (config->viewerType == VIEW_MAP_EDITOR)
    {
        CanvasSettings di = canvas->getCanvasSettings();

        MapEditor * mev = viewFigMapEditor(di.getCenter());
        mev->setPos(di.getStartTile());
        mev->forceUpdateLayer();

        emit sig_title("Map Editor");
    }
    else if (config->viewerType == VIEW_FACE_SET)
    {
        if (config->faceSet)
        {
            FaceSetView * fsv = viewFaceSet(config->faceSet);
            Bounds b(-10.0,10.0,20.0);
            fsv->setBounds(b);
        }
    }

    for (auto it= mViewers.begin(); it != mViewers.end(); it++)
    {
        Layer * viewer = *it;
        canvas->addItem(viewer);    // this does it
    }

    for (auto it= mStyles.begin(); it != mStyles.end(); it++)
    {
        Layer * viewer = *it;
        canvas->addItem(viewer);    // this does it
    }

    CanvasSettings di = canvas->getCanvasSettings();
    canvas->writeBorderSettings(di);

    if (config->circleX)
    {
        CanvasSettings di = canvas->getCanvasSettings();

        MarkX * item = new MarkX(di.getCenter(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        canvas->addItem(item);
    }
    canvas->update();
}

void  WorkspaceViewer::slot_updateDesignInfo()
{
    CanvasSettings di = canvas->getCanvasSettings();

    switch (config->viewerType)
    {
    case VIEW_DESIGN:

        switch (config->designViewer)
        {
        case DV_SHAPES:
        {
            QVector<DesignPtr> & designs = workspace->getDesigns();
            if (designs.count())
            {
                DesignPtr dp = designs.first();
                di = dp->getDesignInfo();
            }
            break;
        }
        case DV_LOADED_STYLE:
        {
            StyledDesign & sd = workspace->getLoadedStyles();
            di = sd.getCanvasSettings();
            break;
        }
        case DV_WS_STYLE:
        {
            StyledDesign & sd = workspace->getWsStyles();
            di = sd.getCanvasSettings();
            break;
        }
        }
        break;
    case VIEW_PROTO:
        if (config->protoViewer == PV_STYLE)    // style
        {
            di.setSizeF(QSizeF(1500.0,1100.0));
            di.setBackgroundColor((QColor(Qt::white)));
            di.setDiameter(200.0);
            di.setStartTile(QPointF(0,0));
        }
        else if (config->protoViewer == PV_WS)
        {
            di.setSizeF(QSizeF(1500.0,1100.0));
            di.setBackgroundColor((TileBlack));
            di.setStartTile(QPointF(0,0));
            di.setDiameter(800.0);
        }
        break;

    case VIEW_PROTO_FEATURE:
        if (config->protoFeatureViewer == PVF_STYLE)
        {
            di.setSizeF(QSizeF(1500.0,1100.0));
            di.setBackgroundColor((QColor(Qt::white)));
            di.setDiameter(200.0);
            di.setStartTile(QPointF(0,0));
        }
        else if (config->protoFeatureViewer == PVF_WS)
        {
            di.setSizeF(QSizeF(1500.0,1100.0));
            di.setBackgroundColor((TileBlack));
            di.setStartTile(QPointF(0,0));
            di.setDiameter(800.0);
        }
        break;

    case VIEW_DEL:
        di.setSizeF(QSizeF(1500.0,1100.0));
        di.setBackgroundColor((TileBlack));
        di.setStartTile(QPointF(0,0));
        di.setDiameter(800.0);
        break;

    case VIEW_FIGURE_MAKER:
    case VIEW_MAP_EDITOR:
        di.setSizeF(QSizeF(900.0,900.0));
        di.setBackgroundColor(config->figureViewBkgdColor);
        di.setStartTile(QPointF(0.0,0.0));
        di.setDiameter(200.0);
        break;

    case VIEW_TILING:
        di.setSizeF(QSizeF(1500.0,1100.0));
        di.setBackgroundColor((Qt::white));
        di.setDiameter(200.0);
        di.setStartTile(QPointF(0,0));
        break;

    case VIEW_TILIING_MAKER:
        di.setSizeF(QSizeF(1000.0,1000.0));
        di.setBackgroundColor((Qt::white));
        di.setStartTile(QPointF(0,0));
        di.setDiameter(200.0);
        break;

    case VIEW_FACE_SET:
        break;  // do nothin
    }
    canvas->writeCanvasSettings(di);
}

void WorkspaceViewer::slot_update()
{
    for (auto it = mViewers.begin(); it != mViewers.end(); it++)
    {
        Layer * l = *it;
        l->update();
    }
    for (auto it = mStyles.begin(); it != mStyles.end(); it++)
    {
        Layer * l = *it;
        l->update();
    }
}

void WorkspaceViewer::viewStyledDesign(StyledDesign &sd)
{
#ifdef TPMSPLASH
    TPMSplash * sp = TPMSplash::getInstance();
    QString astring = QString("Preparing design: %1").arg(sd.getName());
    sp->display(astring);
#endif

    qDebug() << "Viewing styled design" << sd.getName();
    const StyleSet & sset = sd.getStyleSet();
    for (auto it = sset.begin(); it != sset.end(); it++)
    {
        StylePtr  style = *it;
        qDebug().noquote() << "Adding Style:" << Utils::addr(style.get()) << "  " << style->getDescription();
        style->createStyleRepresentation();   // important to do this here
        mStyles.push_back(style.get());
    }

#ifdef TPMSPLASH
    sp->hide();
#endif
}

ProtoView *  WorkspaceViewer::viewProto(StylePtr style)
{
    qDebug() << "++WorkspaceViewer::viewProto (style)";

    PrototypePtr pp = style->getPrototype();

    return viewProto(pp);
}

ProtoView * WorkspaceViewer::viewProto(PrototypePtr proto)
{
    qDebug() << "++WorkspaceViewer::viewProto (proto)";

    ProtoView * protoView = new ProtoView(proto);
    protoView->getLayerTransform();

    mViewers.push_back(protoView);

    return protoView;
}

ProtoFeatureView * WorkspaceViewer::viewProtoFeature(StylePtr style)
{
    qDebug() << "++WorkspaceViewer::viewProtoFeature (style)";

    PrototypePtr pp = style->getPrototype();

    return viewProtoFeature(pp);
}

ProtoFeatureView * WorkspaceViewer::viewProtoFeature( PrototypePtr proto)
{
    qDebug() << "++WorkspaceViewer::viewProtoFeature (proto)";

    ProtoFeatureView * protoView = new ProtoFeatureView(proto);
    protoView->getLayerTransform();

    mViewers.push_back(protoView);

    return protoView;
}

TilingView * WorkspaceViewer::viewTiling(StylePtr style)
{
    qDebug() << "++WorkspaceViewer::viewTiling (style)";

    TilingPtr tiling = style->getPrototype()->getTiling();

    return viewTiling(tiling);
}

TilingView * WorkspaceViewer::viewTiling(TilingPtr tiling)
{
    qDebug() << "++WorkspaceViewer::viewTiling (tiling)";

    TilingView * tilingView = new TilingView(tiling);
    tilingView->getLayerTransform();

    mViewers.push_back(tilingView);

    return tilingView;
}

TilingDesigner * WorkspaceViewer::viewTilingDesigner()
{
    qDebug() << "++WorkspaceViewer::viewTilingDesigner";

    TilingDesigner * tilingDesigner  = TilingDesigner::getInstance();

    mStyles.push_back(tilingDesigner);  // add to non-deletable styles not viewers

    return tilingDesigner;
}

PlacedDesignElementView * WorkspaceViewer::viewPlacedDesignElement(PlacedDesignElementPtr pde)
{
    qDebug() << "++WorkspaceViewer::viewDesignElement:" << pde->toString();

    PlacedDesignElementView * delView = new  PlacedDesignElementView(pde);
    delView->getLayerTransform();

    mViewers.push_back(delView);

    return delView;
}

MapEditor * WorkspaceViewer::viewFigMapEditor(QPointF center)
{
    qDebug() << "++WorkspaceViewer::viewFigMapEditor";

    MapEditor * ed  = MapEditor::getInstance();
    ed->setRotateCenter(center);

    mStyles.push_back(ed);  // add to non-deletable styles not viewers

    return ed;
}

FigureView * WorkspaceViewer::viewFigure(DesignElementPtr dep, QPointF center)
{
    qDebug() << "++WorkspaceViewer::viewFigure";

    FigureView * figView = new FigureView(dep);
    figView->setRotateCenter(center);

    mViewers.push_back(figView);

    return figView;
}

FaceSetView  * WorkspaceViewer::viewFaceSet(FaceSet * set)
{
    qDebug() << "++WorkspaceViewer::viewFaceSet";

    FaceSetView * fsView = new FaceSetView(set);

    mViewers.push_back(fsView);

    return fsView;
}

QVector<Layer*> WorkspaceViewer::getActiveLayers()
{
    QVector<Layer*> layers;
    layers  = mStyles;
    layers += mViewers;
    return layers;
}

