#ifndef VIEW_BMP_STEPPER_H
#define VIEW_BMP_STEPPER_H

#include "sys/engine/stepping_engine.h"
#include "sys/sys/versioning.h"

class ViewBMPStepper : public SteppingEngine
{
public:
    ViewBMPStepper(ImageEngine * parent);

    virtual bool begin() override;
    virtual bool tick()  override;
    virtual bool next()  override;
    virtual bool prev()  override;
    virtual bool end()   override;

    void setWorklist(VersionList & vlist);
    void resync(VersionedName name);

protected:
    VersionList                      imgList;
    QVector<VersionedName>::iterator imgList_it;
};

#endif


