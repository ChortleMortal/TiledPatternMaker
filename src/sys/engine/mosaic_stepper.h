#pragma once

#include "sys/engine/stepping_engine.h"

class MosaicStepper : public SteppingEngine
{
public:
    MosaicStepper(ImageEngine * parent);
    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;

    bool do_immediate();
    void setImmediat(bool set) { mImmediate = set; }

protected:
    bool    mImmediate;
};


