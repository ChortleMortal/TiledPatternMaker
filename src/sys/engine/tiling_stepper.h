#ifndef TILING_STEPPING_ENGINE_H
#define TILING_STEPPING_ENGINE_H

#include "sys/engine/stepping_engine.h"

class TilingStepper : public SteppingEngine
{
public:
    TilingStepper(ImageEngine * parent);
    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;
};

#endif


