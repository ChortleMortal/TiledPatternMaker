#pragma once

#include <QObject>

enum eCropMaker
{
    CM_UNDEFINED,
    CM_MOSAIC,
    CM_PAINTER,
    CM_MAPED
};

extern QString sCropMaker[];

typedef std::shared_ptr<class Crop>   CropPtr;

////////////////////////////////////////////////////////////////////////////
//
//  Crop Maker
//
////////////////////////////////////////////////////////////////////////////

class CropMaker
{
public:
    CropMaker();

    virtual CropPtr getCrop() = 0;
    virtual void    setCrop(CropPtr crop) = 0;
    virtual void    removeCrop() = 0;

    CropPtr createCrop();

    bool    setEmbed(bool state);
    bool    setCropOutside(bool state);

protected:
    class   MosaicMaker * mosaicMaker;

};

////////////////////////////////////////////////////////////////////////////
//
//  Mosaic Crop Maker
//
////////////////////////////////////////////////////////////////////////////

class MosaicCropMaker : public CropMaker
{
public:
    CropPtr getCrop();
    void    setCrop(CropPtr crop);
    void    removeCrop();
};

////////////////////////////////////////////////////////////////////////////
//
//  Painter Crop Maker
//
////////////////////////////////////////////////////////////////////////////

class PainterCropMaker : public CropMaker
{
public:
    CropPtr getCrop();
    void    setCrop(CropPtr crop);
    void    removeCrop();
};
