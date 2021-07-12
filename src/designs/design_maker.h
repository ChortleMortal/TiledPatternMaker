#ifndef DESIGNCONTROL_H
#define DESIGNCONTROL_H

#include <QtCore>

typedef std::shared_ptr<class Design> DesignPtr;

class DesignMaker : public QObject
{
    Q_OBJECT

public:
    static DesignMaker * getInstance();
    static void            releaseInstance();

    // designs
    void                addDesign(DesignPtr d);
    QVector<DesignPtr>& getDesigns();
    QString             getDesignName() { return designName; }
    void                clearDesigns();

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

public slots:
    void designScale(int delta);
    void designRotate(int delta);
    void designMoveY(int delta);
    void designMoveX(int delta);

    void setMaxStep(int max);

private slots:

private:
    DesignMaker();

    static DesignMaker  * mpThis;
    class  View         * view;
    class Configuration * config;

    QVector<DesignPtr>          activeDesigns;
    QString                     designName;

    int maxStep;
    int stepsTaken;
    int selectedLayer;};

#endif // DESIGNCONTROL_H
