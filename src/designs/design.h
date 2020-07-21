/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DESIGN_H
#define DESIGN_H

#include <QtCore>
#include <QtGui>
#include "base/shared.h"
#include "base/canvas_settings.h"

class Configuration;
class Workspace;
class Pattern;
class Border;
class Canvas;



class Design
{
public:
    virtual ~Design();

    virtual void    init();
    virtual bool    build() = 0;
    virtual void    repeat();
    void            destoryPatterns();

    void            updateDesign();

    QString         getTitle() { return title; }
    CanvasSettings &    getDesignInfo() { return info; }

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

    Configuration * config;
    View          * view;
    Workspace     * workspace;

    QVector<PatternPtr> patterns;

    eDesign        _design;

    bool            visible;

    CanvasSettings      info;
};


#endif // DESIGN_H
