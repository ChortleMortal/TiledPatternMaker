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

#include "base/border.h"
#include "base/view.h"
#include "base/canvas_settings.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "designs/design.h"
#include "designs/patterns.h"

#define Enum2Str(e)  {QString(#e)}

int Design::refs = 0;

Q_DECLARE_METATYPE(eDesign)


//////////////////////////////////////////////////////////////////////////////////
/// Design
//////////////////////////////////////////////////////////////////////////////////

Design::Design(eDesign design, QString title)
{
    _design         = design;
    //qDebug() << "creating design" << sDesign[_design];
    refs++;

    this->title = title;

    view        = View::getInstance();
    config      = Configuration::getInstance();
    workspace   = Workspace::getInstance();

    init();
}

Design::~Design()
{
    //qDebug() << "deleting design" << sDesign[_design];
    refs--;
    patterns.clear();
}

void Design::init()
{
    rows            = 0;
    cols            = 0;
    xOffset2        = 0;
    yOffset2        = 0;
    xSeparation     = 0;
    ySeparation     = 0;
    currentStep     = 0;
    visible         = true;

    destoryPatterns();

    info.clear();

    //qDebug().noquote() << "Desgin::init" << sDesign2[_design];
}

void Design::destoryPatterns()
{
    patterns.clear();
}

void Design::repeat()
{
    switch(_design)
    {
    case DESIGN_13:
    case DESIGN_14:
    case DESIGN_18:
        RepeatHexagons();
        break;
    case DESIGN_HU_INSERT:
        RepeatOctagonsFilled();
        break;
    default:
        RepeatSimples();
        break;
    }
}



void Design::updateDesign()
{
    view->update();
}



void Design::showLayer(int layerNum)
{
    qDebug() << "show layer"  << layerNum;
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LayerPtr layer = t->geSubLayer(layerNum);
        if (layer)
        {
            layer->setVisible(true);
        }
    }
    view->update();
}

void Design::hideLayer(int layerNum)
{
    qDebug() << "hide layer"  << layerNum;
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LayerPtr layer = t->geSubLayer(layerNum);
        if (layer)
        {
            layer->setVisible(false);
        }
    }
    view->update();
}

void Design::setVisible(bool visible)
{
    qDebug() << "visible" << visible;

    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        QVector<LayerPtr> & layers = t->getSubLayers();
        for (auto layer : layers)
        {
            layer->setVisible(visible);
        }
    }

    this->visible = visible;

    view->update();
}

void  Design::zPlus(int layerNum)
{
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LayerPtr layer = t->geSubLayer(layerNum);
        if (layer)
        {
            int zlevel = layer->zValue() + 1;
            layer->setZValue(zlevel);
            qDebug() << layerNum << "z-level:" << layer->zValue();
        }
    }
}

void  Design::zMinus(int layerNum)
{
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LayerPtr layer = t->geSubLayer(layerNum);
        if (layer)
        {
            int zlevel = layer->zValue() - 1;
            layer->setZValue(zlevel);
            qDebug() << layerNum << "z-level:" << layer->zValue();
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
//
// The Repeats
//
/////////////////////////////////////////////////////////////////////////////

void Design::RepeatSimples()
{
    // don't move the canvas tile

    //qDebug() << "Repeat Simple";
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LocSimple(t);
    }
}

void Design::LocSimple(PatternPtr pp)
{
    QPointF pt = info.getStartTile();
    pp->setLoc(QPointF(pt.x() + (xSeparation * pp->getCol()),
                       pt.y() + (ySeparation * pp->getRow())));
    //qDebug() << "LocSimple" << pp->getLoc();
}

void Design::RepeatHexagons()
{
    // don't move the canvas tile
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LocHexagon(t);
    }
}

void Design::LocHexagon(PatternPtr t)
{
    QPointF pt = info.getStartTile();
    //qDebug() << t->getRow() << t->getCol() << xSeparation << ySeparation << xOffset2 << yOffset2;
    t->setLoc(QPointF(pt.x() + (xSeparation * t->getCol()) + (((xSeparation/2.0) + xOffset2) * (t->getRow() & 0x01)),
                      pt.y() + (ySeparation * t->getRow()) + (yOffset2 * (t->getCol() & 1))));
    //qDebug() << "repeat hex loc:" << t->getLoc();
}

void Design::RepeatOctagonsFilled()
{
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        LocOctagonFilled(t);
    }
}

void Design::LocOctagonFilled(PatternPtr t)
{
    QPointF pt = info.getStartTile();
    t->setLoc(QPointF(pt.x() + (xSeparation * t->getCol()) - (xSeparation/2.0),
                      pt.y() + (ySeparation * t->getRow()) - (ySeparation/2.0)));
}


////////////////////////////////////////////////////////////
//
//  Steps
//
////////////////////////////////////////////////////////////


void Design::doSteps(int maxStep)
{
    for (int i=0; i < maxStep; i++)
    {
        bool rv = step(i);
        if (!rv)
        {
            return;
        }
     }
}

bool Design::doStep()
{
    return step(currentStep);
}

bool Design::step(int index)
{
    bool rv = true;
    for (auto it = patterns.begin(); it < patterns.end(); it++)
    {
        PatternPtr t = *it;
        rv = t->doStep(index);
    }
    return rv;      // all tiles return the same value
}

int  Design::getStep()
{
    return currentStep;
}

void Design::setStep(int step)
{
    currentStep = step;
}

void Design::deltaStep(int delta)
{
    currentStep += delta;
    if (currentStep < 0)
    {
        currentStep = 0;
    }
}
