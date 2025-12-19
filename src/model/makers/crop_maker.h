#pragma once
#ifndef CROPMAKER_H
#define CROPMAKER_H

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
    bool    setClip(bool state);

protected:
    class   MosaicMaker * mosaicMaker;

};

class MosaicCropMaker : public CropMaker
{
public:
    CropPtr getCrop();
    void    setCrop(CropPtr crop);
    void    removeCrop();
    void    removePainterCrop();
};

class PainterCropMaker : public CropMaker
{
public:
    CropPtr getCrop();
    void    setCrop(CropPtr crop);
    void    removeCrop();
};


#endif
