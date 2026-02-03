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
#include "gui/top/system_view_accessor.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/image_view.h"
#include "gui/viewers/layer.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
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
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// SystemViewController
///
///////////////////////////////////////////////////////////////////////////////////////////////////

SystemViewController::SystemViewController()
{
    theView  = nullptr;
    rx_sigid = 0;

    _mostRecentGangEnable    = VIEW_TILING;  // default - needed even if not true

    QStringList qsl = Sys::config->viewColors;
    Q_ASSERT(qsl.size() == NUM_VIEW_TYPES);
}

void SystemViewController::attach(SystemView * view)
{
    theView = view;
}

SystemViewController::~SystemViewController()
{
    if (Sys::viewController == this && theView)
    {
        QList<int> currentEnables;
        for (auto & v : enabledViewers)
        {
            int i = static_cast<int>(v);
            currentEnables << i;
        }

        QSettings s;
        s.setValue("viewEnables2", QVariant::fromValue(currentEnables));
    }

    slot_unloadView();
    unloadMakers();
    Sys::dumpRefs();
}

void SystemViewController::slot_updateView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->updateView();
    }
}

void SystemViewController::slot_unloadView()
{
    // always unloads so that paint does not fail
    if (theView && Sys::isGuiThread())
    {
        theView->unloadViewers();
    }
}

void SystemViewController::slot_unloadAll()
{
    if (!theView) return;

    qDebug() << "ViewControl::slot_unloadAll";

    if (theView && Sys::isGuiThread())
    {
        theView->unloadLayerContent();
        theView->unloadViewers();
    }

    Sys::debugMapCreate->wipeout();
    Sys::debugMapPaint->wipeout();

    unloadMakers();

    Sys::dumpRefs();

    canvas.setDefaultSize();

    _systemModelXform = Xform();    // reset to unity
    emit sig_resetLayers();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    TilingEvent tevent;
    tevent.event  = TILM_LOAD_EMPTY;
    tevent.tiling = tiling;
    Sys::tilingMaker->sm_takeUp(tevent);

    slot_reconstructView();

    emit sig_unloaded();

    Sys::dumpRefs();
    qDebug() << "ViewControl::slot_unloadAll - created empty tiling and mosaic";
}

void SystemViewController::unloadMakers()
{
    qDebug() << "ViewControl::unloadMakers";
    Sys::dumpRefs();

    Sys::designMaker->unload();
    Sys::mapEditor->unload();
    Sys::mosaicMaker->resetMosaic();
    Sys::prototypeMaker->unload();
    Sys::tilingMaker->unload();

    Sys::dumpRefs();
    qDebug() << "ViewControl::unloadMakers - complete";
}

// Use with caution - the front door is ControlPanel::delegateView
void SystemViewController::viewEnable(eViewType view, bool enable)
{
    if (enable)
    {
        //qDebug() << "Enable"  << sViewerType[view];
        enabledViewers.push_back(view);
        setMostRecentGangEnable(view);
    }
    else
    {
        //qDebug() << "Disable"  << sViewerType[view];
        enabledViewers.removeOne(view);
    }
}

bool SystemViewController::isGangMember(eViewType viewType)
{
    switch (viewType)
    {
    // radio/gang - members of gang are radio buttons
    // single selection
    case VIEW_LEGACY:
    case VIEW_MOSAIC:
    case VIEW_PROTOTYPE:
    case VIEW_MOTIF_MAKER:
    case VIEW_TILING:
    case VIEW_MAP_EDITOR:
    case VIEW_TILING_MAKER:
        return true;

    // independant / additive
    case VIEW_BKGD_IMG:
    case VIEW_GRID:
    case VIEW_CROP:
    case VIEW_DEBUG:
    case VIEW_BMP_IMAGE:
        break;
    }
    return false;
}

QVector<eViewType> SystemViewController::getGangMembers()
{
    QVector<eViewType> vec;
    vec << VIEW_LEGACY;
    vec << VIEW_MOSAIC;
    vec << VIEW_PROTOTYPE;
    vec << VIEW_MOTIF_MAKER;
    vec << VIEW_TILING;
    vec << VIEW_MAP_EDITOR;
    vec << VIEW_TILING_MAKER;
    return vec;
}

uint SystemViewController::enabledGangCount()
{
    uint count = 0;
    for (auto & view : enabledViewers)
    {
        if (isGangMember(view))
        {
            count++;
        }
    }
    return count;
}

void SystemViewController::disableGang()
{
    //qDebug() << "Disable gang";
    for (auto & view : enabledViewers)
    {
        if (isGangMember(view))
        {
            enabledViewers.removeOne(view);
        }
    }
}

bool SystemViewController::isRadioGangSelection(eViewType vtype)
{
    return (getMostRecentGangEnable() == vtype && enabledGangCount() == 1);
}

void SystemViewController::disableAllViews()
{
    enabledViewers.clear();
}

void SystemViewController::setMostRecentGangEnable(eViewType viewType)
{
    if (isGangMember(viewType))
    {
        _mostRecentGangEnable = viewType;
    }
}

void SystemViewController::setSelectedPrimaryLayer(LayerPtr layer)
{
    auto type = layer->viewType();

    //qDebug() << "SystemViewController::setSelectedPrimaryLayer" << sViewerType[type];

    if (layer != _selectedPrimaryLayer.lock() && (type == VIEW_MOSAIC || type == VIEW_TILING))
    {
        // select
        _selectedPrimaryLayer = layer;
        // copy xform to SMX
        auto xf = layer->getModelXform();
        setSystemModelXform(xf,Sys::nextSigid());
    }
}

bool SystemViewController::isEnabled(eViewType view)
{
    return enabledViewers.contains(view);
}


void SystemViewController::slot_reconstructView()
{
    // prevent re-entrant call from the same thread
    static volatile bool busy   = false;
    static volatile bool review = false;

    if (busy)
    {
        qDebug() <<  "SystemViewController::slot_reconstructView" << "- BUSY IGNORED" ;
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
            qDebug() << "SystemViewController::slot_reconstructView" << "RE-VIEW";
            slot_reconstructView();
        }
    }
}

void SystemViewController::reconstructView()
{
    if (theView && Sys::isGuiThread())
    {
        qDebug().noquote() << "SystemViewController::reconstructView";
        theView->setPaintDisable(true);
        theView->unloadViewers();

        for (auto view : std::as_const(enabledViewers))
        {
            enableLayer(view);
        }

        theView->setBackgroundColor(canvas.getBkgdColor());
        theView->setPaintDisable(false);
        theView->update();
    }
}

void SystemViewController::enableLayer(eViewType view)
{
    //qDebug().noquote() << "SystemViewController::enableLayer" << sViewerType[view];

    MosaicPtr mosaic = Sys::mosaicMaker->getMosaic();
    if (!mosaic->isBuilt())
    {
        // build the mosaic
        QString name = mosaic->getName().get();
        QString astring    = QString("Preparing Mosaic: %1").arg(name);
        Sys::splash->display(astring,true);

        mosaic->build(); // important

        Sys::splash->remove(true);
    }

    switch (view)
    {
    case VIEW_MOSAIC:
    {
        // setup clip region
        theView->setPainterCrop(mosaic->getPainterCrop());

        // display the mosaic
        const StyleSet & sset = mosaic->getStyleSet();
        for (const StylePtr & style : std::as_const(sset))
        {
            qDebug().noquote() << "Adding Style:" << style->getDescription();
            theView->addLayer(style);
        }

        const CanvasSettings & modelSettings = Sys::mosaicMaker->getCanvasSettings();
        setBackgroundColor(VIEW_MOSAIC,modelSettings.getBackgroundColor());
    }   break;

    case VIEW_PROTOTYPE:
        theView->addLayer(Sys::prototypeView);
        setBackgroundColor(VIEW_PROTOTYPE);
        break;

    case VIEW_MOTIF_MAKER:
        theView->addLayer(Sys::motifMakerView);
        setBackgroundColor(VIEW_MOTIF_MAKER);
        break;

    case VIEW_TILING:
        for (auto & tiling : Sys::tilingMaker->getTilings())
        {
            qDebug().noquote() << "Viewing tiling:" << tiling->getVName().get();
            theView->addLayer(tiling);
        }
        setBackgroundColor(VIEW_TILING);
        break;

    case VIEW_TILING_MAKER:
        theView->addLayer(Sys::tilingMakerView);
        setBackgroundColor(VIEW_TILING_MAKER);
        break;

    case VIEW_MAP_EDITOR:
        Sys::mapEditorView->forceLayerRecalc(false);    // FIXME is this still needed
        theView->addLayer(Sys::mapEditorView);
        setBackgroundColor(VIEW_MAP_EDITOR);
        break;

    case VIEW_BKGD_IMG:
    {
        BkgdImagePtr bip = Sys::getBackgroundImageFromSource();
        theView->addLayer(bip);
    }   break;

    case VIEW_GRID:
        theView->addLayer(Sys::gridViewer);
        setBackgroundColor(VIEW_GRID);
        break;

    case VIEW_DEBUG:
        theView->addLayer(Sys::debugView);
        break;

    case VIEW_BMP_IMAGE:
        theView->addLayer(Sys::imageViewer);
        break;

    case VIEW_CROP:
        theView->addLayer(Sys::cropMakerView);
        break;

    case VIEW_LEGACY:
    {
        QVector<DesignPtr> &  designs = Sys::designMaker->getActiveDesigns();
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
            setBackgroundColor(VIEW_LEGACY,modelSettings.getBackgroundColor());
        }
    }   break;

    }
}

const Xform & SystemViewController::getSystemModelXform()
{
    return _systemModelXform;
}

void SystemViewController::setSystemModelXform(const Xform & xf, uint sigid)
{
    if (sigid <= rx_sigid)
    {
        //qDebug() << "discarded sigid" << sigid;
        return;
    }
    rx_sigid = sigid;

    _systemModelXform = xf;
}

QString SystemViewController::getBackgroundColor(eViewType vtype)
{
    return Sys::config->viewColors[vtype];
}

void  SystemViewController:: setBackgroundColor(eViewType vtype)
{
    canvas.setBkgdColor(Sys::config->viewColors[vtype]);
}

void  SystemViewController:: setBackgroundColor(eViewType vtype, QColor color)
{
    if (vtype == VIEW_MOSAIC)
    {
        MosaicPtr mosaic = Sys::mosaicMaker->getMosaic();

        if (mosaic)
        {
            CanvasSettings & modelSettings = Sys::mosaicMaker->getCanvasSettings();
            modelSettings.setBackgroundColor(color);
        }
    }
    Sys::config->viewColors[vtype] = color.name();
    canvas.setBkgdColor(color);
}

QSize SystemViewController::viewSize()
{
    if (theView && Sys::isGuiThread())
        return theView->size();
    else
        return getCanvasViewSize();
}
