#ifndef DESIGNCONTROL_H
#define DESIGNCONTROL_H

#include <memory>
#include <QObject>

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

public slots:
    void designScale(int delta);
    void designRotate(int delta);
    void designMoveY(int delta);
    void designMoveX(int delta);

private slots:

private:
    DesignMaker();

    static DesignMaker  * mpThis;
    class  ViewControl  * view;

    QVector<DesignPtr>          activeDesigns;
    QString                     designName;

    int stepsTaken;
    int selectedLayer;};

#endif // DESIGNCONTROL_H
