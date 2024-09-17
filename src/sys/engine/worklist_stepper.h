#ifndef WORKLIST_STEPPING_ENGINE_H
#define WORKLIST_STEPPING_ENGINE_H

#include "sys/engine/stepping_engine.h"
#include "sys/sys/versioning.h"

class WorklistBMPStepper : public SteppingEngine
{
public:
    WorklistBMPStepper(ImageEngine * parent);

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


