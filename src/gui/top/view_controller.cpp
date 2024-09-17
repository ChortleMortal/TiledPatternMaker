#include <QCoreApplication>
#include <QMetaType>
#include <QSettings>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/backgroundimageview.h"
#include "gui/viewers/crop_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/layer.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/mark_x.h"
#include "gui/viewers/measure_view.h"
#include "gui/viewers/motif_view.h"
#include "gui/viewers/prototype_view.h"
#include "gui/widgets/image_layer.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "legacy/legacy_border.h"
#include "legacy/patterns.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/canvas_settings.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////////////////////////////
// This is C code to bypass protection in view class (blame GPT for this stragegy)
///////////////////////////////////////////////////////////////////////////////////////////////////

void  setPaintDisable(View * view, bool disable){ if (view) view->setPaintDisable(disable); }
void  addLayer(View * view, LayerPtr layer)     { if (view) view->addLayer(layer); }
void  addLayer(View * view, Layer  * layer)     { if (view) view->addLayer(layer); }
void  unloadView(View * view)                   { if (view) view->unloadView(); }
void  setClip(View * view, CropPtr crop)        { if (view) view->setClip(crop); }
void  setFixedSize(View * view, QSize size)     { if (view) view->setFixedSize(size); }
void  setSize(View * view, QSize size)          { if (view) view->setSize(size); }
void  clearLayout(View* view)                   { if (view) view->clearLayout(); }
void  setViewBackgroundColor(View * view,QColor c) { if (view) view->setViewBackgroundColor(c); }
bool  viewCanPaint(View * view)                 { if (view) return view->viewCanPaint(); else return false; }
bool  splashCanPaint(View * view)               { if (view) return view->splashCanPaint(); else return false; }

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// ViewController
///
///////////////////////////////////////////////////////////////////////////////////////////////////

ViewController::ViewController()
{
    mostRecentPrimeView = VIEW_MOSAIC;  // default
    theView             = nullptr;
}

void ViewController::init(View * view)
{
    theView         = view;

    config          = Sys::config;
    panel           = Sys::controlPanel;
    designMaker     = Sys::designMaker;
    mosaicMaker     = Sys::mosaicMaker;
    prototypeMaker  = Sys::prototypeMaker;
    tilingMaker     = Sys::tilingMaker;

    prototypeView   = Sys::prototypeView;
    motifView       = Sys::motifView;
    tilingMakerView = Sys::tilingMakerView;
    mapedView       = Sys::mapEditorView;
    bimageView      = Sys::backgroundImageView;
    gridView        = Sys::gridViewer;
    measureView     = Sys::measureView;

    QStringList qsl = Sys::config->viewColors;
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
    }

    slot_unloadView();
    unloadMakers();
    Sys::dumpRefs();
}

void ViewController::slot_unloadView()
{
    // always unloads so that paint does not fail
    if (theView)
    {
        ::unloadView(theView);
    }
}

void  ViewController::setSize(QSize sz)
{
    ::setSize(theView,sz);
}

void  ViewController::setFixedSize(QSize sz)
{
    ::setFixedSize(theView,sz);
}

void ViewController::clearLayout()
{
    ::clearLayout(theView);
}

void ViewController::slot_unloadAll()
{
    if (!theView) return;

    qDebug() << "ViewControl::slot_unloadAll";

    ::unloadView(theView);

    unloadMakers();

    Sys::dumpRefs();

    canvas.reInit();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    tilingMaker->sm_takeUp(tiling,TILM_LOAD_EMPTY);

    slot_reconstructView();

    Sys::dumpRefs();
    qDebug() << "ViewControl::slot_unloadAll - created empty tiling and mosaic";
}

void ViewController::unloadMakers()
{
    qDebug() << "ViewControl::unloadMakers";
    Sys::dumpRefs();

    removeAllImages();
    
    designMaker->unload();

    Sys::mapEditor->unload();

    mosaicMaker->resetMosaic();

    prototypeMaker->unload();

    tilingMaker->unload();

    Sys::dumpRefs();
    qDebug() << "ViewControl::unloadMakers - complete";
}

// Use with caution - the front door is ControlPanel::delegateView
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

bool ViewController::isPrimaryView(eViewType viewType)
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
        return true;

    // secondary views
    default:
    case VIEW_BKGD_IMG:
    case VIEW_GRID:
    case VIEW_BORDER:
    case VIEW_CROP:
    case VIEW_MEASURE:
    case VIEW_CENTER:
    case VIEW_IMAGE:
        return false;
    }
}

uint ViewController::enabledPrimaryViews()
{
    uint count = 0;
    for (const auto & view : enabledViewers)
    {
        if (isPrimaryView(view))
        {
            count++;
        }
    }
    return count;
}

void ViewController::disablePrimeViews()
{
    for (const auto & view : enabledViewers)
    {
        if (isPrimaryView(view))
        {
            enabledViewers.removeOne(view);
        }
    }
}

void ViewController::disableAllViews()
{
    enabledViewers.clear();
}

void ViewController::setMostRecent(eViewType viewType)
{
    if (isPrimaryView(viewType))
    {
        mostRecentPrimeView = viewType;
    }
}

bool ViewController::isEnabled(eViewType view)
{
    return enabledViewers.contains(view);
}

bool ViewController::viewCanPaint()
{
    return ::viewCanPaint(theView);
}

bool ViewController::splashCanPaint()
{
    return ::splashCanPaint(theView);
}

void ViewController::slot_reconstructView()
{
    // prevent re-entrant call from the same thread
    static volatile bool busy   = false;
    static volatile bool review = false;

    if (busy)
    {
        qDebug() <<  __FUNCTION__ << "- BUSY IGNORED" ;
        review = true;
    }
    else
    {
        busy = true;
        reconstructView();
        busy = false;

        if (review)
        {
            review = false;
            qDebug() << __FUNCTION__ << "RE-VIEW";
            slot_reconstructView();
        }
    }
}

void ViewController::reconstructView()
{
    if (!theView)
    {
        return;
    }

    qDebug().noquote() << __FUNCTION__ << "mostRecentPrimeView=" << sViewerType[getMostRecent()];

    ::unloadView(theView);

    ::setPaintDisable(theView,true);

    // viewers
    setupEnabledViewLayers();

    // crops
    if (Sys::cropViewer->getShowCrop(CM_MOSAIC) || Sys::cropViewer->getShowCrop(CM_PAINTER) || Sys::cropViewer->getShowCrop(CM_MAPED))
    {
        ::addLayer(theView,Sys::cropViewer);
    }

    // debug view
    if (Sys::debugView->getShow())
    {
        ::addLayer(theView,Sys::debugView);
    }

    ::setViewBackgroundColor(theView, canvas.getBkgdColor());

    ::setPaintDisable(theView,false);

    theView->slot_update();
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

        case VIEW_IMAGE:
            viewBMPImages();
            break;

        case VIEW_BORDER:
        case VIEW_CROP:
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
            ::addLayer(theView,pat);
        }
    }

    if (designs.count())
    {
        DesignPtr dp = designs.first();
        const CanvasSettings & modelSettings = dp->getDesignInfo();
        if (dp->border)
        {
            ::addLayer(theView,dp->border);
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
        QString name = mosaic->getName().get();
        qDebug() << __FUNCTION__ << name;

        QString saveString = panel->getStatus();
        QString astring    = QString("Preparing Mosaic: %1").arg(name);
        Sys::splash->display(astring);
        panel->setStatus(name);

        mosaic->build();                // important

        Sys::splash->remove();
        panel->setStatus(saveString);

        auto crop = mosaic->getPainterCrop();
        if (crop && crop->getClip())
        {
            ::setClip(theView, crop);     // causes view to set clip region
        }

        const StyleSet & sset = mosaic->getStyleSet();
        for (const StylePtr & style : std::as_const(sset))
        {
            qDebug().noquote() << "Adding Style:" << style->getDescription();
            if (canvas.getModelAlignment() != M_ALIGN_MOSAIC)
                style->setModelXform(getCurrentModelXform(),false);
            ::addLayer(theView,style);
        }

        auto border = mosaic->getBorder();
        if (border)
        {
            qDebug().noquote() << "Adding Style:" << border->getDescription();
            if (canvas.getModelAlignment() != M_ALIGN_MOSAIC)
                border->setModelXform(getCurrentModelXform(),false);
            ::addLayer(theView,border);
        }
    }
    else
    {
        qDebug() << __FUNCTION__ << "- no mosaic";
    }
    
    const CanvasSettings & modelSettings = mosaicMaker->getCanvasSettings();
    setBackgroundColor(VIEW_MOSAIC,modelSettings.getBackgroundColor());
}

void ViewController::viewPrototype()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    ::addLayer(theView,prototypeView);
    
    setBackgroundColor(VIEW_PROTOTYPE);
}

void ViewController::viewMotifMaker()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    // dont set canvas xform
    ::addLayer(theView,motifView);
    
    setBackgroundColor(VIEW_MOTIF_MAKER);
}

void ViewController::viewTiling()
{
    if (!theView) return;

    TilingPtr tiling = tilingMaker->getSelected();

    if (tiling)
        qDebug() << __FUNCTION__ << tiling->getName().get();

    if (canvas.getModelAlignment() != M_ALIGN_TILING)
    {
        tiling->setModelXform(getCurrentModelXform(),false);
    }

    ::addLayer(theView,tiling);

    setBackgroundColor(VIEW_TILING);
}

void ViewController::viewTilingMaker()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    if (canvas.getModelAlignment() != M_ALIGN_TILING)
    {
        tilingMakerView->setModelXform(getCurrentModelXform(),false);
    }

    ::addLayer(theView,tilingMakerView);

    setBackgroundColor(VIEW_TILING_MAKER);
}

void ViewController::viewMapEditor()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    mapedView->forceLayerRecalc(false);

    ::addLayer(theView,mapedView);
    
    setBackgroundColor(VIEW_MAP_EDITOR);
}

void ViewController::viewBackgroundImage()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    ::addLayer(theView,bimageView);
}

void ViewController::viewBMPImages()
{
    if (!theView) return;

    qDebug() << __FUNCTION__;

    // other images
    for (ImgLayerPtr & ilp : images)
    {
        ::addLayer(theView,ilp);
    }
}

void ViewController::viewGrid()
{
    if (!theView) return;

    ::addLayer(theView,gridView);

    setBackgroundColor(VIEW_GRID);

    gridView->setModelXform(getCurrentModelXform(),false);
}

void ViewController::viewDebug()
{
    if (!theView) return;

    // measure
    if (Sys::measure)
    {
        ::addLayer(theView,measureView);
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
        ::addLayer(theView,item);
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
            auto border = mosaic->getBorder();
            if (border)
            {
                border->setModelXform(xf, update);
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

void ViewController::procImgViewKey(QKeyEvent * k)
{
    for (ImgLayerPtr & ilp : images)
    {
        emit ilp->sig_keyPressed(k);
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
