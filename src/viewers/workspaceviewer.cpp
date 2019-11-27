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
#include "base/misc.h"
#include "base/tpmsplash.h"
#include "base/view.h"
#include "base/utilities.h"
#include "base/workspace.h"
#include "designs/patterns.h"
#include "makers/tilingmaker.h"
#include "makers/mapeditor.h"
#include "viewers/placeddesignelementview.h"
#include "viewers/facesetview.h"
#include "viewers/figureview.h"
#include "viewers/PrototypeView.h"
#include "viewers/ProtoFeatureView.h"
#include "viewers/TilingView.h"
#include "viewers/workspaceviewer.h"
#include "style/Style.h"

SizeAndBounds WorkspaceViewer::viewDimensions[] = {
    {VIEW_DESIGN,       Bounds(-10.0,10.0,20.0), QSizeF(1500,1100)},  // dummy values
    {VIEW_PROTO,        Bounds(-10.0,10.0,20.0), QSizeF(1500,1100)},
    {VIEW_PROTO_FEATURE,Bounds(-10.0,10.0,20.0), QSizeF(1500,1100)},
    {VIEW_DEL,          Bounds(-10.0,10.0,20.0), QSizeF(1500,1100)},
    {VIEW_FIGURE_MAKER, Bounds(-10.0,10.0,20.0), QSizeF( 900, 900)},
    {VIEW_TILING,       Bounds(-10.0,10.0,20.0), QSizeF(1500,1100)},
    {VIEW_TILIING_MAKER,Bounds( -5.0, 5.0,10.0), QSizeF(1000,1000)},  // dummy
    {VIEW_MAP_EDITOR,   Bounds(-10.0,10.0,20.0), QSizeF( 900, 900)},  // dummy
    {VIEW_FACE_SET,     Bounds( 10.0,10.0,20.0), QSizeF(1500,1100)}   // not relevant

};

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
    for (int i= VIEW_START; i <= VIEW_MAX; i++)
    {
        SizeAndBounds sab = viewDimensions[i];
        QTransform t      = calculateViewTransform(sab);
        viewTransforms.push_back(t);
    }
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

    eViewType vtype = config->viewerType;
    switch (vtype)
    {
    case VIEW_DESIGN:
        if (config->designViewer == DV_SHAPES)
        {
            mDesigns = workspace->getDesigns();

            emit sig_title(workspace->getDesignName());
        }
        else
        {
            StyledDesign   & sd = (config->designViewer == DV_LOADED_STYLE) ? workspace->getLoadedStyles() : workspace->getWsStyles();
            viewStyledDesign(sd);

            emit sig_title(sd.getName());
        }
        break;

    case VIEW_PROTO:
        if (config->protoViewer == PV_STYLE)    // style
        {
            StyledDesign   & sd = workspace->getLoadedStyles();
            const StyleSet & sset = sd.getStyleSet();
            for (auto it = sset.begin(); it != sset.end(); it++)
            {
                StylePtr  pStyle = *it;
                qDebug().noquote() << "Style:" << pStyle->getDescription();
                qDebug().noquote() << "Add style to  viewProto:" << pStyle->getInfo();
                viewProto(pStyle);
            }
            emit sig_title(workspace->getLoadedStyles().getName());
        }
        else if (config->protoViewer == PV_WS)
        {
            PrototypePtr pp = workspace->getWSPrototype();
            if (pp)
            {
                viewProto(pp);
            }
        }
        emit sig_title("WS Prototype");
        break;

    case VIEW_PROTO_FEATURE:
        if (config->protoFeatureViewer == PVF_STYLE)
        {
            StyledDesign   & sd = workspace->getLoadedStyles();
            const StyleSet & sset = sd.getStyleSet();
            for (auto it = sset.begin(); it != sset.end(); it++)
            {
                StylePtr  pStyle = *it;
                qDebug().noquote() << pStyle->getDescription();
                qDebug().noquote() << "Add style to  WorkspaceViewer:" << pStyle->getInfo();
                viewProtoFeature(pStyle);
            }
            emit sig_title(workspace->getLoadedStyles().getName());
        }
        else if (config->protoFeatureViewer == PVF_WS)
        {
            PrototypePtr pp = workspace->getWSPrototype();
            if (pp)
            {
                viewProtoFeature(pp);
            }
            emit sig_title("WS Proto Feature");
        }
        break;

    case VIEW_DEL:
    {
        StyledDesign & sd  = (config->delViewer == DEL_STYLES) ? workspace->getLoadedStyles() : workspace->getWsStyles();

        StylePtr st = sd.getFirstStyle();
        if (!st) return;

        PrototypePtr pp                    = st->getPrototype();
        TilingPtr   tiling                 = pp->getTiling();
        QList<PlacedFeaturePtr>	& placed   = tiling->getPlacedFeatures();
        QVector<DesignElementPtr> &  dels  = pp->getDesignElements();
        QVector<QTransform>       & tforms = pp->getLocations();
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
                    QTransform tr                  = pfp->getTransform();
                    PlacedDesignElementPtr pdel    = make_shared<PlacedDesignElement>(delp,tr);
                    viewPlacedDesignElement(pdel);
                }
            }
        }
        emit sig_title(sd.getName());
        break;
    }

    case VIEW_FIGURE_MAKER:
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
        figView->setPos(di.getStartTile());

        QString astring = QString("WS Figure: dep=%1 fig=%2").arg(Utils::addr(dep.get())).arg(Utils::addr(dep->getFigure().get()));
        emit sig_title(astring);
        break;
    }

    case VIEW_TILING:
    {
        TilingPtr tp;
        if (config->tilingViewer  == TV_STYLE)
        {
            tp = workspace->getLoadedStyles().getTiling();
        }
        else
        {
            Q_ASSERT(config->tilingViewer == TV_WORKSPACE);
            tp = workspace->getTiling();
        }
        if (tp)
        {
            setTitle(tp);
            viewTiling(tp);
        }
    }
        break;

    case VIEW_TILIING_MAKER:
    {
        TilingMaker * td = viewTilingMaker();
        TilingPtr  tp       = td->getTiling();
        setTitle(tp);
    }
        break;

    case VIEW_MAP_EDITOR:
    {
        CanvasSettings di = canvas->getCanvasSettings();

        MapEditor * mev = viewFigMapEditor(di.getCenter());
        mev->setPos(di.getStartTile());
        mev->forceUpdateLayer();

        emit sig_title("Map Editor");
        break;
    }

    case VIEW_FACE_SET:
        if (config->faceSet)
        {
            viewFaceSet(config->faceSet);
        }
        break;
    }

    // now add to the scene

    Scene * scene = canvas->swapScenes();

    if (config->autoClear)
    {
        canvas->clearCanvas();
    }

    updateDesignInfo();        // always done

    for (auto it = mDesigns.begin(); it != mDesigns.end(); it++)
    {
        DesignPtr dp = *it;
        canvas->addDesign(dp.get());
    }

    for (auto it= mViewers.begin(); it != mViewers.end(); it++)
    {
        Layer * viewer = *it;
        scene->addItem(viewer);    // this does it
    }

    qDebug() << "mStyles count =" << mStyles.size();
    for (auto it= mStyles.begin(); it != mStyles.end(); it++)
    {
        Layer * viewer = *it;
        qDebug() << "layer" << Utils::addr(viewer)  << viewer->getName();
        scene->addItem(viewer);    // this does it
    }

    if (config->circleX)
    {
        CanvasSettings di = canvas->getCanvasSettings();

        MarkX * item = new MarkX(di.getCenter(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        scene->addItem(item);
    }

    View * view = View::getInstance();
    view->setScene(scene);
    view->matchSizeToCanvas();
    view->show();
}

void  WorkspaceViewer::updateDesignInfo()
{
    CanvasSettings di;      // start with defaults

    eViewType vtype = config->viewerType;
    switch (vtype)
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
        di.setSizeF(viewDimensions[vtype].viewSize);
        if (config->protoViewer == PV_STYLE)    // style
        {
            di.setBackgroundColor((QColor(Qt::white)));
            di.setDiameter(200.0);
            di.setStartTile(QPointF(0,0));
        }
        else if (config->protoViewer == PV_WS)
        {
            di.setBackgroundColor((TileBlack));
            di.setStartTile(QPointF(0,0));
            di.setDiameter(800.0);
        }
        break;

    case VIEW_PROTO_FEATURE:
        di.setSizeF(viewDimensions[vtype].viewSize);
        if (config->protoFeatureViewer == PVF_STYLE)
        {
            di.setBackgroundColor((QColor(Qt::white)));
            di.setDiameter(200.0);
            di.setStartTile(QPointF(0,0));
        }
        else if (config->protoFeatureViewer == PVF_WS)
        {
            di.setBackgroundColor((TileBlack));
            di.setStartTile(QPointF(0,0));
            di.setDiameter(800.0);
        }
        break;

    case VIEW_DEL:
        di.setSizeF(viewDimensions[vtype].viewSize);
        di.setBackgroundColor((TileBlack));
        di.setStartTile(QPointF(0,0));
        di.setDiameter(800.0);
        break;

    case VIEW_FIGURE_MAKER:
    case VIEW_MAP_EDITOR:
        di.setSizeF(viewDimensions[vtype].viewSize);
        di.setBackgroundColor(config->figureViewBkgdColor);
        di.setStartTile(QPointF(0.0,0.0));
        di.setDiameter(200.0);
        break;

    case VIEW_TILING:
        di.setSizeF(viewDimensions[vtype].viewSize);
        di.setBackgroundColor((Qt::white));
        di.setDiameter(200.0);
        di.setStartTile(QPointF(0,0));
        break;

    case VIEW_TILIING_MAKER:
        di.setSizeF(viewDimensions[vtype].viewSize);
        di.setBackgroundColor((Qt::white));
        di.setStartTile(QPointF(0,0));
        di.setDiameter(200.0);
        break;

    case VIEW_FACE_SET:
        break;  // do nothing
    }
    canvas->setCanvasSettings(di);
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

    mViewers.push_back(tilingView);

    return tilingView;
}

TilingMaker * WorkspaceViewer::viewTilingMaker()
{
    qDebug() << "++WorkspaceViewer::viewTilingMaker";

    TilingMaker * tilingMaker  = TilingMaker::getInstance();

    mStyles.push_back(tilingMaker);  // add to non-deletable styles not viewers

    return tilingMaker;
}

PlacedDesignElementView * WorkspaceViewer::viewPlacedDesignElement(PlacedDesignElementPtr pde)
{
    qDebug() << "++WorkspaceViewer::viewDesignElement:" << pde->toString();

    PlacedDesignElementView * delView = new  PlacedDesignElementView(pde);

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

QTransform WorkspaceViewer::calculateViewTransform(SizeAndBounds & sab)
{
    Bounds b    = sab.viewBounds;
    QSizeF sz   = sab.viewSize;
    eViewType e = sab.viewType;

    qreal aspect = sz.width() / sz.height();
    qreal height = b.width / aspect;
    qreal scalex = sz.width()/b.width;

    QTransform first  = QTransform::fromTranslate(-b.left, - (b.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,(sz.width() - sz.height())/2.0);
    QTransform full   = first * second * third;

    qDebug().noquote() << sViewerType[e] << Transform::toInfoString(full);

#if 0
    ViewTransform vt;
    vt.viewType  = e;
    vt.scale     = Transform::scalex(full);
    vt.translate = Transform::trans(full);
#endif
    return full;
/*
    Results:
    VIEW_DESIGN         scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO          scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO_FEATURE  scale=75 rot=0 (0) trans=750 550
    VIEW_DEL            scale=75 rot=0 (0) trans=750 550
    VIEW_FIGURE_MAKER   scale=45 rot=0 (0) trans=450 450
    VIEW_TILING         scale=75 rot=0 (0) trans=750 550
    VIEW_TILIING_MAKER  scale=100 rot=0 (0)trans=500 500
    VIEW_MAP_EDITOR     scale=45 rot=0 (0) trans=450 450
*/
}

QTransform WorkspaceViewer::getViewTransform(eViewType e)
{
    return viewTransforms[e];
}

void WorkspaceViewer::setTitle(TilingPtr tp)
{
    if (!tp) return;

    QString str = QString("%1 : %2 : %3 : %4").arg(sViewerType[config->viewerType]).arg(tp->getName()).arg(tp->getDescription()).arg(tp->getAuthor());
    emit sig_title(str);
}
