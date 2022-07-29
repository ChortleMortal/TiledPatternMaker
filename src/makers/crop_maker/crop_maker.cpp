#include "makers/crop_maker/crop_maker.h"
#include "geometry/crop.h"
#include "geometry/map.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "mosaic/mosaic.h"
#include "viewers/crop_view.h"
#include "viewers/viewcontrol.h"

CropMaker * CropMaker::mpThis = nullptr;

QString sCropMakerState[] =
{
    "INACTIVE",
    "ACTIVE",
    "COMPLETE"
};

CropMaker * CropMaker::getInstance()
{
    if (!mpThis)
    {
        mpThis = new CropMaker();
    }
    return mpThis;
}

CropMaker::CropMaker()
{
    mosaicMaker = MosaicMaker::getInstance();
    crview      = CropView::getSharedInstance();
    crview->init(this);

    setState(CROPMAKER_STATE_INACTIVE);
}

CropPtr CropMaker::getCrop()
{
    return getActiveCrop();
}

CropPtr CropMaker::loadCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    CropPtr crop;
    if (mosaic)
    {
        crop = mosaic->getCrop();
    }

    if (crop)
    {
        // the crop is associated with the loaded mosaic
        setActiveCrop(crop);
        setState(CROPMAKER_STATE_ACTIVE);
    }
    else
    {
        setState(CROPMAKER_STATE_INACTIVE);
    }

    return crop;
}

CropPtr CropMaker::createCrop()
{
    // this sets up a default crop
    ViewControl  * view = ViewControl::getInstance();
    QRectF rect    = view->rect();
    QPointF center = rect.center();
    QSizeF sz      = rect.size();
    sz            *= 0.8;
    rect.setSize(sz);
    rect.moveCenter(center);
    rect = crview->screenToWorld(rect);

    // create ctop
    CropPtr crop = std::make_shared<Crop>();

    // default polygon
    crop->setPolygon(8,1.0,0.0);
    crop->setCircle(std::make_shared<Circle>(center,10.0));
    crop->setAspect(ASPECT_UNCONSTRAINED);
    crop->setAspectVertical(false);
    // this being last makes it a rectanglular crop
    crop->setRect(rect);

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        mosaic->initCrop(crop);
        setActiveCrop(crop);
    }
    else
    {
        setLocalCrop(crop);
    }

    setState(CROPMAKER_STATE_ACTIVE);

    return crop;
}

void CropMaker::removeCrop()
{
    CropPtr nullCrop;
    setActiveCrop(nullCrop);    // resets local crop too

    auto mosaic = MosaicMaker::getInstance()->getMosaic();
    if (mosaic)
    {
        mosaic->resetCrop(nullCrop);
    }

    setState(CROPMAKER_STATE_INACTIVE);
}

// merges the crop into the map - called by map editor
bool CropMaker::embedCrop(MapPtr map)
{
    Q_ASSERT(getState() == CROPMAKER_STATE_ACTIVE);

    if (!map)
        return false;

    auto crop = getActiveCrop();
    if (!crop)
        return false;

    qDebug() << "embed crop in map";

    switch (crop->getCropType())
    {
    case CROP_RECTANGLE:
        map->embedCrop(crop->getRect());
        break;

    case CROP_CIRCLE:
        map->embedCrop(crop->getCircle());
        break;

    case CROP_POLYGON:
    case CROP_UNDEFINED:
        qWarning() << "not implemented";
        return false;
    }

    crop->embed();
    if (crop->isUsed())
    {
        setState(CROPMAKER_STATE_COMPLETE);
    }

    return true;
}

// deletes everything outside of the crop rectangle - called by map editgor
bool CropMaker::cropMap(MapPtr map)
{
    Q_ASSERT(getState() == CROPMAKER_STATE_ACTIVE);

    if (!map)
        return false;

    CropPtr crop = getActiveCrop();
    if (!crop)
        return false;

    qDebug() << "apply crop to map";

    switch (crop->getCropType())
    {
    case CROP_RECTANGLE:
        map->cropOutside(crop->getRect());
        break;
    case CROP_CIRCLE:
        map->cropOutside(crop->getCircle());
        break;
    case CROP_POLYGON:
        map->cropOutside(crop->getPolygon());
        break;
    case CROP_UNDEFINED:
        qWarning() << "Crop Not defined";
        return false;
    }

    crop->apply();
    if (crop->isUsed())
    {
        setState(CROPMAKER_STATE_COMPLETE);
    }
    return true;
}

void CropMaker::setState(eCropMakerState state)
{
    _state = state;
}

