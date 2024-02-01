#include <QCoreApplication>
#include <QMetaType>
#include <QSettings>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>

#include "viewers/view_controller.h"
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
#include "misc/sys.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/controlpanel.h"
#include "settings/canvas_settings.h"
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
#include "viewers/view.h"
#include "widgets/image_layer.h"
#include "tiledpatternmaker.h"

using std::make_shared;

ViewController::ViewController()
{
    mostRecentPrimeView = VIEW_MOSAIC;  // default
    theView             = nullptr;
}

void ViewController::init(View * view)
{
    theView         = view;

    config          = Configuration::getInstance();
    panel           = ControlPanel::getInstance();
    designMaker     = DesignMaker::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
    tilingMaker     = TilingMaker::getInstance();

    prototypeView   = PrototypeView::getInstance();
    motifView       = MotifView::getInstance();
    tilingMakerView = TilingMakerView::getInstance();
    mapedView       = MapEditorView::getInstance();
    bimageView      = BackgroundImageView::getInstance();
    gridView        = GridView::getInstance();
    borderView      = BorderView::getInstance();
    cropViewer      = CropViewer::getInstance();
    measureView     = MeasureView::getInstance();

    QStringList qsl = Configuration::getInstance()->viewColors;
    Q_ASSERT(qsl.size() == NUM_VIEW_TYPES);
}

ViewController::~ViewController()
{
    if (Sys::viewController == this && theView)
    {
        QList<int> currentEnables;
        for (const auto & v : enabledViewers)
        {
            int i = static_cast<int>(v);
            currentEnables << i;
        }

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
#endif
        QSettings s;
        s.setValue("viewEnables", QVariant::fromValue(currentEnables));

        TilingMakerView::releaseInstance();
        PrototypeView::releaseInstance();
        MotifView::releaseInstance();
        MapEditorView::releaseInstance();
        MeasureView::releaseInstance();
        GridView::releaseInstance();
        CropViewer::releaseInstance();
        BorderView::releaseInstance();
    }
}

void ViewController::slot_unloadView()
{
    // remove from scene but does not delete
    if (theView)
        theView->unloadView();
}

void ViewController::slot_unloadAll()
{
    if (!theView)  return;

    qDebug() << "ViewControl::slot_unloadAll";
    Sys::dumpRefs();

    theView->unloadView();
    Sys::dumpRefs();

    removeAllImages();
    Sys::dumpRefs();
    
    bimageView->unload();

    designMaker->unload();
    Sys::dumpRefs();

    MapEditor::getInstance()->unload();
    Sys::dumpRefs();

    mosaicMaker->resetMosaic();
    Sys::dumpRefs();

    prototypeMaker->unload();
    Sys::dumpRefs();

    tilingMaker->unload();
    Sys::dumpRefs();

    canvas.reInit();
    Sys::dumpRefs();

    slot_reconstructView();
    Sys::dumpRefs();

    qDebug() << "ViewControl::slot_unloadAll - complete";

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    tilingMaker->sm_takeUp(tiling,TILM_LOAD_EMPTY);
    Sys::dumpRefs();
    qDebug() << "ViewControl::slot_unloadAll - created empty tiling and mosaic";
}


void ViewController::viewEnable(eViewType view, bool enable)
{
    if (enable)
    {
        enabledViewers.push_back(view);
        setMostRecent(view);
    }
    else
    {
        enabledViewers.removeOne(view);
    }
}

void ViewController::disablePrimeViews()
{
    for (const auto & view : enabledViewers)
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
        case VIEW_MAP_EDITOR:
            enabledViewers.removeOne(view);
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

void ViewController::disableAllViews()
{
    enabledViewers.clear();
}

void ViewController::setMostRecent(eViewType viewType)
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
    case VIEW_MAP_EDITOR:
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

bool ViewController::isEnabled(eViewType view)
{
    return enabledViewers.contains(view);
}

void ViewController::slot_reconstructView()
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
            slot_reconstructView();
        }
    }
}

void ViewController::refreshView()
{
    if (!theView)
    {
        return;
    }

    qDebug().noquote() << "+ ViewController::refreshView mostRecentPrimeView=" << sViewerType[getMostRecent()];

    theView->unloadView();

    theView->setPaintEnable(false);

    // viewers
    setupEnabledViewLayers();

    // other images
    for (ImgLayerPtr & ilp : images)
    {
        theView->addLayer(ilp);
    }

    // crops
    if (cropViewer->getShowCrop())
    {
        theView->addLayer(cropViewer);
    }

    theView->setViewBackgroundColor(canvas.getBkgdColor());

    theView->setPaintEnable(true);
    theView->update(); // trigger repaint
}

void ViewController::setupEnabledViewLayers()
{
    for (auto view : std::as_const(enabledViewers))
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

void ViewController::viewDesign()
{
    if (!theView) return;

    QVector<DesignPtr> &  designs = designMaker->getActiveDesigns();
    for (auto design :  designs)
    {
        QVector<PatternPtr> & pats = design->getPatterns();
        for (PatternPtr & pat : pats)
        {
            theView->addLayer(pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        const CanvasSettings & modelSettings = dp->getDesignInfo();
        if (dp->border)
        {
            theView->addLayer(dp->border);
        }
        setBackgroundColor(VIEW_DESIGN,modelSettings.getBackgroundColor());
    }
}

void ViewController::viewMosaic()
{
    if (!theView) return;

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        QString name = mosaic->getName();
        qDebug() << "ViewController::viewMosaic" << name;

        QString astring = QString("Preparing Mosaic: %1").arg(name);
        theApp->splash(astring);
        panel->pushPanelStatus(name);

        mosaic->build();                // important

        theApp->removeSplash();
        panel->popPanelStatus();

        const StyleSet & sset = mosaic->getStyleSet();
        for (const StylePtr & style : std::as_const(sset))
        {
            qDebug().noquote() << "Adding Style:" << style->getDescription();
            if (canvas.getModelAlignment() != M_ALIGN_MOSAIC)
            {
                style->setModelXform(getCurrentModelXform(),false);
            }
            theView->addLayer(style);
        }

        auto border = mosaic->getBorder();
        if (border)
        {
            borderView->setBorder(border);
            theView->addLayer(borderView);
        }
    }
    else
    {
        qDebug() << "ViewController::viewMosaic - no mosaic";
    }
    
    const CanvasSettings & modelSettings = mosaicMaker->getCanvasSettings();
    setBackgroundColor(VIEW_MOSAIC,modelSettings.getBackgroundColor());
}

void ViewController::viewPrototype()
{
    if (!theView) return;

    qDebug() << "++ViewController::viewPrototype";

    theView->addLayer(prototypeView);
    
    setBackgroundColor(VIEW_PROTOTYPE);
}

void ViewController::viewMotifMaker()
{
    if (!theView) return;

    qDebug() << "++ViewController::viewMotifMaker";

    // dont set canvas xform
    theView->addLayer(motifView);
    
    setBackgroundColor(VIEW_MOTIF_MAKER);
}

void ViewController::viewTiling()
{
    if (!theView) return;

    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
        qDebug() << "++ViewController::viewTiling (tiling)"  << tiling->getTitle();

    if (canvas.getModelAlignment() != M_ALIGN_TILING)
    {
        tiling->setModelXform(getCurrentModelXform(),false);
    }
    theView->addLayer(tiling);

    setBackgroundColor(VIEW_TILING);
}

void ViewController::viewTilingMaker()
{
    if (!theView) return;

    qDebug() << "++ViewController::viewTilingMaker";

    if (canvas.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingMakerView->setModelXform(getCurrentModelXform(),false);
    }
    theView->addLayer(tilingMakerView);

    setBackgroundColor(VIEW_TILING_MAKER);
}

void ViewController::viewMapEditor()
{
    if (!theView) return;

    qDebug() << "++ViewController::viewMapEditor";

    mapedView->forceLayerRecalc(false);

    theView->addLayer(mapedView);
    
    setBackgroundColor(VIEW_MAP_EDITOR);
}

void ViewController::viewBackgroundImage()
{
    if (!theView) return;

    qDebug() << "++ViewController::viewBackgroundImage";

    theView->addLayer(bimageView);
}

void ViewController::viewGrid()
{
    if (!theView) return;

    theView->addLayer(gridView);

    setBackgroundColor(VIEW_GRID);
}

void ViewController::viewDebug()
{
    if (!theView) return;

    // measure
    if (Sys::measure)
    {
        theView->addLayer(measureView);
    }
    else
    {
        measureView->clear();
    }

    // big blue cross
    if (Sys::circleX)
    {
        MarkXPtr item = make_shared<MarkX>(theView->rect().center(), QPen(Qt::blue,5), QString("center"));
        item->setHuge();
        theView->addLayer(item);
    }
}

const Xform & ViewController::getCurrentModelXform()
{
    static const Xform unityXform;

    switch (canvas.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
    {
        MosaicPtr mosaic = mosaicMaker->getMosaic();
        if (mosaic)
        {
            StylePtr style = mosaic->getFirstStyle();
            if (style)
            {
                return style->getModelXform();
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
                return tiling->getModelXform();
            }
        }
        return unityXform;
    }
    default:
    case M_ALIGN_NONE:
        return unityXform;
    }
}

void ViewController::setCurrentModelXform(const Xform & xf, bool update)
{
    switch (canvas.getModelAlignment())
    {
    case M_ALIGN_MOSAIC:
        if (MosaicPtr mosaic = mosaicMaker->getMosaic())
        {
            for (const auto & style : mosaic->getStyleSet())
            {
                style->setModelXform(xf, update);
            }
            // FIXNE - should border be set too?
        }
        else
            qWarning() << "ViewControl::setCurrentXform - nothing to set";
        break;

    case M_ALIGN_TILING:
        if (tilingMaker->getTilings().size())
        {
            for (const auto & tiling : tilingMaker->getTilings())
            {
                tiling->setModelXform(xf,update);
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

void  ViewController::removeImage(ImageLayerView * img)
{
    for (ImgLayerPtr & ilp : images)
    {
        ImageLayerView * il = ilp.get();
        if (il == img)
        {
            images.removeAll(ilp);
            return;
        }
    }
}

QString ViewController::getBackgroundColor(eViewType vtype)
{
    return config->viewColors[vtype];
}

void  ViewController:: setBackgroundColor(eViewType vtype)
{
    canvas.setBkgdColor(config->viewColors[vtype]);
}

void  ViewController:: setBackgroundColor(eViewType vtype, QColor color)
{
    if (vtype == VIEW_MOSAIC)
    {
        MosaicPtr mosaic = mosaicMaker->getMosaic();

        if (mosaic)
        {
            CanvasSettings & modelSettings = mosaicMaker->getCanvasSettings();
            modelSettings.setBackgroundColor(color);
        }
    }
    config->viewColors[vtype] = color.name();
    canvas.setBkgdColor(color);
}
