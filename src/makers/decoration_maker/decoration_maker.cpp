#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "style/colored.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"
#include "style/tile_colors.h"
#include "base/mosaic.h"
#include "viewers/viewcontrol.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"

using std::make_shared;

DecorationMaker * DecorationMaker::mpThis = nullptr;

DecorationMaker * DecorationMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new DecorationMaker;
    }
    return mpThis;
}

DecorationMaker::DecorationMaker()
{
    _mosaic = make_shared<Mosaic>();
}

void DecorationMaker::init()
{
    motifMaker  = MotifMaker::getInstance();
    tilingMaker = TilingMaker::getSharedInstance();
    viewControl = ViewControl::getInstance();
}

void DecorationMaker::takeDown(MosaicPtr mosaic)
{
    _mosaic = mosaic;

    viewControl->setFillData(mosaic->getSettings().getFillData());

    // setup prototypes
    tilingMaker->eraseTilings();
    motifMaker->erasePrototypes();

    QVector<PrototypePtr> protos = mosaic->getPrototypes();
    for (auto& proto : protos)
    {
        motifMaker->takeDown(proto);
    }
#if 0
    PrototypePtr pp = protos.first();
    DesignElementPtr dp = pp->getDesignElement(0);
    viewControl->setSelectedDesignElement(dp);
#endif
}

/* actions are
    1. erase mosaic and re-create using new prototype (thin)
    2. replace prototype in existing mosaic (keeps the styles)
    3. add prototype to existing mosaic
*/

void DecorationMaker::sm_createMosaic(const QVector<PrototypePtr> prototypes)
{
    QColor oldColor = _mosaic->getSettings().getBackgroundColor();

    // This is a new mosaic
    _mosaic = make_shared<Mosaic>();
    ModelSettings & mosaicSettings = _mosaic->getSettings();

    for (auto prototype : prototypes)
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);

        TilingPtr tp = prototype->getTiling();
        ModelSettings & tilingSettings = tp->getSettings();
        mosaicSettings = tilingSettings;
    }

    // tiling has no intrinsic background color
    mosaicSettings.setBackgroundColor(oldColor);
}

void DecorationMaker::sm_addPrototype(const QVector<PrototypePtr> prototypes)
{
    for (auto prototype : prototypes)
    {
        StylePtr thick = make_shared<Plain>(prototype);
        _mosaic->addStyle(thick);
        prototype->setBorder(_mosaic->getBorder());
    }
}

void DecorationMaker::sm_replacePrototype(PrototypePtr prototype)
{
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->setPrototype(prototype);
        prototype->setBorder(_mosaic->getBorder());
    }
}

void DecorationMaker::sm_resetStyles()
{
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->resetStyleRepresentation();
    }
}

void DecorationMaker::sm_takeUp(QVector<PrototypePtr> prototypes, eSM_Event mode)
{
    qDebug() << "DecorationMaker::takeUp()" << sSM_Events[mode];

    switch (mode)
    {
    case SM_LOAD_EMPTY:
        resetMosaic();
        break;

    case SM_LOAD_SINGLE:
        sm_createMosaic(prototypes);
        break;

    case SM_RELOAD_SINGLE:
        sm_resetStyles();
        break;

    case SM_LOAD_MULTI:
        sm_addPrototype(prototypes);
        break;

    case SM_RELOAD_MULTI:
        sm_replacePrototype(prototypes.first());
        break;

    case SM_LOAD_FROM_MOSAIC:
        qWarning("Invalid mode");
        break;

    case SM_FEATURE_CHANGED:
    case SM_FIGURE_CHANGED:
    case SM_TILING_CHANGED:
        sm_resetStyles();
        break;

    case SM_RENDER:
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
}

MosaicPtr DecorationMaker::getMosaic()
{
    return _mosaic;
}

ModelSettings & DecorationMaker::getMosaicSettings()
{
    return _mosaic->getSettings();
}

void DecorationMaker::resetMosaic()
{
    _mosaic = make_shared<Mosaic>();
}

StylePtr DecorationMaker::makeStyle(eStyleType type, StylePtr oldStyle)
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
