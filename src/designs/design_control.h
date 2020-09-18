#ifndef DESIGNCONTROL_H
#define DESIGNCONTROL_H

#include <QtCore>

class Workspace;
class WorkspaceViewer;
class View;
class Configuration;

class DesignControl : public QObject
{
    Q_OBJECT

public:
    static DesignControl * getInstance();
    static void            releaseInstance();

    void ProcKeyLeft( int delta, bool isALT);
    void ProcKeyRight(int delta, bool isALT);
    void ProcKeyDown( int delta, bool isALT);
    void ProcKeyUp(   int delta, bool isALT);

    void designReposition(qreal, qreal);
    void designOffset(qreal, qreal);
    void designOrigin(int, int);
    void designLayerSelect(int);
    void designLayerZPlus();
    void designLayerZMinus();
    void designLayerShow();
    void designLayerHide();
    void designToggleVisibility(int design);

    void designScale(int delta);
    void designRotate(int delta, bool cw);
    void designMoveY(int delta);
    void designMoveX(int delta);

    void stopTimer();
    void startTimer();

    void setStep(int step);
    bool step(int delta);       // from keyboard
    void setMaxStep(int max);

private slots:
    void slot_nextStep();   // from timer


private:
    DesignControl();

    static DesignControl * mpThis;
    QTimer            * timer;
    Workspace         * workspace;
    Configuration     * config;


    int maxStep;
    int stepsTaken;
    int selectedLayer;};

#endif // DESIGNCONTROL_H
