#pragma once
#ifndef MOSAICMAKER_H
#define MOSAICMAKER_H

#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "enums/estyletype.h"
#include "enums/estatemachineevent.h"
#include "settings/canvas_settings.h"
#include "settings/filldata.h"

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

    MosaicPtr   loadMosaic(QString name);
    bool        saveMosaic(QString name, bool forceOverwrite);

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
    void        sig_mosaicLoaded(QString name);
    void        sig_mosaicWritten();

public slots:

protected:
    void        sm_resetStyles();
    void        sm_createMosaic(const QVector<ProtoPtr> prototypes);
    void        sm_addPrototype(const QVector<ProtoPtr> prototypes);
    void        sm_replacePrototype(ProtoPtr prototype);

private:
    class PrototypeMaker * prototypeMaker;
    class ViewController * viewControl;
    class View           * view;
    class Configuration  * config;
    class ControlPanel   * controlPanel;
    class TilingMaker    * tilingMaker;

    MosaicPtr           _mosaic;
};

#endif
