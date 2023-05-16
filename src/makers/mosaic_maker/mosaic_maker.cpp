#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/cycler.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_manager.h"
#include "makers/map_editor/map_editor.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/panel.h"
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
#include "viewers/viewcontrol.h"

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

MosaicMaker::MosaicMaker()
{
    _mosaic = make_shared<Mosaic>();
}

void MosaicMaker::init()
{
    prototypeMaker = PrototypeMaker::getInstance();
    config         = Configuration::getInstance();
    controlPanel   = ControlPanel::getInstance();
    viewControl    = ViewControl::getInstance();
    tilingMaker    = TilingMaker::getSharedInstance();
    auto cycler    = Cycler::getInstance();

    connect(cycler,&Cycler::sig_cycleLoadMosaic,    this,   &MosaicMaker::slot_cycleLoadMosaic);
    connect(this,  &MosaicMaker::sig_cycler_ready,  cycler, &Cycler::slot_ready);

}

void MosaicMaker::slot_loadMosaic(QString name,bool ready)
{
    qDebug().noquote() << "TiledPatternMaker::slot_loadXML() <" << name << ">";

    controlPanel->setLoadState(ControlPanel::LOADING_MOSAIC,name);

    config->currentlyLoadedXML.clear();

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        viewControl->removeAllImages();

        viewControl->slot_refreshView();
        if (ready)
        {
            emit sig_cycler_ready();
        }
        emit sig_mosaicLoaded(name);
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void MosaicMaker::slot_cycleLoadMosaic(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_cycleLoadMosaic() <" << name << ">";

    controlPanel->setLoadState(ControlPanel::LOADING_MOSAIC,name);

    config->currentlyLoadedXML.clear();

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        viewControl->removeAllImages();

        viewControl->slot_refreshView();
        emit sig_cycler_ready();
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void MosaicMaker::slot_saveMosaic(QString filename)
{
    qDebug() << "TiledPatternMaker::slot_saveMosaic()";

    QString savedFile;
    MosaicManager mm;
    bool rv = mm.saveMosaic(filename,savedFile,false);
    if (rv)
    {
        auto mapEditor = MapEditor::getInstance();
        mapEditor->keepStash(savedFile);
        config->lastLoadedXML      = savedFile;
        config->currentlyLoadedXML = savedFile;

        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(savedFile);
    }
}


void MosaicMaker::sm_takeDown(MosaicPtr mosaic)
{
    _mosaic = mosaic;

    FillData fd = mosaic->getSettings().getFillData();
    viewControl->setFillData(fd);

    QVector<ProtoPtr> protos = mosaic->getPrototypes();
    prototypeMaker->sm_takeDown(protos);
}

/* actions are
    1. erase mosaic and re-create using new prototype (thin)
    2. replace prototype in existing mosaic (keeps the styles)
    3. add prototype to existing mosaic
*/

void MosaicMaker::sm_createMosaic(const QVector<ProtoPtr> prototypes)
{
    QColor oldColor = _mosaic->getSettings().getBackgroundColor();
    Xform  xf       = viewControl->getCurrentXform2();

    // This is a new mosaic
    _mosaic = make_shared<Mosaic>();
    ModelSettings & mosaicSettings = _mosaic->getSettings();

    for (auto prototype : prototypes)
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);

        TilingPtr tp = prototype->getTiling();
        if (tp)
        {
            thick->setCanvasXform(tp->getCanvasXform());
            mosaicSettings = tp->getData().getSettings();
        }
        else
        {
            // whaddya gonna do?
            thick->setCanvasXform(xf);
        }
    }

    // tiling has no intrinsic background color
    mosaicSettings.setBackgroundColor(oldColor);
}

void MosaicMaker::sm_addPrototype(const QVector<ProtoPtr> prototypes)
{
    for (auto prototype : prototypes)
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);
        prototype->resetCrop(_mosaic->getCrop());
    }
}

void MosaicMaker::sm_replacePrototype(ProtoPtr prototype)
{
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->setPrototype(prototype);
        prototype->resetCrop(_mosaic->getCrop());
    }
}

void MosaicMaker::sm_resetStyles()
{
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->resetStyleRepresentation();
    }
}

void MosaicMaker::sm_takeUp(QVector<ProtoPtr> prototypes, eMOSM_Event event)
{
    // note - passing the prototypes themselves, not references, so that prototype maker weak pointers remain good.

    qInfo().noquote() << "MosaicMaker::takeUp()" << sMOSM_Events[event] << "protos" << prototypes.count();
    switch (event)
    {
    case MOSM_LOAD_EMPTY:
        sm_createMosaic(prototypes);
        break;

    case MOSM_LOAD_SINGLE:
        sm_createMosaic(prototypes);
        break;

    case MOSM_RELOAD_SINGLE:
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
        viewControl->update();
}

MosaicPtr MosaicMaker::getMosaic()
{
    return _mosaic;
}

ModelSettings & MosaicMaker::getMosaicSettings()
{
    return _mosaic->getSettings();
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
