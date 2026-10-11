#include <QCoreApplication>
#include <QMetaType>
#include <QSettings>
#include <QDataStream>
#include <QVariant>
#include <QVariantList>
#include <QWindow>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_accessor.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/image_view.h"
#include "gui/viewers/layer.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
#include "gui/top/system_view.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "legacy/legacy_border.h"
#include "legacy/patterns.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/borders/border.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "model/settings/canvas.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////////////////////////////
///
///     ViewController
///
///////////////////////////////////////////////////////////////////////////////////////////////////

ViewController::ViewController() : ViewAccessor()
{
    rx_sigid = 0;
    canvas   = nullptr;

    _mostRecentGangEnable    = Layer::Type::Tiling;  // default - needed even if not true
}

ViewController::~ViewController()
{
    if (Sys::systemViewController == this && isAttached())
    {
        QList<int> currentEnables;
        for (auto & v : enabledViewers)
        {
            int i = static_cast<int>(v);
            currentEnables << i;
        }

        QSettings s = Sys::settings();
        s.setValue("viewEnables2", QVariant::fromValue(currentEnables));
    }

    slot_unloadView();
    unloadMakers();

    Sys::dumpRefs();
}

void ViewController::slot_updateView()
{
    updateView();
}

void ViewController::slot_unloadView()
{
    // always unloads so that paint does not fail
    unloadViewers();
}

void ViewController::slot_unloadAll()
{
    if (!isAttached()) return;

    qDebug() << "ViewControl::slot_unloadAll";

    unloadContent();
    unloadViewers();


    Sys::debugMapCreate->wipeout();
    Sys::debugMapPaint->wipeout();

    unloadMakers();

    Sys::dumpRefs();

    if (canvas)
        canvas->init();

    _systemModelXform = Xform();    // reset to unity
    emit sig_resetLayers();

    // there is always at least an emtpy tiling
    TilingPtr tiling = make_shared<Tiling>();
    TilingEvent tevent;
    tevent.event  = TilingMaker::Event::LoadEmpty;
    tevent.tiling = tiling;
    Sys::tilingMaker->sm_takeUp(tevent);

    slot_reconstructView();

    emit sig_unloaded();

    Sys::dumpRefs();
    qDebug() << "ViewControl::slot_unloadAll - created empty tiling and mosaic";
}

void ViewController::unloadMakers()
{
    qDebug() << "ViewControl::unloadMakers";
    Sys::dumpRefs();

    Sys::designMaker->unload();
    Sys::mapEditor->unload();
    Sys::mosaicMaker->resetMosaic();
    Sys::motifMaker->unload();
    Sys::tilingMaker->unload();

    Sys::dumpRefs();
    qDebug() << "ViewControl::unloadMakers - complete";
}

// Use with caution - the front door is ControlPanel::delegateView
void ViewController::viewEnable(Layer::Type view, bool enable)
{
    if (view == Layer::Type::MotifMaker)
    {
        Sys::motifMakerView->setLayerEnabled(enable);
    }

    if (view == Layer::Type::ConstructionLines && enable && Sys::tilingMakerView->getConstructionLines().isEmpty())
    {
        // covers startup, where the tiling/mosaic loads before the saved layer-enable
        // state is restored, so the load-on-tiling-load path above finds the view disabled
        Sys::tilingMaker->loadConstructionLines();
    }

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

bool ViewController::isGangMember(Layer::Type type)
{
    switch (type)
    {
    // radio/gang - members of gang are radio buttons
    // single selection
    case Layer::Type::LegacyDesign:
    case Layer::Type::Mosaic:
    case Layer::Type::Prototype:
    case Layer::Type::MotifMaker:
    case Layer::Type::Tiling:
    case Layer::Type::MapEditor:
    case Layer::Type::TilingMaker:
        return true;

    // independant / additive
    case Layer::Type::BkgdImage:
    case Layer::Type::Grid:
    case Layer::Type::ConstructionLines:
    case Layer::Type::Crop:
    case Layer::Type::DebugLayer:
    case Layer::Type::BMPImage:
        break;
    }
    return false;
}

QVector<Layer::Type> ViewController::getGangMembers()
{
    QVector<Layer::Type> vec;
    vec << Layer::Type::LegacyDesign;
    vec << Layer::Type::Mosaic;
    vec << Layer::Type::Prototype;
    vec << Layer::Type::MotifMaker;
    vec << Layer::Type::Tiling;
    vec << Layer::Type::MapEditor;
    vec << Layer::Type::TilingMaker;
    return vec;
}

uint ViewController::enabledGangCount()
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

void ViewController::disableGang()
{
    //qDebug() << "Disable gang";
    auto members = enabledViewers;     // copy - viewEnable() mutates enabledViewers
    for (auto & view : members)
    {
        if (isGangMember(view))
        {
            viewEnable(view,false);
        }
    }
}

bool ViewController::isRadioGangSelection(Layer::Type type)
{
    return (getMostRecentGangEnable() == type && enabledGangCount() == 1);
}

void ViewController::disableAllViews()
{
    enabledViewers.clear();
}

void ViewController::setMostRecentGangEnable(Layer::Type viewType)
{
    if (isGangMember(viewType))
    {
        _mostRecentGangEnable = viewType;
    }
}

void ViewController::setSelectedPrimaryLayer(LayerPtr layer)
{
    auto type = layer->type();

    //qDebug() << "SystemViewController::setSelectedPrimaryLayer" << sViewerType[type];

    if (layer != _selectedPrimaryLayer.lock() && (type == Layer::Type::Mosaic || type == Layer::Type::Tiling))
    {
        // select
        _selectedPrimaryLayer = layer;
        // copy xform to SMX
        auto xf = layer->getModelXform();
        setSystemModelXform(xf,Sys::nextSigid());
    }
}

bool ViewController::isEnabled(Layer::Type view)
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

void ViewController::reconstructView()
{
    if (isAttached() && Sys::isGuiThread())
    {
        qDebug().noquote() << "SystemViewController::reconstructView";
        setPaintDisable(true);
        unloadViewers();

        for (auto view : std::as_const(enabledViewers))
        {
            populateLayer(view);
        }

        taker->take();
        setPaintDisable(false);
        updateView();
    }
}

void ViewController::populateLayer(Layer::Type type)
{
    //qDebug().noquote() << "SystemViewController::populateLayer" << sViewerType[view];
    
    switch (type)
    {
    case Layer::Type::Mosaic:
    {
        MosaicPtr mosaic = Sys::mosaicMaker->getMosaic();
        QString name = mosaic->getName().get();
        //qDebug().noquote() << "Populating mosaic: " << name;
    
        if (!mosaic->isBuilt())
        {
            // build the mosaic
       
            QString astring    = QString("Preparing Mosaic: %1").arg(name);
            Sys::systemViewController->splash_display(astring);
            Sys::controlPanel->overridePagelStatus(astring);

            mosaic->build(); // important

            Sys::systemViewController->splash_remove();
            Sys::controlPanel->restorePageStatus();
        }

        // setup clip region
        setPainterCrop(mosaic->getPainterCrop());

        // display the mosaic
        const StyleSet & sset = mosaic->getStyleSet();
        for (const StylePtr & style : std::as_const(sset))
        {
            qDebug().noquote() << "Adding Style:" << style->getDescription();
            addLayer(style);
        }

        setBkgdColor(mosaic->getBkgdColor(),type);
    }   break;

    case Layer::Type::Prototype:
        addLayer(Sys::prototypeView);
        setBkgdColor(type);
        break;

    case Layer::Type::MotifMaker:
        addLayer(Sys::motifMakerView);
        setBkgdColor(type);
        break;

    case Layer::Type::Tiling:
        for (auto & tiling : Sys::tilingMaker->getTilings())
        {
            qDebug().noquote() << "Viewing tiling:" << tiling->getVName().get();
            addLayer(tiling);
            setBkgdColor(type);
        }
        break;

    case Layer::Type::TilingMaker:
        addLayer(Sys::tilingMakerView);
        setBkgdColor(type);
        break;

    case Layer::Type::MapEditor:
        Sys::mapEditorView->forceLayerRecalc(false);
        addLayer(Sys::mapEditorView);
        setBkgdColor(type);
        break;

    case Layer::Type::BkgdImage:
    {
        addLayer(Sys::backgroundImage);
        setBkgdColor(type);
    }   break;

    case Layer::Type::Grid:
        addLayer(Sys::gridViewer);
        break;

    case Layer::Type::ConstructionLines:
        addLayer(Sys::constructionLinesView);
        break;

    case Layer::Type::DebugLayer:
        addLayer(Sys::debugView);
        setBkgdColor(type);
        break;

    case Layer::Type::BMPImage:
        addLayer(Sys::imageViewer);
        setBkgdColor(type);
        break;

    case Layer::Type::Crop:
        addLayer(Sys::cropMakerView);
        break;

    case Layer::Type::LegacyDesign:
    {
        QVector<DesignPtr> &  designs = Sys::designMaker->getActiveDesigns();
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
            if (dp->border)
            {
                addLayer(dp->border);
            }
            setBkgdColor(type);
        }
    }   break;
    }
}

const Xform & ViewController::getSystemModelXform()
{
    return _systemModelXform;
}

void ViewController::setSystemModelXform(const Xform & xf, uint sigid)
{
    if (sigid <= rx_sigid)
    {
        //qDebug() << "discarded sigid" << sigid;
        return;
    }
    rx_sigid = sigid;

    _systemModelXform = xf;
}

QSize ViewController::viewSize()
{
    if (isAttached() && Sys::isGuiThread())
        return getTakerViewSize();
    else
        return canvas->getViewSize();
}

void ViewController::bringToPrimaryScreen()
{
    QScreen * primary = qApp->primaryScreen();

    QWindow * wh = windowHandle();
    if (wh)
    {
        wh->setScreen(primary);

        move(primary->geometry().center() - Sys::systemViewController->getViewRect().center());
        setWindowState((Sys::systemViewController->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        raiseView();  // for MacOS
        activateWindow();  // for windows
    }
}


