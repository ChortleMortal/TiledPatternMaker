#pragma once
#ifndef MOSAICMAKER_H
#define MOSAICMAKER_H

#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "sys/enums/estyletype.h"
#include "sys/enums/estatemachineevent.h"
#include "model/settings/canvas_settings.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Style>            StylePtr;

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
    void        sm_takeUp(QVector<ProtoPtr> prototypes, eMOSM_Event event);

    // setters/getters
    MosaicPtr   getMosaic();
    void        resetMosaic();
    
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
    void        sm_resetStyles();
    void        sm_createMosaic(const QVector<ProtoPtr> prototypes);
    void        sm_addPrototype(const QVector<ProtoPtr> prototypes);
    void        sm_removePrototype(const QVector<ProtoPtr> prototypes);
    void        sm_replacePrototype(ProtoPtr prototype);

private:
    class PrototypeMaker * prototypeMaker;
    class ViewController * viewControl;
    class Configuration  * config;
    class ControlPanel   * controlPanel;
    class TilingMaker    * tilingMaker;

    MosaicPtr           _mosaic;
};

#endif
