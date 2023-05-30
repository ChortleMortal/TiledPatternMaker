#include "viewers/viewcontrol.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "legacy/legacy_border.h"
#include "legacy/patterns.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/layer.h"
#include "misc/mark_x.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/panel.h"
#include "settings/model_settings.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "viewers/backgroundimageview.h"
#include "viewers/border_view.h"
#include "viewers/motif_view.h"
#include "viewers/grid_view.h"
#include "viewers/crop_view.h"
#include "viewers/map_editor_view.h"
#include "viewers/measure_view.h"
#include "viewers/prototype_view.h"
#include "viewers/tiling_view.h"
#include "viewers/view.h"
#include "widgets/image_layer.h"

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

ViewControl::ViewControl() : View()
{}

void ViewControl::init()
{
    View::init();

    config          = Configuration::getInstance();
    panel           = ControlPanel::getInstance();
    designMaker     = DesignMaker::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();

    prototypeView   = PrototypeView::getInstance();
    motifView       = MotifView::getInstance();
    tilingView      = TilingView::getInstance();
    tilingMakerView = TilingMakerView::getInstance();
    mapedView       = MapEditorView::getInstance();
    gridView        = GridView::getInstance();
    measureView     = MeasureView::getInstance();
    cropViewer      = CropViewer::getInstance();
    borderView      = BorderView::getInstance();

    dontPaint = false;
    disableAllViews();
    viewEnable(config->getViewerType(),true);
}

ViewControl::~ViewControl()
{
    BackgroundImageView::releaseInstance();
    TilingView::releaseInstance();
    TilingMakerView::releaseInstance();
    PrototypeView::releaseInstance();
    MotifView::releaseInstance();
    MapEditorView::releaseInstance();
    MeasureView::releaseInstance();
    GridView::releaseInstance();
    CropViewer::releaseInstance();
    BorderView::getInstance();
}

void ViewControl::slot_unloadView()
{
    // remove from scene but does not delete
    unloadView();
}

void ViewControl::slot_dontPaint(bool dont)
{
    qDebug() << "dont paint" << dont;
    dontPaint = dont;
}

void ViewControl::slot_unloadAll()
{
    emit sig_identifyYourself();

    qDebug() << "ViewControl::slot_unloadAll";
    dumpRefs();

    unloadView();
    dumpRefs();

    removeAllImages();
    dumpRefs();
    
    auto bip = BackgroundImageView::getInstance();
    bip->unload();

    designMaker->unload();
    dumpRefs();

    MapEditor::getInstance()->unload();
    dumpRefs();

    mosaicMaker->resetMosaic();
    dumpRefs();

    prototypeMaker->unload();
    dumpRefs();

    tilingMaker->unload();
    dumpRefs();

    frameSettings.reInit();
    dumpRefs();

    emit sig_identifyYourself();

    slot_refreshView();
    dumpRefs();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    tilingMaker->sm_takeUp(tiling,TILM_LOAD_EMPTY);

    emit sig_identifyYourself();
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
    update();
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

    paintEnable(false);

    unloadView();

    // viewers
    setupViewers();

    // background image
    auto bip = BackgroundImageView::getInstance();
    {
        if (bip->isLoaded())
        {
            qDebug() << "adding image" << bip->getName();
            addLayer(bip);
        }
    }

    // other images
    for (ImgLayerPtr & ilp : images)
    {
        addLayer(ilp);
    }

    // grid
    if (config->showGrid)
    {
        if (config->gridUnits == GRID_UNITS_TILE)
        {
            gridView->setCanvasXform(getCurrentXform());
        }
        else
        {
            gridView->setCanvasXform(unityXform);
        }

        addLayer(gridView);
    }

    if (config->measure)
    {
        measureView->setMeasureMode(true);
        addLayer(measureView);
    }
    else
    {
        measureView->setMeasureMode(false);
    }

    // resize
    QSize sz = frameSettings.getCropSize(config->getViewerType());
    if (sz != size())
    {
        qDebug() << "ViewControl set size :" << sz;
        QWidget::resize(sz);
    }

    // crops
#if 0
    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        auto crop = mosaic->getCrop();
        if (crop)
        {
            addLayer(cropViewer);
        }
    }
#else
    if (cropViewer->getShowCrop())
    {
        addLayer(cropViewer);
    }
#endif

    // big blue cross
    if (config->circleX)
    {
        MarkXPtr item = make_shared<MarkX>(rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        addLayer(item);
    }

    if (!dontPaint)
    {
        // trigger repaint
        paintEnable(true);
        update();
        emit sig_viewUpdated();
    }
    else
    {
        qWarning() << "Painting disabled";
    }
}

void ViewControl::setupViewers()
{
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
    QVector<DesignPtr> &  designs = designMaker->getActiveDesigns();
    for (auto design :  designs)
    {
        QVector<PatternPtr> & pats = design->getPatterns();
        for (PatternPtr & pat : pats)
        {
            addLayer(pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        ModelSettings & settings = dp->getDesignInfo();
        setBackgroundColor(settings.getBackgroundColor());
        if (dp->border)
        {
            addLayer(dp->border);
        }
    }
}

void ViewControl::viewMosaic()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        QString name = mosaic->getName();
        qDebug() << "ViewController::viewMosaic" << name;

        QString astring = QString("Preparing Mosaic: %1").arg(name);
        panel->splashMosiac(astring);
        panel->pushPanelStatus(name);

        const StyleSet & sset = mosaic->getStyleSet();
        for (StylePtr style : sset)
        {
            qDebug().noquote() << "Adding Style:" << style.get() << "  " << style->getDescription();
            if (frameSettings.getModelAlignment() != M_ALIGN_MOSAIC)
            {
                style->setCanvasXform(getCurrentXform());
            }
            style->createStyleRepresentation();   // important to do this here
            addLayer(style);
        }

        auto border = mosaic->getBorder();
        if (border)
        {
            borderView->setBorder(border);
            addLayer(borderView);
        }

        panel->removeSplashMosaic();
        panel->popPanelStatus();
    }
    else
    {
        qDebug() << "ViewController::viewMosaic - no mosaic";
    }

    ModelSettings & settings = mosaicMaker->getMosaicSettings();
    setBackgroundColor(settings.getBackgroundColor());
}

void ViewControl::viewPrototype()
{
    qDebug() << "++ViewController::viewPrototype";

    addLayer(prototypeView);

    setBackgroundColor(Qt::white);
}

void ViewControl::viewMotifMaker()
{
    qDebug() << "++ViewController::viewMotifMaker";

    // dont set canvas xform
    addLayer(motifView);

   if (config->motifBkgdWhite)
        setBackgroundColor(QColor(Qt::white));
    else
        setBackgroundColor(QColor(Qt::black));
}

void ViewControl::viewTiling()
{
    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
        qDebug() << "++ViewController::viewTiling (tiling)"  << tiling->getName();

    tilingView->setTiling(tiling);
    if (frameSettings.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingView->setCanvasXform(getCurrentXform());
    }
    addLayer(tilingView);

    setBackgroundColor(Qt::white);
}

void ViewControl::viewTilingMaker()
{
    qDebug() << "++ViewController::viewTilingMaker";

    if (frameSettings.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingMakerView->setCanvasXform(getCurrentXform());
    }
    addLayer(tilingMakerView);

    setBackgroundColor(Qt::white);
}

void ViewControl::viewMapEditor()
{
    qDebug() << "++ViewController::viewMapEditor";

    addLayer(mapedView);
    mapedView->forceLayerRecalc();

    if (config->motifBkgdWhite)
        setBackgroundColor(QColor(Qt::white));
    else
        setBackgroundColor(QColor(Qt::black));
}

const Xform & ViewControl::getCurrentXform()
{
    switch (frameSettings.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
    {
        MosaicPtr mosaic = mosaicMaker->getMosaic();
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
        auto tilings = tilingMaker->getTilings();
        if (tilings.count() > 0)
        {
            TilingPtr tiling = tilings.first();
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

void ViewControl::setCurrentXform(const Xform & xf)
{
    switch (frameSettings.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
        if (MosaicPtr mosaic = mosaicMaker->getMosaic())
        {
            for (auto & style : mosaic->getStyleSet())
            {
                 style->setCanvasXform(xf);
            }
        }
        else
            qWarning() << "ViewControl::setCurrentXform - nothing to set";
        break;

    case M_ALIGN_TILING:
        if (tilingMaker->getTilings().size())
        {
            for (auto tiling : tilingMaker->getTilings())
            {
                tiling->setCanvasXform(xf);
            }
        }
        else
            qWarning() << "ViewControl::setCurrentXform - nothing to set";
        break;

    default:
    case M_ALIGN_NONE:
        qWarning() << "ViewControl::setCurrentXform - no alignment";
        break;
    }
}

void  ViewControl::removeImage(ImageLayer * img)
{
    for (ImgLayerPtr & ilp : images)
    {
        ImageLayer * il = ilp.get();
        if (il == img)
        {
            images.removeAll(ilp);
            return;
        }
    }
}
