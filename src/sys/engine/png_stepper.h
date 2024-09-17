#ifndef PNG_STEPPING_ENGINE_H
#define PNG_STEPPING_ENGINE_H

#include "sys/engine/stepping_engine.h"

class PNGStepper : public SteppingEngine
{
    Q_OBJECT

public:
    PNGStepper(ImageEngine * parent);
    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;

signals:
    void cycle_sig_unloadView();

protected:
    void cycleShowPNG(VersionedName file, int row, int col);

private:
    int             pngRow;
    int             pngCol;
    int             pngIndex;

};

#endif


