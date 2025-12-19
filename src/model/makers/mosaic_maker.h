#pragma once
#ifndef MOSAICMAKER_H
#define MOSAICMAKER_H

#include <QObject>

#include "model/settings/canvas_settings.h"
#include "sys/enums/estatemachineevent.h"
#include "sys/enums/estyletype.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Style>            StylePtr;

class LoadUnit;

class MosaicMaker : public QObject
{
    Q_OBJECT

public:
    MosaicMaker();
    ~MosaicMaker();
    void        init();

    MosaicPtr   loadMosaic(VersionedFile &file);
    bool        saveMosaic(MosaicPtr mosaic, bool forceOverwrite);

    // state machine
    void        sm_takeDown(MosaicPtr mosaic);
    void        sm_takeUp(MosaicEvent & mosEvent);
    void        sm_resetStyles();

    // setters/getters
    MosaicPtr   getMosaic();
    void        resetMosaic();
    LoadUnit *  getLoadUnit()   {return loadUnit; }
    void        reload();

    void        setCanvasSettings(CanvasSettings & ms);
    CanvasSettings & getCanvasSettings();

    // operations
    StylePtr    makeStyle(eStyleType type, StylePtr oldStyle);

signals:
    void        sig_mosaicLoaded(VersionedFile vfile);
    void        sig_mosaicWritten();
    void        sig_updateView();
    void        sig_reconstructView();

protected:
    void        sm_createMosaic(ProtoPtr prototype);
    void        sm_addPrototype(ProtoPtr prototype);
    void        sm_removePrototype(ProtoPtr prototype);
    void        sm_replacePrototype(ProtoPtr prototype);

private:
    MosaicPtr   _mosaic;

    LoadUnit  * loadUnit;
};

#endif
