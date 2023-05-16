#pragma once
#ifndef DESIGN_H
#define DESIGN_H

#include "enums/edesign.h"
#include "settings/model_settings.h"

class Configuration;

typedef std::shared_ptr<class Pattern>       PatternPtr;
typedef std::shared_ptr<class LegacyBorder>  LegacyBorderPtr;

class Design
{
public:
    virtual ~Design();

    virtual void    init();
    virtual bool    build() = 0;
    virtual void    repeat();
    void            destoryPatterns();

    void            updateDesign();

    QString          getTitle() { return title; }
    ModelSettings &  getDesignInfo() { return settings; }

    QVector<PatternPtr> &      getPatterns()      { return patterns; }

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
    QString         getDesignName() {return sDesign2[_design];}
    static QString  getDesignName(eDesign des) {return sDesign2[des];}

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
    class View       * view;

    QVector<PatternPtr> patterns;

    eDesign        _design;

    bool            visible;

    ModelSettings  settings;
};


#endif // DESIGN_H
