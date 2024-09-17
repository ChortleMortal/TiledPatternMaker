#pragma once
#ifndef DESIGN_H
#define DESIGN_H

#include <QObject>
#include "sys/enums/edesign.h"
#include "model/settings/canvas_settings.h"

class Configuration;

typedef std::shared_ptr<class Pattern>       PatternPtr;
typedef std::shared_ptr<class LegacyBorder>  LegacyBorderPtr;

class Design : public QObject
{
    Q_OBJECT

public:
    virtual ~Design();

    virtual void    init();
    virtual bool    build() = 0;
    virtual void    repeat();
    void            destoryPatterns();

    QString         getTitle() { return title; }

    CanvasSettings &  getDesignInfo() { return settings; }
    void            setDesignInfo(const CanvasSettings & ms) { settings = ms; }

    QVector<PatternPtr> &   getPatterns()      { return patterns; }

    void            doSteps(int maxIndex = 100);
    bool            doStep();
    bool            step(int index);
    int             getStep();
    void            setStep(int step);
    void            deltaStep(int delta);

    virtual void    showLayer(int layerNum);
    virtual void    hideLayer(int layerNum);

    bool            isVisible() { return visible; }
    void            setVisible(bool visible);

    void            zPlus(int layerNum);
    void            zMinus(int layerNum);

    eDesign         getDesign() { return _design; }
    QString         getDesignName() {return designs[_design];}
    static QString  getDesignName(eDesign des) {return designs[des];}

    qreal           getXseparation() { return xSeparation; }
    qreal           getYseparation() { return ySeparation; }
    void            setXseparation(qreal sep) { xSeparation = sep; }
    void            setYseparation(qreal sep) { ySeparation = sep; }

    qreal           getXoffset2() { return xOffset2; }
    qreal           getYoffset2() { return yOffset2; }
    void            setXoffset2(qreal offset) { xOffset2 = offset; }
    void            setYoffset2(qreal offset) { yOffset2 = offset; }

    static int      refs;

    LegacyBorderPtr  border;

signals:
    void            sig_updateView();

protected:
    Design(eDesign design, QString title);

    void    RepeatSimples();
    void    RepeatHexagons();
    void    RepeatOctagonsFilled();

    void    LocSimple(PatternPtr t);
    void    LocHexagon(PatternPtr t);
    void    LocOctagonFilled(PatternPtr t);

    QString title;

    int             rows;
    int             cols;
    int             currentStep;

    qreal           xSeparation;
    qreal           ySeparation;
    qreal           xOffset2;
    qreal           yOffset2;

    Configuration    * config;

    QVector<PatternPtr> patterns;

    eDesign        _design;

    bool            visible;
    
    CanvasSettings  settings;
};


#endif // DESIGN_H
