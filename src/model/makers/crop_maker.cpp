#include "gui/top/system_view.h"
#include "gui/viewers/crop_maker_view.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "sys/geometry/crop.h"
#include "sys/sys.h"

QString sCropMaker[] =
{
    "Not in use",
    "Mosaic Crop",
    "Painter Crop",
    "Map Editor Crop"
};

////////////////////////////////////////////////////////////////////////////
//
//  Crop Maker
//
////////////////////////////////////////////////////////////////////////////

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

CropPtr CropMaker::createCrop()
{
    // this sets up a default crop
    QRectF srect    = Sys::systemViewController->getViewRect();
    QPointF scenter = srect.center();
    QSizeF ssz      = srect.size();
    ssz            *= 0.8;
    srect.setSize(ssz);
    srect.moveCenter(scenter);

    QRectF mrect    = Sys::cropMakerView->screenToModel(srect);
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

////////////////////////////////////////////////////////////////////////////
//
//  Mosaic Crop Maker
//
////////////////////////////////////////////////////////////////////////////

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

CropPtr MosaicCropMaker::getCrop()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);
    return mosaic->getCrop();
}

////////////////////////////////////////////////////////////////////////////
//
//  Painter Crop Maker
//
////////////////////////////////////////////////////////////////////////////

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
    Q_ASSERT(mosaic);
    return mosaic->getPainterCrop();
}
