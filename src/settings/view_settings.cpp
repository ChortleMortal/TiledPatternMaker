#include "settings/view_settings.h"
#include "misc/defaults.h"
#include "settings/configuration.h"

ViewData::ViewData()
{
}

ViewData::ViewData(Bounds bounds, QSize size, QColor color)
{
    initBounds    = bounds;
    this->bounds  = bounds;
    initSize      = size;
    cropSize      = size;
    zoomSize      = size;
    bkgdColor     = color;

    calculateTransform();
    calculateBaseInv();
}

ViewData::ViewData(const ViewData & other)
{
    initBounds  = other.initBounds;
    initSize    = other.initSize;
    bounds      = other.bounds;
    cropSize    = other.cropSize;
    zoomSize    = other.zoomSize;
    transform   = other.transform;
    bkgdColor   = other.bkgdColor;
}

ViewData & ViewData::operator=(const ViewData & other)
{
    initBounds  = other.initBounds;
    initSize    = other.initSize;
    bounds      = other.bounds;
    cropSize    = other.cropSize;
    zoomSize    = other.zoomSize;
    transform   = other.transform;
    bkgdColor   = other.bkgdColor;

    return *this;
}

void ViewData::reInit()
{
    bounds      = initBounds;
    cropSize    = initSize;
    zoomSize    = initSize;

    calculateTransform();
    calculateBaseInv();
}

void ViewData::setDeltaSize(QSize size)
{
    cropSize += size;
    if (Configuration::getInstance()->scaleToView)
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

void ViewData::calculateTransform()
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

ViewSettings::ViewSettings()
{
    setModelAlignment(M_ALIGN_TILING);  // default

    add(VIEW_DESIGN,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_MOSAIC,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_PROTOTYPE,         Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_MOTIF_MAKER,       Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_TILING,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_TILING_MAKER,      Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_BORDER,            Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_CROP,              Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_MAP_EDITOR,        Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_BKGD_IMG,          Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));
    add(VIEW_GRID,              Bounds(-10.0,10.0,20.0), QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT), QColor(Qt::white));

    QStringList qsl = Configuration::getInstance()->viewColors;
    for (qsizetype i = 0; i < qsl.size(); i++)
    {
        const auto & colorname = qsl.at(i);
        QColor color(colorname);
        setBkgdColor(static_cast<eViewType>(i), color);
    }
}

void  ViewSettings::add(eViewType evt, Bounds bounds, QSize size, QColor color)
{
    settings[evt] =  new ViewData(bounds,size, color);
}

void  ViewSettings::reInit()
{
    reInit(VIEW_DESIGN);
    reInit(VIEW_MOSAIC);
    reInit(VIEW_PROTOTYPE);
    reInit(VIEW_MOTIF_MAKER);
    reInit(VIEW_TILING);
    reInit(VIEW_TILING_MAKER);
    reInit(VIEW_MAP_EDITOR);
    reInit(VIEW_BKGD_IMG);
    reInit(VIEW_GRID);
    reInit(VIEW_BORDER);
    reInit(VIEW_CROP);
  //reInit(VIEW_MEASURE);
  //reInit(VIEW_CENTER);
  //reInit(VIEW_IMAGE);
}

void  ViewSettings::initialiseCommon(QSize cropSize, QSize  zoomSize)
{
    // list does not include the tiling maker, motif maker, and design
  //initialise(VIEW_DESIGN,cropSize,zoomSize);
    initialise(VIEW_MOSAIC,cropSize,zoomSize);
    initialise(VIEW_PROTOTYPE,cropSize,zoomSize);
  //initialise(VIEW_MOTIF_MAKER,cropSize,zoomSize);
    initialise(VIEW_TILING,cropSize,zoomSize);
    initialise(VIEW_TILING_MAKER,cropSize,zoomSize);
    initialise(VIEW_MAP_EDITOR,cropSize,zoomSize);
    initialise(VIEW_BKGD_IMG,cropSize,zoomSize);
    initialise(VIEW_GRID,cropSize,zoomSize);
    initialise(VIEW_BORDER,cropSize,zoomSize);
    initialise(VIEW_CROP,cropSize,zoomSize);
  //initialise(VIEW_MEASURE,cropSize,zoomSize);
  //initialise(VIEW_CENTER,cropSize,zoomSize);
  //initialise(VIEW_IMAGE,cropSize,zoomSize);
}

void  ViewSettings::setCommonDeltaSizes(QSize sz)
{
    // list does not include the tiling maker, motif maker, and design
  //setDeltaSize(VIEW_DESIGN,sz);
    setDeltaSize(VIEW_MOSAIC,sz);
    setDeltaSize(VIEW_PROTOTYPE,sz);
  //setDeltaSize(VIEW_MOTIF_MAKER,sz);
    setDeltaSize(VIEW_TILING,sz);
    setDeltaSize(VIEW_TILING_MAKER,sz);
    setDeltaSize(VIEW_MAP_EDITOR,sz);
    setDeltaSize(VIEW_BKGD_IMG,sz);
    setDeltaSize(VIEW_GRID,sz);
    setDeltaSize(VIEW_BORDER,sz);
    setDeltaSize(VIEW_CROP,sz);
  //setDeltaSize(VIEW_MEASURE,sz);
  //setDeltaSize(VIEW_CENTER,sz);
  //setDeltaSize(VIEW_IMAGE,sz);
}

void  ViewSettings::reInit(eViewType evt)
{
   settings[evt]->reInit();
}

void ViewSettings::reInitBkgdColors(QColor bcolor)
{
  //setBkgdColor(VIEW_DESIGN,bcolor);
   setBkgdColor(VIEW_MOSAIC,bcolor);
   setBkgdColor(VIEW_PROTOTYPE,bcolor);
   setBkgdColor(VIEW_MOTIF_MAKER,bcolor);
   setBkgdColor(VIEW_TILING,bcolor);
   setBkgdColor(VIEW_TILING_MAKER,bcolor);
   setBkgdColor(VIEW_MAP_EDITOR,bcolor);
   setBkgdColor(VIEW_BKGD_IMG,bcolor);
   setBkgdColor(VIEW_GRID,bcolor);
   setBkgdColor(VIEW_BORDER,bcolor);
   setBkgdColor(VIEW_CROP,bcolor);
 //setBkgdColor(VIEW_MEASURE,bcolor);
 //setBkgdColor(VIEW_CENTER,bcolor);
 //setBkgdColor(VIEW_IMAGE,bcolor);
}

void ViewSettings::initialise(eViewType e, QSize cropSize, QSize zoomSize)
{
    qDebug().noquote() << "FrameSettings::initialise()" << sViewerType[e] << cropSize << zoomSize;
    settings[e]->setCropSize(cropSize);
    settings[e]->setZoomSize(zoomSize);
    settings[e]->calculateBaseInv();
}

void ViewSettings::setBkgdColor(eViewType e, QColor color)
{
    settings[e]->setBkgdColor(color);
    Configuration::getInstance()->viewColors[static_cast<int>(e)] = color.name();
}

