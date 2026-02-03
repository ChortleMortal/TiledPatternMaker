#include "gui/map_editor/map_editor.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/image_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/prototypes/prototype.h"
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
#include "sys/engine/image_engine.h"
#include "sys/sys.h"
#include "sys/sys/load_unit.h"

using std::make_shared;

MosaicMaker::MosaicMaker()
{
    _mosaic = make_shared<Mosaic>();

    loadUnit = new LoadUnit(LT_MOSAIC);
}

MosaicMaker::~MosaicMaker()
{
    _mosaic.reset();
}

void MosaicMaker::init()
{
    connect(this, &MosaicMaker::sig_updateView,      Sys::viewController, &SystemViewController::slot_updateView);
    connect(this, &MosaicMaker::sig_reconstructView, Sys::viewController, &SystemViewController::slot_reconstructView);
}

// This the most complete load of a Mosaic - it does everything needed
// and notifies other pages with sig_mosaicLoaded
MosaicPtr MosaicMaker::loadMosaic(VersionedFile &file)
{
    qDebug().noquote() << "MosaicMaker::loadMosaic" << file.getVersionedName().get();

    Sys::debugView->unloadLayerContent();

    loadUnit->start(file);

    // the advantage of using the manager is that it does not persist
    MosaicPtr mosaic;
    MosaicManager mm;
    mosaic = mm.loadMosaic(file);
    if (mosaic)
    {
        Sys::imageViewer->unloadLayerContent();

        // starts the chain reaction
        sm_takeDown(mosaic);

        emit sig_reconstructView();
        emit sig_mosaicLoaded(file);

        loadUnit->end(LS_LOADED);
    }
    else
    {
        loadUnit->end(LS_FAILED);
    }

    return mosaic;
}

void MosaicMaker::reload()
{
    VersionedFile vf = loadUnit->getLoadFile();
    if (!vf.isEmpty())
        loadMosaic(vf);
}

bool MosaicMaker::saveMosaic(MosaicPtr mosaic, bool forceOverwrite)
{
    qDebug() << "TiledPatternMaker::saveMosaic"  << mosaic->getName().get();

    // match size of mosaic view
    auto & canvas    = Sys::viewController->getCanvas();
    QSize size       = canvas.getViewSize();
    QSize zsize      = canvas.getCanvasSize();

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
        loadUnit->declareLoaded(retSavedFile);

        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(retSavedFile);
    }
    return rv;
}

void MosaicMaker::sm_takeDown(MosaicPtr mosaic)
{
	_mosaic = mosaic;

    QVector<ProtoPtr> protos = _mosaic->getPrototypes();

    qDebug() << "MosaicMaker::sm_takeDown styles=" << _mosaic->numStyles() << "protos=" << protos.size();

    if (mosaic->getBkgdImage())
        Sys::currentBkgImage = BKGD_IMAGE_MOSAIC;
    else
        Sys::currentBkgImage = BKGD_IMAGE_NONE;

    Sys::prototypeMaker->sm_takeDown(protos);

    CanvasSettings csettings = mosaic->getCanvasSettings();

    Canvas & canvas = Sys::viewController->getCanvas();
    canvas.setCanvasSize(csettings.getCanvasSize());

    if (Sys::config->splitScreen)
    {
        Sys::viewController->setFixedSize(csettings.getViewSize());
    }
    else
    {
        Sys::viewController->setSize(csettings.getViewSize());
    }

    Sys::viewController->setBackgroundColor(VIEW_MOSAIC,csettings.getBackgroundColor());

    auto border = mosaic->getBorder();
    if (border && border->getRequiresConversion())
    {
        auto style  = mosaic->getFirstRegularStyle();
        auto xf     = style->getModelXform();
        border->setModelXform(xf,true,Sys::nextSigid());
        border->legacy_convertToModelUnits();
        border->setRequiresConversion(false);
        border->resetStyleRepresentation();
    }

    emit Sys::viewController->sig_resetLayers();
    Sys::viewController->setSelectedPrimaryLayer(mosaic->getFirstRegularStyle());
}

/* actions are
    1. erase mosaic and re-create using new prototype (thin)
    2. replace prototype in existing mosaic (keeps the styles)
    3. add prototype to existing mosaic
*/

void MosaicMaker::sm_createMosaic(ProtoPtr prototype)
{
    QColor oldColor = _mosaic->getCanvasSettings().getBackgroundColor();

    // This is a new mosaic
    _mosaic = make_shared<Mosaic>();

    CanvasSettings csettings = _mosaic->getCanvasSettings();

    prototype->setMosaic(_mosaic);

    StylePtr thick = make_shared<Plain>(prototype);
    _mosaic->addStyle(thick);

    TilingPtr tp = prototype->getTiling();
    if (tp)
    {
        thick->setModelXform(tp->getModelXform(),false,Sys::nextSigid());
        csettings = tp->hdr().getCanvasSettings();
    }
    else
    {
        // whaddya gonna do?
        Xform  xf = Sys::viewController->getSystemModelXform();
        thick->setModelXform(xf,false,Sys::nextSigid());
    }

    // tiling has no intrinsic background color
    csettings.setBackgroundColor(oldColor);
    _mosaic->setCanvasSettings(csettings);
}

void MosaicMaker::sm_addPrototype(ProtoPtr prototype)
{
    auto existing_protos = _mosaic->getPrototypes();
    if (existing_protos.contains(prototype))
    {
        qWarning() << "MosaicMaker::sm_addPrototype" <<" trying to add exisitng prototype";
        return;
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

void MosaicMaker::sm_removePrototype(ProtoPtr prototype)
{
    auto existing_protos = _mosaic->getPrototypes();
    if (!existing_protos.contains(prototype))
    {
        qWarning() << "MosaicMaker::sm_removePrototype" << "could not delete prototype";
        return;
    }

    QVector<StylePtr> fordeletion;
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

void MosaicMaker::sm_takeUp(MosaicEvent & mosEvent)
{
    // note - passing the prototype, not a references,
    // so prototype maker weak pointers remain good.

    auto event = mosEvent.event;
    auto proto = mosEvent.prototype;

    qDebug().noquote() << "MosaicMaker::sm_takeUp" << sMOSM_Events[event];

    switch (mosEvent.event)
    {
    case MOSM_LOAD_PROTO_EMPTY:
        sm_createMosaic(proto);
        break;

    case MOSM_LOAD_PROTO_SINGLE:
        sm_createMosaic(proto);
        break;

    case  MOSM_LOAD_PROTO_MULTI:
        sm_addPrototype(proto);
        break;

    case MOSM_RELOAD_PROTO_SINGLE:
        // the prototype remains the same, but may have different content
        // so just reset the styles so that they may be drawn again
        sm_resetStyles();
        break;

    case MOSM_RELOAD_PROTO_MULTI:
        sm_replacePrototype(proto);
        sm_resetStyles();
        break;

    case MOSM_PROTO_CHANGED:
        sm_resetStyles();
        break;

    case MOSM_PROTO_DELETED:
        sm_removePrototype(proto);
        break;

    default:
        qCritical().noquote() << "MosaicMaker::sm_takeUp" << "unsuported event" << sMOSM_Events[event];
        break;
    }

    if (Sys::viewController->isEnabled(VIEW_MOSAIC))
        emit sig_reconstructView();
    else
        emit sig_updateView();
}

MosaicPtr MosaicMaker::getMosaic()
{
    Q_ASSERT(_mosaic);  // there is always a mosaic
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
