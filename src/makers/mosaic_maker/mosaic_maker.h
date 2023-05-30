#pragma once
#ifndef MOSAICMAKER_H
#define MOSAICMAKER_H

#include <memory>
#include <QObject>
#include "enums/estyletype.h"
#include "enums/estatemachineevent.h"
#include "settings/model_settings.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Style>            StylePtr;

class MosaicMaker : public QObject
{
    Q_OBJECT
public:
    static MosaicMaker * getInstance();
    static void          releaseInstance();

    void           init();

    // state machine
    void           sm_takeDown(MosaicPtr mosaic);
    void           sm_takeUp(QVector<ProtoPtr> prototypes, eMOSM_Event event);

    // setters/getters
    MosaicPtr      getMosaic();
    void           resetMosaic();
    ModelSettings& getMosaicSettings();

    // operations
    StylePtr       makeStyle(eStyleType type, StylePtr oldStyle);

signals:
    void        sig_mosaicLoaded(QString name);
    void        sig_mosaicWritten();
    void        sig_cycler_ready();

public slots:
    void slot_loadMosaic(QString name, bool ready);
    void slot_cycleLoadMosaic(QString name);
    void slot_saveMosaic(QString name);

protected:
    void sm_resetStyles();
    void sm_createMosaic(const QVector<ProtoPtr> prototypes);
    void sm_addPrototype(const QVector<ProtoPtr> prototypes);
    void sm_replacePrototype(ProtoPtr prototype);

private:
    MosaicMaker();
    static MosaicMaker * mpThis;

    class PrototypeMaker * prototypeMaker;
    class ViewControl    * viewControl;
    class Configuration  * config;
    class ControlPanel   * controlPanel;
    class TilingMaker    * tilingMaker;

    MosaicPtr           _mosaic;
};

#endif
