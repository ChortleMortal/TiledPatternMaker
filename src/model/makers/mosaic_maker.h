#pragma once

#include <QMessageBox>
#include <QObject>
#include <QVector>
#include <QColor>

#include "sys/enums/estyletype.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Style>            StylePtr;

class LoadUnit;
class MosaicEvent;

extern const QString sMOSM_Events[];

class MosaicMaker : public QObject
{
    Q_OBJECT

public:

    // Mosaic Maker state machine events
    enum class eMosiacMakerEvent
    {
        LoadEmpty = 0,
        LoadProtoSingle,
        LoadProtoMulti,
        ReloadProtoSingle,
        ReloadProtoMulti,
        ProtoDeleted,
        ProtoChanged,
    };

    using Event = eMosiacMakerEvent;

    MosaicMaker();
    ~MosaicMaker();
    void        init();

    MosaicPtr   load(VersionedFile &file);
    bool        save(VersionedFile mosaicFile, QString & failMsg, bool cleanseMaps, QStringList & unverifiedMaps);

    // state machine
    void        sm_takeDown(MosaicPtr mosaic);
    void        sm_takeUp(MosaicEvent & mosEvent);
    void        sm_resetStyles();

    // setters/getters
    MosaicPtr   getMosaic();
    void        resetMosaic();
    void        setBkgdColor(QColor color);     // the mosaic's and the configured mosaic color
    LoadUnit *  getLoadUnit()   {return loadUnit; }
    void        reload();

    // operations
    StylePtr    makeStyle(StylePtr style, eStyleType newType);

signals:
    void        sig_mosaicLoading(VersionedFile vfile);
    void        sig_mosaicLoaded( VersionedFile vfile);   // a new/different mosaic is now in memory
    void        sig_mosaicWritten(VersionedFile vfile);   // the current in-memory mosaic was saved (data unchanged)
    void        sig_updateView();
    void        sig_reconstructView();
    void        sig_messageBox(QString msg, QString info, QMessageBox::Icon icon, bool modal);
    void        sig_interlaceUnwoven();

protected:
    MosaicPtr   loadMosaic(VersionedFile vfile, QString & failMsg, QString & warnMsg);

    void        sm_createMosaic(ProtoPtr prototype);
    void        sm_addPrototype(ProtoPtr prototype);
    void        sm_removePrototype(ProtoPtr prototype);
    void        sm_replacePrototype(const QVector<ProtoPtr> & oldPrototypes, ProtoPtr newPrototype);

private:
    MosaicPtr   _mosaic;

    LoadUnit  * loadUnit;
};

class MosaicEvent
{
public:
    using Event = MosaicMaker::Event;

    Event               event;
    ProtoPtr            prototype;
    QVector<ProtoPtr>   oldPrototypes;  // used by ReloadProtoMulti to know which styles to repoint
};