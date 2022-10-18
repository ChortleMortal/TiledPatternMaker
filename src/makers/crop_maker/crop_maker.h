#ifndef CROPMAKER_H
#define CROPMAKER_H

#include <memory>
#include <QPointF>
#include <QPainter>

typedef std::shared_ptr<class Crop>  CropPtr;
typedef std::shared_ptr<class Map>   MapPtr;
typedef std::shared_ptr<class CropView>   CropViewPtr;

typedef std::weak_ptr<class Crop>    WeakCropPtr;

class MosaicMaker;

enum eCropMakerState
{
    CROPMAKER_STATE_INACTIVE,
    CROPMAKER_STATE_ACTIVE,
    CROPMAKER_STATE_COMPLETE
};

extern QString sCropMakerState[];

class CropMaker
{
public:
    static CropMaker * getInstance();

    CropPtr loadCrop();
    void    unloadCrop();

    CropPtr createCrop();
    CropPtr getCrop();
    void    removeCrop();
    bool    embedCrop(MapPtr map);
    bool    cropMap(MapPtr map);

    void    setActiveCrop(CropPtr crop) { activeCrop = crop;  localCrop.reset(); }
    CropPtr getActiveCrop()             { return activeCrop; }
    void    setLocalCrop(CropPtr crop)  { localCrop = crop; activeCrop = crop; }
    CropPtr getLocalCrop()              { return localCrop; }

    eCropMakerState getState() { return _state; }
    void            setState(eCropMakerState state);

protected:
    CropMaker();


private:
    static CropMaker * mpThis;
    CropViewPtr         crview;
    MosaicMaker *       mosaicMaker;

    eCropMakerState     _state;
    CropPtr             activeCrop;     // this is the crop being used (not owned)
    CropPtr             localCrop;      // this is a crop made here (owned)

};

#endif
