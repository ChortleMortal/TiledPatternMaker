#include "makers/crop_maker/crop_maker.h"
#include "geometry/crop.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/sys.h"
#include "mosaic/mosaic.h"
#include "viewers/crop_view.h"
#include "viewers/view.h"

CropMaker::CropMaker()
{
    mosaicMaker = MosaicMaker::getInstance();
    cropViewer  = CropViewer::getInstance();
}

CropPtr CropMaker::getCrop()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    CropPtr   crop;
    if (mosaic)
    {
        crop = mosaic->getCrop();
    }

    return crop;
}

bool CropMaker::setEmbed(bool state)
{
    auto crop = getCrop();
    if (!crop)
        return false;

    crop->setEmbed(state);

    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetProtoMaps();

    return true;
}

bool CropMaker::setApply(bool state)
{
    auto crop = getCrop();
    if (!crop)
        return false;

    crop->setApply(state);

    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetProtoMaps();
    return true;
}

CropPtr CropMaker::createCrop()
{
    // this sets up a default crop
    View  * view   = Sys::view;
    QRectF rect    = view->rect();
    QPointF center = rect.center();
    QSizeF sz      = rect.size();
    sz            *= 0.8;
    rect.setSize(sz);
    rect.moveCenter(center);
    rect = cropViewer->screenToWorld(rect);

    // create crop
    CropPtr crop = std::make_shared<Crop>();

    // default polygon
    crop->setPolygon(8,1.0,0.0);
    Circle ac(center,10.0);
    crop->setCircle(ac);
    crop->setAspect(ASPECT_UNCONSTRAINED);
    crop->setAspectVertical(false);
    // this being last makes it a rectanglular crop
    crop->setRect(rect);
    return crop;
}

void CropMaker::setCrop(CropPtr crop)
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->setCrop(crop);
}

void CropMaker::removeCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetCrop();
}
