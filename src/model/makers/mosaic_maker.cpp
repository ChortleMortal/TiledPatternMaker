#include <QFile>
#include "gui/map_editor/map_editor.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/image_view.h"
#include "model/borders/border_plain.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/borders/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/prototypes/prototype.h"
#include "model/settings/canvas.h"
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
    connect(this, &MosaicMaker::sig_updateView,      Sys::systemViewController, &ViewController::slot_updateView);
    connect(this, &MosaicMaker::sig_reconstructView, Sys::systemViewController, &ViewController::slot_reconstructView);
}

// This the most complete load of a Mosaic - it does everything needed
// and notifies other pages with sig_mosaicLoaded
MosaicPtr MosaicMaker::load(VersionedFile &file)
{
    qDebug().noquote() << "MosaicMaker::loadMosaic" << file.getVersionedName().get();

    Sys::debugView->unloadLayerContent();

    emit sig_mosaicLoading(file);
    loadUnit->start(file);

    QString failMsg;
    QString warnMsg;
    MosaicPtr mosaic = loadMosaic(file,failMsg,warnMsg);
    if (mosaic)
    {
        Sys::imageViewer->unloadLayerContent();

        // starts the chain reaction
        sm_takeDown(mosaic);

        emit sig_reconstructView();
        emit sig_mosaicLoaded(file);

        if (!warnMsg.isEmpty())
            emit sig_messageBox(warnMsg,QString(),QMessageBox::Warning,false);

        loadUnit->end(LS_LOADED);
    }
    else
    {
        loadUnit->end(LS_FAILED);
        emit sig_messageBox(failMsg,QString(),QMessageBox::Critical,false);
    }

    return mosaic;
}

void MosaicMaker::reload()
{
    VersionedFile vf = loadUnit->getLoadFile();
    if (!vf.isEmpty())
    {
        load(vf);
    }
}

bool MosaicMaker::save(VersionedFile mosaicFile, QString & failMsg, bool cleanseMaps, QStringList & unverifiedMaps)
{
    qDebug() << "TiledPatternMaker::saveMosaic"  << _mosaic->getName().get();

    // if a tiling is still the active canvas (i.e. this mosaic was built fresh from a
    // tiling this session and never activated - see sm_createMosaic()), the mosaic's
    // own canvas may be stale (only seeded once, at creation time) - bring it up to
    // date from the tiling actually being edited before saving, and only then make
    // the mosaic active, since saving it is a deliberate "this is the current thing" signal
    if (Sys::getActiveCanvasOwner() == ACTIVE_TILING)
    {
        TilingPtr tiling = Sys::tilingMaker->getSelectedTiling();
        if (tiling && _mosaic->getTilings().contains(tiling))
        {
            Canvas * mos_canvas = _mosaic->getMosaicCanvas();
            *mos_canvas = *(tiling->hdr().getTilingCanvas());
            Sys::setActiveCanvas(ACTIVE_MOSAIC);
        }
    }

    _mosaic->setBkgdRecord(Sys::backgroundImage->toRecord());

    qDebug() << "Saving XML to:"  << mosaicFile.getPathedName();

    MosaicWriter writer;
    writer.setCleanseMaps(cleanseMaps);
    bool rv = writer.writeXML(mosaicFile,_mosaic);
    unverifiedMaps = writer.getUnverifiedMaps();
    if (!rv)
    {
        failMsg = writer.getFailMsg();
    }
    else
    {
        VersionedName vname = mosaicFile.getVersionedName();
        _mosaic->setName(vname);
        loadUnit->declareLoaded(mosaicFile);

        // note: sig_mosaicLoaded is NOT emitted here - the in-memory mosaic is unchanged by a
        // save, only its file name/version may have. Consumers that need to resync (e.g. the
        // Map Editor's stash, page_loaders' file list) should react to sig_mosaicWritten instead.
        emit sig_mosaicWritten(mosaicFile);
    }
    return rv;
}

void MosaicMaker::sm_takeDown(MosaicPtr mosaic)
{
	_mosaic = mosaic;

    QVector<ProtoPtr> protos = _mosaic->getPrototypes();

    qDebug() << "MosaicMaker::sm_takeDown styles=" << _mosaic->numStyles() << "protos=" << protos.size();

    // the mosaic's background image (or its tiling's, if it has none) becomes the one image
    Sys::backgroundImage->load(mosaic->getBkgdRecord());

    Sys::motifMaker->sm_takeDown(protos);

    Sys::setActiveCanvas(ACTIVE_MOSAIC);
    Sys::systemViewController->setTakerViewSize(Sys::getActiveCanvas()->getViewSize());

    Sys::config->setBkgdColor(Layer::Type::Mosaic,mosaic->getBkgdColor());

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

    emit Sys::systemViewController->sig_resetLayers();
    Sys::systemViewController->setSelectedPrimaryLayer(mosaic->getFirstRegularStyle());
}

/* actions are
    1. erase mosaic and re-create using new prototype (thin)
    2. replace prototype in existing mosaic (keeps the styles)
    3. add prototype to existing mosaic
*/

void MosaicMaker::sm_createMosaic(ProtoPtr prototype)
{
    // This is a new mosaic
    _mosaic = make_shared<Mosaic>();
    _mosaic->setBkgdColor(Sys::config->getBkgdColor(Layer::Type::Mosaic));


    prototype->setMosaic(_mosaic);

    StylePtr thick = make_shared<Plain>(prototype);
    _mosaic->addStyle(thick);

    Canvas * mos_canvas = _mosaic->getMosaicCanvas();

    TilingPtr tp = prototype->getTiling();
    if (tp)
    {
        thick->setModelXform(tp->getModelXform(),false,Sys::nextSigid());
        // the mosaic has its own Canvas (it is what gets saved), so its content - not its
        // pointer - must be seeded from the tiling's canvas or the saved view size defaults
        *mos_canvas = *(tp->hdr().getTilingCanvas());
    }
    else
    {
        // whaddya gonna do?
        Xform  xf = Sys::systemViewController->getSystemModelXform();
        thick->setModelXform(xf,false,Sys::nextSigid());
    }


    // deliberately does NOT activate the mosaic's canvas - while creating/editing a
    // tiling, the tiling stays the active canvas even though a mosaic now silently
    // exists alongside it; see MosaicMaker::save() for when the mosaic canvas is
    // actually brought up to date and made active
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

void MosaicMaker::sm_replacePrototype(const QVector<ProtoPtr> & oldPrototypes, ProtoPtr newPrototype)
{
    newPrototype->setMosaic(_mosaic);

    if (oldPrototypes.isEmpty())
    {
        // defensive fallback only - every caller should supply the old prototype(s) it is replacing
        qWarning() << "MosaicMaker::sm_replacePrototype - no old prototypes given, repointing all styles";
    }

    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto & style : std::as_const(sset))
    {
        if (!oldPrototypes.isEmpty() && !oldPrototypes.contains(style->getPrototype()))
            continue;   // this style belongs to a different (unrelated) prototype - leave it alone

        style->setPrototype(newPrototype);
        auto crop = _mosaic->getCrop();
        if (crop)
            newPrototype->setCrop(crop);
    }
}

void MosaicMaker::sm_resetStyles()
{
    _mosaic->resetStyleMaps();
}

void MosaicMaker::sm_takeUp(MosaicEvent & mosEvent)
{
    // note - passing the prototype, not a references,
    // so prototype maker weak pointers remain good.

    auto event = mosEvent.event;
    auto proto = mosEvent.prototype;

    qDebug().noquote() << "MosaicMaker::sm_takeUp" << sMOSM_Events[(int)event];

    switch (mosEvent.event)
    {
    case Event::LoadEmpty:
        sm_createMosaic(proto);
        break;

    case Event::LoadProtoSingle:
        sm_createMosaic(proto);
        break;

    case Event::LoadProtoMulti:
        sm_addPrototype(proto);
        break;

    case Event::ReloadProtoSingle:
        // the prototype remains the same, but may have different content
        // so just reset the styles so that they may be drawn again
        sm_resetStyles();
        break;

    case Event::ReloadProtoMulti:
        sm_replacePrototype(mosEvent.oldPrototypes, proto);
        sm_resetStyles();
        break;

    case Event::ProtoChanged:
        sm_resetStyles();
        break;

    case Event::ProtoDeleted:
        sm_removePrototype(proto);
        break;
    }

    if (Sys::systemViewController->isEnabled(Layer::Type::Mosaic))
        emit sig_reconstructView();
    else
        emit sig_updateView();
}

MosaicPtr MosaicMaker::getMosaic()
{
    Q_ASSERT(_mosaic);  // there is always a mosaic
    return _mosaic;
}

void MosaicMaker::resetMosaic()
{
    _mosaic = make_shared<Mosaic>();
}

void MosaicMaker::setBkgdColor(QColor color)
{
    _mosaic->setBkgdColor(color);
    Sys::config->setBkgdColor(Layer::Type::Mosaic,color);
}

StylePtr MosaicMaker::makeStyle(StylePtr style, eStyleType newType)
{
    StylePtr newStyle;

    switch (newType)
    {
    case STYLE_PLAIN:
        newStyle = make_shared<Plain>(style);
        break;
    case STYLE_THICK:
        newStyle = make_shared<Thick>(style);
        break;
    case STYLE_OUTLINED:
        newStyle = make_shared<Outline>(style);
        break;
    case STYLE_INTERLACED:
        newStyle = make_shared<Interlace>(style);
        break;
    case STYLE_EMBOSSED:
        newStyle = make_shared<Emboss>(style);
        break;
    case STYLE_SKETCHED:
        newStyle = make_shared<Sketch>(style);
        break;
    case STYLE_FILLED:
        newStyle = make_shared<Filled>(style);
        break;
    case STYLE_TILECOLORS:
        newStyle = make_shared<TileColors>(style);
        break;
    case STYLE_BORDER:
        newStyle = make_shared<BorderPlain>(_mosaic.get());
        break;
    case STYLE_NONE:
        Q_ASSERT(false);
        break;
    }

    return newStyle;
}

const QString sMOSM_Events[] =
    {
        E2STR(LoadEmpty),
        E2STR(LoadProtoSingle),
        E2STR(LoadProtoMulti),
        E2STR(ReloadProtoSingle),
        E2STR(ReloadProtoMulti),
        E2STR(ProtoDeleted),
        E2STR(ProtoChanged)
};

MosaicPtr MosaicMaker::loadMosaic(VersionedFile vfile, QString & failMsg, QString & warnMsg)
{
    qDebug().noquote() << "MosaicMaker::loadMosaic()" << vfile.getVersionedName().get();

    MosaicPtr mosaic;

    if (vfile.isEmpty())
    {
        failMsg = QString("File <%1>not found").arg(vfile.getVersionedName().get());
        return mosaic;
    }

    QFile afile(vfile.getPathedName());
    if (!afile.exists())
    {
        failMsg = QString("File <%1>not found").arg(vfile.getPathedName());
        return mosaic;
    }

    qDebug().noquote() << "Loading:"  << vfile.getPathedName();

    // load
    MosaicReader reader(Sys::systemViewController);
    mosaic = reader.readXML(vfile);

    if (!mosaic)
    {
        failMsg = QString("Load ERROR - %1").arg(reader.getFailMessage());
        return mosaic;
    }

    warnMsg = reader.getWarnings().join("\n");

    mosaic->setName(vfile.getVersionedName());

    //mosaic->dumpTransforms();

    return mosaic;
}
