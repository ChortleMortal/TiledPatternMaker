#include "settings/frame_settings.h"
#include "misc/defaults.h"
#include "settings/configuration.h"

FrameData::FrameData()
{
    config = Configuration::getInstance();
}

FrameData::FrameData(Bounds bounds, QSize size)
{
    config = Configuration::getInstance();

    initBounds    = bounds;
    this->bounds  = bounds;
    initSize      = size;
    cropSize      = size;
    zoomSize      = size;

    calculateTransform();
    calculateBaseInv();
}

FrameData::FrameData(const FrameData & other)
{
    config = Configuration::getInstance();

    initBounds  = other.initBounds;
    bounds      = other.bounds;
    initSize    = other.initSize;
    cropSize    = other.cropSize;
    zoomSize    = other.zoomSize;
    transform   = other.transform;
}

FrameData & FrameData::operator=(const FrameData & other)
{
    config = Configuration::getInstance();

    initBounds  = other.initBounds;
    bounds      = other.bounds;
    initSize    = other.initSize;
    cropSize    = other.cropSize;
    zoomSize    = other.zoomSize;
    transform   = other.transform;

    return *this;
}

void FrameData::reInit()
{
    bounds      = initBounds;
    cropSize    = initSize;
    zoomSize    = initSize;

    calculateTransform();
    calculateBaseInv();
}

void FrameData::setDeltaSize(QSize size)
{
    cropSize += size;
    if (config->scaleToView)
    {
        zoomSize += size;
        calculateTransform();
    }
}

/*
    Results:
    VIEW_DESIGN         scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO          scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO_FEATURE  scale=75 rot=0 (0) trans=750 550
    VIEW_DEL            scale=75 rot=0 (0) trans=750 550
    VIEW_FIGURE_MAKER   scale=45 rot=0 (0) trans=450 450
    VIEW_TILING         scale=75 rot=0 (0) trans=750 550
    VIEW_TILIING_MAKER  scale=100 rot=0 (0)trans=500 500
    VIEW_MAP_EDITOR     scale=45 rot=0 (0) trans=450 450
*/

void FrameData::calculateTransform()
{
    if (zoomSize.isNull())
    {
        transform.reset();
        return;
    }

    qreal w      = qreal(zoomSize.width());
    qreal h      = qreal(zoomSize.height());
    qreal aspect = w / h;
    qreal height = bounds.width / aspect;
    qreal scalex = w /bounds.width;

    QTransform first  = QTransform().translate(-bounds.left, - (bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,((w -h)/2.0));
    transform         = first * second * third;
    //qDebug().noquote() << Transform::toInfoString(transform);
}

///////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////

FrameSettings::FrameSettings()
{
    modelAlignment = M_ALIGN_TILING;  // default

    add(VIEW_DESIGN,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_MOSAIC,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_PROTOTYPE,         Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_MOTIF_MAKER,       Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_TILING,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_TILING_MAKER,      Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_BORDER,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_CROP,              Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_MAP_EDITOR,        Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_BKGD_IMG,          Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    add(VIEW_GRID,              Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
}

void  FrameSettings::add(eViewType evt, Bounds bounds, QSize size)
{
    settings[evt] =  new FrameData(bounds,size);
}

void  FrameSettings::reInit()
{
    reInit(VIEW_DESIGN);
    reInit(VIEW_MOSAIC);
    reInit(VIEW_PROTOTYPE);
    reInit(VIEW_MOTIF_MAKER);
    reInit(VIEW_TILING);
    reInit(VIEW_TILING_MAKER);
    reInit(VIEW_BORDER);
    reInit(VIEW_CROP);
    reInit(VIEW_MAP_EDITOR);
    reInit(VIEW_BKGD_IMG);
    reInit(VIEW_GRID);
}

void  FrameSettings::initialiseCommon(QSize cropSize, QSize  zoomSize)
{
    // list does not include the tiling maker, motif maker, and design
    initialise(VIEW_MOSAIC,cropSize,zoomSize);
    initialise(VIEW_PROTOTYPE,cropSize,zoomSize);
    initialise(VIEW_TILING,cropSize,zoomSize);
    initialise(VIEW_TILING_MAKER,cropSize,zoomSize);
    initialise(VIEW_BORDER,cropSize,zoomSize);
    initialise(VIEW_CROP,cropSize,zoomSize);
    initialise(VIEW_MAP_EDITOR,cropSize,zoomSize);
    initialise(VIEW_BKGD_IMG,cropSize,zoomSize);
    initialise(VIEW_GRID,cropSize,zoomSize);
}

void  FrameSettings::setCommonDeltaSizes(QSize sz)
{
    // list does not include the tiling maker, motif maker, and design
    setDeltaSize(VIEW_MOSAIC,sz);
    setDeltaSize(VIEW_PROTOTYPE,sz);
    setDeltaSize(VIEW_TILING,sz);
    setDeltaSize(VIEW_TILING_MAKER,sz);
    setDeltaSize(VIEW_BORDER,sz);
    setDeltaSize(VIEW_CROP,sz);
    setDeltaSize(VIEW_MAP_EDITOR,sz);
    setDeltaSize(VIEW_BKGD_IMG,sz);
    setDeltaSize(VIEW_GRID,sz);
}

void  FrameSettings::reInit(eViewType evt)
{
   settings[evt]->reInit();
}

void FrameSettings::initialise(eViewType e, QSize cropSize, QSize zoomSize)
{
    //qDebug().noquote() << "FrameSettings::initialise()" << sViewerType[e] << cropSize << zoomSize;
    settings[e]->setCropSize(cropSize);
    settings[e]->setZoomSize(zoomSize);
    settings[e]->calculateBaseInv();
}

QTransform FrameSettings::getTransform(eViewType e)
{
    QTransform t = settings[e]->getTransform();
    //qDebug().noquote() << "FrameSettings::getTransform" << sViewerType[e] << Transform::toInfoString(t);
    return t;
}

void  FrameSettings::setDeltaSize(eViewType e, QSize sz)
{
    settings[e]->setDeltaSize(sz);
}

QSize FrameSettings::getCropSize(eViewType e)
{
    QSize sz =  settings[e]->getCropSize();
    //qDebug().noquote() << "FrameSettings::getCropSize()" << sViewerType[e] << sz;
    return sz;
}

QSize FrameSettings::getZoomSize(eViewType e)
{
    QSize sz =  settings[e]->getZoomSize();
    //qDebug().noquote() << "FrameSettings::getZoomSize()" << sViewerType[e] << sz;
    return sz;
}

