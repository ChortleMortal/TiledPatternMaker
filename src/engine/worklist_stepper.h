#ifndef WORKLIST_STEPPING_ENGINE_H
#define WORKLIST_STEPPING_ENGINE_H

#include "engine/stepping_engine.h"
#include "widgets/versioned_list_widget.h"

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
    void resync(QString name);

protected:

    QStringList           imgList;
    QStringList::iterator imgList_it;
};

#endif


