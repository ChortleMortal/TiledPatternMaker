#include "makers/mosaic_maker/mosaic_maker.h"
#include "engine/image_engine.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_manager.h"
#include "misc/sys.h"
#include "makers/map_editor/map_editor.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "style/emboss.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/thick.h"
#include "style/tile_colors.h"
#include "tile/tiling.h"
#include "tile/backgroundimage.h"
#include "viewers/backgroundimageview.h"
#include "viewers/view_controller.h"
#include "viewers/view.h"

using std::make_shared;

MosaicMaker * MosaicMaker::mpThis = nullptr;

MosaicMaker * MosaicMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new MosaicMaker;
    }
    return mpThis;
}

void MosaicMaker::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

MosaicMaker::MosaicMaker()
{
    _mosaic = make_shared<Mosaic>();
}

void MosaicMaker::init()
{
    prototypeMaker = PrototypeMaker::getInstance();
    config         = Configuration::getInstance();
    controlPanel   = ControlPanel::getInstance();
    viewControl    = Sys::viewController;
    view           = Sys::view;
    tilingMaker    = TilingMaker::getInstance();
}

// This the most complete load of a Mosaic - it does everything needed
// and notifies other pages with sig_mosaicLoaded
MosaicPtr MosaicMaker::loadMosaic(QString name)
{
    qDebug().noquote() << "MosaicMaker::loadMosaic" << name;

    LoadUnit & loadUnit = view->getLoadUnit();
    loadUnit.setLoadState(LOADING_MOSAIC,name);

    // the advantage of using the manager is that it does not persist
    MosaicPtr mosaic;
    MosaicManager mm;
    mosaic = mm.loadMosaic(name);
    if (mosaic)
    {
        viewControl->removeAllImages();

        // starts the chain reaction
        sm_takeDown(mosaic);

        viewControl->slot_reconstructView();
        emit sig_mosaicLoaded(name);
    }

    loadUnit.resetLoadState();

    return mosaic;
}

bool MosaicMaker::saveMosaic(QString filename, bool forceOverwrite)
{
    qDebug() << "TiledPatternMaker::saveMosaic"  << filename;

    // match size of mosaic view
    auto & canvas    = viewControl->getCanvas();
    QSize size       = view->getCurrentSize();
    QSizeF zsize     = canvas.getSize();

    CanvasSettings cs = _mosaic->getCanvasSettings();
    cs.setViewSize(size);
    cs.setCanvasSize(zsize);
    _mosaic->setCanvasSettings(cs);

    QString savedFile;
    MosaicManager mm;
    bool rv = mm.saveMosaic(filename,savedFile,forceOverwrite);
    if (rv)
    {
        auto mapEditor = MapEditor::getInstance();
        mapEditor->keepStash(savedFile);
        LoadUnit & loadUnit = view->getLoadUnit();
        loadUnit.setLoadState(LOADING_MOSAIC,savedFile);
        loadUnit.resetLoadState();

        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(savedFile);
    }
    return rv;
}

void MosaicMaker::sm_takeDown(MosaicPtr mosaic)
{
	_mosaic = mosaic;

    auto bip   = _mosaic->getBkgdImage();
    auto bview = BackgroundImageView::getInstance();
    bview->setImage(bip);      // sets or clears

    if (bip)
    {
        bview->setModelXform(bip->getImageXform(),false);
    }

    QVector<ProtoPtr> protos = _mosaic->getPrototypes();

    qDebug() << "MosaicMaker::sm_takeDown styles=" << _mosaic->numStyles() << "protos=" << protos.size();

    prototypeMaker->sm_takeDown(protos);

    CanvasSettings csettings = mosaic->getCanvasSettings();
    auto fd = csettings.getFillData();

    Canvas & canvas = viewControl->getCanvas();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.setFillData(fd);
    canvas.initCanvasSize(csettings.getCanvasSize());

    if (config->splitScreen)
    {
        view->setFixedSize(csettings.getViewSize());
    }
    else
    {
        view->resize(csettings.getViewSize());
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
    CanvasSettings mosModelSettings = _mosaic->getCanvasSettings();

    for (auto & prototype : std::as_const(prototypes))
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);

        TilingPtr tp = prototype->getTiling();
        if (tp)
        {
            thick->setModelXform(tp->getModelXform(),false);
            mosModelSettings = tp->getData().getSettings();
        }
        else
        {
            // whaddya gonna do?
            Xform  xf = viewControl->getCurrentModelXform();
            thick->setModelXform(xf,false);
        }
    }

    // tiling has no intrinsic background color
    mosModelSettings.setBackgroundColor(oldColor);
    _mosaic->setCanvasSettings(mosModelSettings);
}

void MosaicMaker::sm_addPrototype(const QVector<ProtoPtr> prototypes)
{
    for (auto & prototype : std::as_const(prototypes))
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);
        auto crop = _mosaic->getCrop();
        if (crop)
            prototype->setCrop(crop);
    }
}

void MosaicMaker::sm_replacePrototype(ProtoPtr prototype)
{
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

    qDebug().noquote() << "MosaicMaker::takeUp()" << sMOSM_Events[event] << "protos" << prototypes.count();
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
        break;

    case MOSM_TILE_CHANGED:
    case MOSM_MOTIF_CHANGED:
    case MOSM_TILING_CHANGED:
        sm_resetStyles();
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
    }

    if (viewControl->isEnabled(VIEW_MOSAIC))
        viewControl->slot_reconstructView();
    else
        Sys::view->update();
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
        Q_ASSERT(false);
        break;
    }

    return newStyle;
}
