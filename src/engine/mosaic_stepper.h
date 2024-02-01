#ifndef MOSAIC_STEPPING_ENGINE_H
#define MOSAIC_STEPPING_ENGINE_H

#include "engine/stepping_engine.h"

class MosaicStepper : public SteppingEngine
{
public:
    MosaicStepper(ImageEngine * parent);
    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;
};


#endif


