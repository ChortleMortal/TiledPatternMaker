#pragma once
#ifndef CROPMAKER_H
#define CROPMAKER_H

#include<memory>

typedef std::shared_ptr<class Crop>   CropPtr;

class CropMaker
{
public:
    CropMaker();

    virtual CropPtr getCrop();
    virtual void    setCrop(CropPtr crop);
    virtual void    removeCrop();

    CropPtr createCrop();

    bool    setEmbed(bool state);
    bool    setApply(bool state);

protected:

private:
    class CropViewer    * cropViewer;
    class MosaicMaker   * mosaicMaker;

};

#endif
