#pragma once
#ifndef DESIGNCONTROL_H
#define DESIGNCONTROL_H

#include <QObject>
#include <QMap>
#include "sys/enums/edesign.h"

extern const QString sLegacyMode[];

enum eLegacyMode
{
    LEGACY_MODE_DES_POS,
    LEGACY_MODE_DES_LAYER_SELECT,
    LEGACY_MODE_DES_ZLEVEL,
    LEGACY_MODE_DES_STEP,
    LEGACY_MODE_MODE_DES_SEPARATION,
    LEGACY_MODE_DES_ORIGIN,
    LEGACY_MODE_DES_OFFSET
};

class LoadUnit;

typedef std::shared_ptr<class Design> DesignPtr;

class DesignMaker : public QObject
{
    Q_OBJECT

public:
    DesignMaker();
    void init();

    // designs
    void                addDesign(DesignPtr d);
    QVector<DesignPtr>& getActiveDesigns()        { return activeDesigns; }
    QMap<eDesign,DesignPtr>& getAvailDesigns()    { return availableDesigns; }
    DesignPtr           getDesign(eDesign design) { return availableDesigns.value(design); }
    QString             getDesignName()           { return designName; }
    void                unload();

    void ProcKeyLeft();
    void ProcKeyRight();
    void ProcKeyDown();
    void ProcKeyUp();

    void designReposition(qreal, qreal);
    void designOffset(qreal, qreal);
    void designOrigin(int, int);
    void designLayerSelect(int);
    void designLayerZPlus();
    void designLayerZMinus();
    void designLayerShow();
    void designLayerHide();
    void designToggleVisibility(int design);

    void setStep(int step);
    bool step(int delta);       // from keyboard

    LoadUnit * getLoadUnit() { return loadUnit; }
    void reload();

    void     setLegacyKbdMode(eLegacyMode mode);
    bool     isLegacyKbdMode(eLegacyMode mode);
    eLegacyMode  getLegacyKbdMode() { return _legacyKeyboardMode; }
    QString  getLegacyKbdModeStr();
    void     resetLegacyKbdMode();

signals:
    void sig_loadedDesign(eDesign design);
    void sig_updateView();
    void sig_reconstructView();
    void sig_LegacyKbdMode(eLegacyMode);

public slots:
    void slot_loadDesign(eDesign design);
    void slot_buildDesign(eDesign design);

    void designScale(int delta);
    void designRotate(int delta);
    void designMoveY(int delta);
    void designMoveX(int delta);

private slots:

private:
    class  SystemViewController * viewControl;
    class  Configuration        * config;
    class  LoadUnit             * loadUnit;

    QMap<eDesign,DesignPtr>     availableDesigns;
    QVector<DesignPtr>          activeDesigns;
    QString                     designName;

    eLegacyMode                 _legacyKeyboardMode;
    int                         stepsTaken;
    int                         selectedLayer;
};

#endif // DESIGNCONTROL_H
