#include "model/makers/crop_maker.h"
#include "sys/geometry/crop.h"
#include "model/makers/mosaic_maker.h"
#include "sys/sys.h"
#include "model/mosaics/mosaic.h"
#include "gui/viewers/crop_viewer.h"
#include "gui/top/system_view.h"

QString sCropMaker[] =
{
    "UNDEFINED",
    "MOSAIC",
    "PAINTER",
    "MAP EDITOR"
};

CropMaker::CropMaker()
{
    mosaicMaker = Sys::mosaicMaker;
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

bool CropMaker::setCropOutside(bool state)
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

bool CropMaker::setClip(bool state)
{
    auto crop = getCrop();
    if (!crop)
        return false;

    crop->setClip(state);

    return true;
}

CropPtr CropMaker::createCrop()
{
    // this sets up a default crop
    QRectF srect    = Sys::viewController->viewRect();
    QPointF scenter = srect.center();
    QSizeF ssz      = srect.size();
    ssz            *= 0.8;
    srect.setSize(ssz);
    srect.moveCenter(scenter);

    QRectF mrect    = Sys::cropViewer->screenToModel(srect);
    QPointF mcenter = mrect.center();

    // create crop
    CropPtr crop = std::make_shared<Crop>();

    // default polygon
    APolygon p(8,0.0,1.0);
    p.setPos(mcenter);
    crop->setPolygon(p);

    Circle ac(mcenter,1.0);
    crop->setCircle(ac);

    crop->setAspect(ASPECT_UNCONSTRAINED);
    crop->setAspectVertical(false);

    // this being last makes it a rectanglular crop
    crop->setRect(mrect);
    return crop;
}

void MosaicCropMaker::setCrop(CropPtr crop)
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->setCrop(crop);
}

void MosaicCropMaker::removeCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetCrop();
}

void MosaicCropMaker::removePainterCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetPainterCrop();
}

CropPtr MosaicCropMaker::getCrop()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    CropPtr   crop;
    if (mosaic)
    {
        crop = mosaic->getCrop();
    }

    return crop;
}

void PainterCropMaker::setCrop(CropPtr crop)
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->setPainterCrop(crop);
}

void PainterCropMaker::removeCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    mosaic->resetPainterCrop();
}

CropPtr PainterCropMaker::getCrop()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    CropPtr   crop;
    if (mosaic)
    {
        crop = mosaic->getPainterCrop();
    }

    return crop;
}
