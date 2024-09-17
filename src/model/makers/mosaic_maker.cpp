#include "model/makers/mosaic_maker.h"
#include "sys/engine/image_engine.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "sys/sys/load_unit.h"
#include "sys/sys.h"
#include "gui/map_editor/map_editor.h"
#include "model/prototypes/prototype.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/styles/emboss.h"
#include "model/styles/filled.h"
#include "model/styles/interlace.h"
#include "model/styles/outline.h"
#include "model/styles/plain.h"
#include "model/styles/sketch.h"
#include "model/styles/thick.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/tiling.h"
#include "model/tilings/backgroundimage.h"
#include "gui/viewers/backgroundimageview.h"
#include "gui/viewers/debug_view.h"
#include "gui/top/view_controller.h"
#include "gui/top/view.h"

using std::make_shared;

MosaicMaker::MosaicMaker()
{
    _mosaic = make_shared<Mosaic>();
}

MosaicMaker::~MosaicMaker()
{
    _mosaic.reset();
}

void MosaicMaker::init()
{
    prototypeMaker = Sys::prototypeMaker;
    config         = Sys::config;
    controlPanel   = Sys::controlPanel;
    viewControl    = Sys::viewController;
    tilingMaker    = Sys::tilingMaker;

    connect(this, &MosaicMaker::sig_updateView,      Sys::view,   &View::slot_update);
    connect(this, &MosaicMaker::sig_reconstructView, viewControl, &ViewController::slot_reconstructView);
}

// This the most complete load of a Mosaic - it does everything needed
// and notifies other pages with sig_mosaicLoaded
MosaicPtr MosaicMaker::loadMosaic(VersionedFile &file)
{
    qDebug().noquote() << "MosaicMaker::loadMosaic" << file.getVersionedName().get();

    Sys::debugView->clear();

    Sys::loadUnit->setLoadState(LOADING_MOSAIC,file);

    // the advantage of using the manager is that it does not persist
    MosaicPtr mosaic;
    MosaicManager mm;
    mosaic = mm.loadMosaic(file);
    if (mosaic)
    {
        viewControl->removeAllImages();

        // starts the chain reaction
        sm_takeDown(mosaic);

        emit sig_reconstructView();
        emit sig_mosaicLoaded(file);
    }

    Sys::loadUnit->resetLoadState();

    return mosaic;
}

bool MosaicMaker::saveMosaic(MosaicPtr mosaic, bool forceOverwrite)
{
    qDebug() << "TiledPatternMaker::saveMosaic"  << mosaic->getName().get();

    // match size of mosaic view
    auto & canvas    = viewControl->getCanvas();
    QSize size       = Sys::view->getSize();
    QSize zsize      = canvas.getSize();

    CanvasSettings cs = _mosaic->getCanvasSettings();
    cs.setViewSize(size);
    cs.setCanvasSize(zsize);
    _mosaic->setCanvasSettings(cs);

    VersionedFile retSavedFile;
    MosaicManager mm;
    bool rv = mm.saveMosaic(mosaic,retSavedFile,forceOverwrite);
    if (rv)
    {
        Sys::mapEditor->keepStash(retSavedFile.getVersionedName());
        Sys::loadUnit->setLoadState(LOADING_MOSAIC,retSavedFile);
        Sys::loadUnit->resetLoadState();

        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(retSavedFile);
    }
    return rv;
}

void MosaicMaker::sm_takeDown(MosaicPtr mosaic)
{
	_mosaic = mosaic;

    auto bip   = _mosaic->getBkgdImage();
    auto bview = Sys::backgroundImageView;
    bview->setImage(bip);      // sets or clears

    if (bip)
    {
        bview->setModelXform(bip->getImageXform(),false);
    }

    QVector<ProtoPtr> protos = _mosaic->getPrototypes();

    qDebug() << "MosaicMaker::sm_takeDown styles=" << _mosaic->numStyles() << "protos=" << protos.size();

    prototypeMaker->sm_takeDown(protos);

    CanvasSettings csettings = mosaic->getCanvasSettings();

    Canvas & canvas = viewControl->getCanvas();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.initCanvasSize(csettings.getCanvasSize());

    if (config->splitScreen)
    {
        viewControl->setFixedSize(csettings.getViewSize());
    }
    else
    {
        viewControl->setSize(csettings.getViewSize());
    }

    viewControl->setBackgroundColor(VIEW_MOSAIC,csettings.getBackgroundColor());
}

/* actions are
    1. erase mosaic and re-create using new prototype (thin)
    2. replace prototype in existing mosaic (keeps the styles)
    3. add prototype to existing mosaic
*/

void MosaicMaker::sm_createMosaic(const QVector<ProtoPtr> prototypes)
{
    QColor oldColor = _mosaic->getCanvasSettings().getBackgroundColor();

    // This is a new mosaic
    _mosaic = make_shared<Mosaic>();

    CanvasSettings csettings = _mosaic->getCanvasSettings();

    for (auto & prototype : std::as_const(prototypes))
    {
        prototype->setMosaic(_mosaic);

        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);

        TilingPtr tp = prototype->getTiling();
        if (tp)
        {
            thick->setModelXform(tp->getModelXform(),false);
            csettings = tp->getData().getCanvasSettings();
        }
        else
        {
            // whaddya gonna do?
            Xform  xf = viewControl->getCurrentModelXform();
            thick->setModelXform(xf,false);
        }
    }

    // tiling has no intrinsic background color
    csettings.setBackgroundColor(oldColor);
    _mosaic->setCanvasSettings(csettings);
}

void MosaicMaker::sm_addPrototype(const QVector<ProtoPtr> prototypes)
{
    auto existing_protos = _mosaic->getPrototypes();

    for (auto & prototype : std::as_const(prototypes))
    {
        if (existing_protos.contains(prototype))
        {
            continue;

        }

        // this is the new prototype
        prototype->setMosaic(_mosaic);

        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);
        auto crop = _mosaic->getCrop();
        if (crop)
        {
            prototype->setCrop(crop);
        }
    }
}

void MosaicMaker::sm_removePrototype(const QVector<ProtoPtr> prototypes)
{
    auto existing_protos = _mosaic->getPrototypes();

    QVector<StylePtr> fordeletion;
    for (auto & prototype : std::as_const(existing_protos))
    {
        if (prototypes.contains(prototype))
        {
            continue;
        }

        // remove syles containing this proto
        for (auto & style : _mosaic->getStyleSet())
        {
            if (style->getPrototype() == prototype)
            {
                fordeletion.push_back(style);
            }
        }

        for (auto & style : fordeletion)
        {
            _mosaic->deleteStyle(style);
        }
    }
}

void MosaicMaker::sm_replacePrototype(ProtoPtr prototype)
{
    prototype->setMosaic(_mosaic);

    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto & style : std::as_const(sset))
    {
        style->setPrototype(prototype);
        auto crop = _mosaic->getCrop();
        if (crop)
            prototype->setCrop(crop);
    }
}

void MosaicMaker::sm_resetStyles()
{
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto & style : std::as_const(sset))
    {
        style->resetStyleRepresentation();
    }
}

void MosaicMaker::sm_takeUp(QVector<ProtoPtr> prototypes, eMOSM_Event event)
{
    // note - passing the prototypes themselves, not references, so that prototype maker weak pointers remain good.

    qDebug().noquote() << __FUNCTION__ << sMOSM_Events[event] << "protos" << prototypes.count();
    switch (event)
    {
    case MOSM_LOAD_EMPTY:
        sm_createMosaic(prototypes);
        break;

    case MOSM_LOAD_SINGLE:
        sm_createMosaic(prototypes);
        break;

    case MOSM_RELOAD_SINGLE:
        // the prototype remains the same, but may have different content
        // so just reset the styles so that they may be drawn again
        sm_resetStyles();
        break;

    case MOSM_LOAD_MULTI:
        sm_addPrototype(prototypes);
        break;

    case MOSM_RELOAD_MULTI:
        sm_replacePrototype(prototypes.first());
        sm_resetStyles();
        break;

    case MOSM_TILING_CHANGED:
        sm_replacePrototype(prototypes.first());
        sm_resetStyles();
        break;

    case MOSM_TILE_CHANGED:
    case MOSM_MOTIF_CHANGED:
        sm_resetStyles();
        break;

    case MOSM_TILING_DELETED:
        sm_removePrototype(prototypes);
        break;

    case MOSM_RENDER:
        if (!_mosaic->hasContent())
        {
            sm_createMosaic(prototypes);
        }
        else
        {
            sm_resetStyles();
        }
        break;

    default:
        qCritical().noquote() << __FUNCTION__ << "unsuported event" << sMOSM_Events[event];
    }

    if (viewControl->isEnabled(VIEW_MOSAIC))
        emit sig_reconstructView();
    else
        emit sig_updateView();
}

MosaicPtr MosaicMaker::getMosaic()
{
    return _mosaic;
}

CanvasSettings & MosaicMaker::getCanvasSettings()
{
    return _mosaic->getCanvasSettings();
}

void MosaicMaker::setCanvasSettings(CanvasSettings &ms)
{
    _mosaic->setCanvasSettings(ms);
}

void MosaicMaker::resetMosaic()
{
    _mosaic = make_shared<Mosaic>();
}

StylePtr MosaicMaker::makeStyle(eStyleType type, StylePtr oldStyle)
{
    StylePtr newStyle;

    switch (type)
    {
    case STYLE_PLAIN:
        newStyle = make_shared<Plain>(oldStyle);
        break;
    case STYLE_THICK:
        newStyle = make_shared<Thick>(oldStyle);
        break;
    case STYLE_OUTLINED:
        newStyle = make_shared<Outline>(oldStyle);
        break;
    case STYLE_INTERLACED:
        newStyle = make_shared<Interlace>(oldStyle);
        break;
    case STYLE_EMBOSSED:
        newStyle = make_shared<Emboss>(oldStyle);
        break;
    case STYLE_SKETCHED:
        newStyle = make_shared<Sketch>(oldStyle);
        break;
    case STYLE_FILLED:
        newStyle = make_shared<Filled>(oldStyle);
        break;
    case STYLE_TILECOLORS:
        newStyle = make_shared<TileColors>(oldStyle);
        break;
    case STYLE_STYLE:
    case STYLE_BORDER:
        Q_ASSERT(false);
        break;
    }

    return newStyle;
}
