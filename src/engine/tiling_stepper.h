#ifndef TILING_STEPPING_ENGINE_H
#define TILING_STEPPING_ENGINE_H

#include "engine/stepping_engine.h"

class TilingStepper : public SteppingEngine
{
public:

public:
    TilingStepper(ImageEngine * parent);
    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;

protected:
    void loadTiling(QString name);
};

#endif


