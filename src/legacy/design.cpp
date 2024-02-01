#include "viewers/view.h"
#include "legacy/design.h"
#include "legacy/patterns.h"
#include "misc/sys.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include <QCoreApplication>

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QDebug>
#endif

int Design::refs = 0;

Q_DECLARE_METATYPE(eDesign)

extern class TiledPatternMaker * theApp;


//////////////////////////////////////////////////////////////////////////////////
/// Design
//////////////////////////////////////////////////////////////////////////////////

Design::Design(eDesign design, QString title)
{
    _design         = design;
    //qDebug() << "creating design" << sDesign[_design];
    refs++;

    this->title = title;

    config  = Configuration::getInstance();
    view    = Sys::view;

    Design::init();
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
        for (auto layer : std::as_const(layers))
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
    QPointF pt = settings.getStartTile();
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
    QPointF pt = settings.getStartTile();
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
    QPointF pt = settings.getStartTile();
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
    emit theApp->sig_refreshView();

    for (int i=0; i < maxStep; i++)
    {
        bool rv = step(i);
        if (!rv)
        {
            return;
        }
        view->update();
        QCoreApplication::processEvents();
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
