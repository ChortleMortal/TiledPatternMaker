#pragma once
#ifndef DESIGNCONTROL_H
#define DESIGNCONTROL_H

#include <memory>
#include <QObject>
#include <QMap>
#include "enums/edesign.h"

typedef std::shared_ptr<class Design> DesignPtr;

class DesignMaker : public QObject
{
    Q_OBJECT

public:
    static DesignMaker * getInstance();
    static void            releaseInstance();

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


signals:
    void sig_loadedDesign(eDesign design);

public slots:
    void slot_loadDesign(eDesign design);
    void slot_buildDesign(eDesign design);

    void designScale(int delta);
    void designRotate(int delta);
    void designMoveY(int delta);
    void designMoveX(int delta);

private slots:

private:
    DesignMaker();

    static DesignMaker  * mpThis;
    class  ViewControl  * view;
    class  Configuration* config;

    QMap<eDesign,DesignPtr>     availableDesigns;
    QVector<DesignPtr>          activeDesigns;
    QString                     designName;

    int stepsTaken;
    int selectedLayer;
};

#endif // DESIGNCONTROL_H
