#include <QCoreApplication>
#include <QMetaType>
#include <QSettings>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>

#include "viewers/viewcontrol.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "legacy/legacy_border.h"
#include "legacy/patterns.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/layer.h"
#include "misc/mark_x.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/controlpanel.h"
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
#include "tiledpatternmaker.h"

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

ViewControl::ViewControl() : View(this)
{
}

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
    bkgdImageView   = BackgroundImageView::getInstance();

    dontPaint = false;
    setMostRecent(VIEW_MOSAIC);     // default
    disablePrimeViews();
}

ViewControl::~ViewControl()
{
    QList<int> currentEnables;
    for (const auto & v : enabledViews)
    {
        int i = static_cast<int>(v);
        currentEnables << i;
    }

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
#endif

    QSettings s;
    s.setValue("viewEnables", QVariant::fromValue(currentEnables));

    BackgroundImageView::releaseInstance();
    TilingView::releaseInstance();
    TilingMakerView::releaseInstance();
    PrototypeView::releaseInstance();
    MotifView::releaseInstance();
    MapEditorView::releaseInstance();
    MeasureView::releaseInstance();
    GridView::releaseInstance();
    CropViewer::releaseInstance();
    BorderView::releaseInstance();
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

    viewSettings.reInit();
    dumpRefs();

    slot_refreshView();
    dumpRefs();

    qDebug() << "ViewControl::slot_unloadAll - complete";

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    tilingMaker->sm_takeUp(tiling,TILM_LOAD_EMPTY);
    dumpRefs();
    qDebug() << "ViewControl::slot_unloadAll - created empty tiling and mosaic";
}


void ViewControl::viewEnable(eViewType view, bool enable)
{
    if (enable)
    {
        enabledViews.push_back(view);
        setMostRecent(view);
    }
    else
    {
        enabledViews.removeOne(view);
    }
}

void ViewControl::disablePrimeViews()
{
    for (const auto & view : enabledViews)
    {
        switch (view)
        {
        // primary views
        case VIEW_DESIGN:
        case VIEW_MOSAIC:
        case VIEW_PROTOTYPE:
        case VIEW_MOTIF_MAKER:
        case VIEW_TILING:
        case VIEW_TILING_MAKER:
        case  VIEW_MAP_EDITOR:
            enabledViews.removeOne(view);
            break;

         // secondary views
        case VIEW_BKGD_IMG:
        case VIEW_GRID:
        case VIEW_BORDER:
        case VIEW_CROP:
        case VIEW_MEASURE:
        case VIEW_CENTER:
        case VIEW_IMAGE:
            // do nothing
            break;
        }
    }
}

void ViewControl::setMostRecent(eViewType viewType)
{
    switch (viewType)
    {
    // primary views
    case VIEW_DESIGN:
    case VIEW_MOSAIC:
    case VIEW_PROTOTYPE:
    case VIEW_MOTIF_MAKER:
    case VIEW_TILING:
    case VIEW_TILING_MAKER:
    case  VIEW_MAP_EDITOR:
        mostRecentPrimeView = viewType;
        break;

        // secondary views
    case VIEW_BKGD_IMG:
    case VIEW_GRID:
    case VIEW_BORDER:
    case VIEW_CROP:
    case VIEW_MEASURE:
    case VIEW_CENTER:
    case VIEW_IMAGE:
        // do nothing
        break;
    }
}

bool ViewControl::isEnabled(eViewType view)
{
    return enabledViews.contains(view);
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
    qDebug().noquote() << "+ ViewController::refreshView mostRecentPrimeView=" << sViewerType[getMostRecent()];

    paintEnable(false);

    unloadView();

    // viewers
    setupEnabledViewLayers();

    // other images
    for (ImgLayerPtr & ilp : images)
    {
        addLayer(ilp);
    }

    // resize
    QSize sz = viewSettings.getCropSize(mostRecentPrimeView);
    if (sz != size())
    {
        qDebug() << "ViewController::refreshView set size :" << sz;
        if (config->limitViewSize)
        {
            QScreen * pri = QGuiApplication::primaryScreen();
            QSize size = pri->availableSize();
            if (sz.width() > size.width())
            {
                sz.setWidth(size.width());
            }
            if (sz.height() > size.height())
            {
                sz.setHeight(size.height());
            }
        }
        resize(sz);
    }

    // crops
    if (cropViewer->getShowCrop())
    {
        addLayer(cropViewer);
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

void ViewControl::setupEnabledViewLayers()
{
    for (auto view : enabledViews)
    {
        switch (view)
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

        case VIEW_BKGD_IMG:
            viewBackgroundImage();
            break;

        case VIEW_GRID:
            viewGrid();
            break;

        case VIEW_MEASURE:
        case VIEW_CENTER:
            viewDebug();
            break;

        case VIEW_BORDER:
        case VIEW_CROP:
        case VIEW_IMAGE:
            // these do not have separate enable layer enables
            break;
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
        ModelSettings & modelSettings = dp->getDesignInfo();
        setViewBackgroundColor(modelSettings.getBackgroundColor());
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
        theApp->splash(astring);
        panel->pushPanelStatus(name);

        const StyleSet & sset = mosaic->getStyleSet();
        for (StylePtr style : sset)
        {
            qDebug().noquote() << "Adding Style:" << style.get() << "  " << style->getDescription();
            if (viewSettings.getModelAlignment() != M_ALIGN_MOSAIC)
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

        theApp->removeSplash();
        panel->popPanelStatus();
    }
    else
    {
        qDebug() << "ViewController::viewMosaic - no mosaic";
    }

    ModelSettings & modelSettings = mosaicMaker->getMosaicSettings();
    setViewBackgroundColor(modelSettings.getBackgroundColor());
}

void ViewControl::viewPrototype()
{
    qDebug() << "++ViewController::viewPrototype";

    addLayer(prototypeView);

    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_PROTOTYPE));
}

void ViewControl::viewMotifMaker()
{
    qDebug() << "++ViewController::viewMotifMaker";

    // dont set canvas xform
    addLayer(motifView);

    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_MOTIF_MAKER));
}

void ViewControl::viewTiling()
{
    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
        qDebug() << "++ViewController::viewTiling (tiling)"  << tiling->getName();

    tilingView->setTiling(tiling);
    if (viewSettings.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingView->setCanvasXform(getCurrentXform());
    }
    addLayer(tilingView);

    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_TILING));
}

void ViewControl::viewTilingMaker()
{
    qDebug() << "++ViewController::viewTilingMaker";

    if (viewSettings.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingMakerView->setCanvasXform(getCurrentXform());
    }
    addLayer(tilingMakerView);

    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_TILING_MAKER));
}

void ViewControl::viewMapEditor()
{
    qDebug() << "++ViewController::viewMapEditor";

    addLayer(mapedView);
    mapedView->forceLayerRecalc();

    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_MAP_EDITOR));
}

void ViewControl::viewBackgroundImage()
{
    if (bkgdImageView->isLoaded())
    {
        qDebug() << "adding image" << bkgdImageView->getName();
        addLayer(bkgdImageView);
    }
    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_BKGD_IMG));
}

void ViewControl::viewGrid()
{
    addLayer(gridView);
    setViewBackgroundColor(viewSettings.getBkgdColor(VIEW_GRID));
}

void ViewControl::viewDebug()
{
    // measure
    if (config->measure)
    {
        addLayer(measureView);
    }
    else
    {
        measureView->clear();
    }

    // big blue cross
    if (config->circleX)
    {
        MarkXPtr item = make_shared<MarkX>(rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        addLayer(item);
    }
}

const Xform & ViewControl::getCurrentXform()
{
    static const Xform unityXform;

    switch (viewSettings.getModelAlignment())
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
    switch (viewSettings.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
        if (MosaicPtr mosaic = mosaicMaker->getMosaic())
        {
            for (const auto & style : mosaic->getStyleSet())
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
            for (const auto & tiling : tilingMaker->getTilings())
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
