#include "design_control.h"
#include "base/shared.h"
#include "base/workspace.h"
#include "base/configuration.h"
#include "viewers/workspace_viewer.h"

DesignControl * DesignControl::mpThis = nullptr;

DesignControl * DesignControl::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new DesignControl();
    }
    return mpThis;
}

void DesignControl::releaseInstance()
{
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

DesignControl::DesignControl()
{
    selectedLayer   = 0;
    maxStep         = 0;
    stepsTaken      = 0;

    workspace = Workspace::getInstance();
    config    = Configuration::getInstance();

    timer     = new QTimer();
    connect(timer, &QTimer::timeout, this, &DesignControl::slot_nextStep);

}

void DesignControl::designLayerSelect(int layer)
{
    selectedLayer = layer;
}

void DesignControl::designLayerZPlus()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->zPlus(selectedLayer);
        }
    }
}

void DesignControl::designLayerZMinus()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->zMinus(selectedLayer);
        }
    }
}

void DesignControl::designLayerShow()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    qDebug() << "slot_showLayer() designs=" << designs.count();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->showLayer(selectedLayer);
        }
    }
}

void DesignControl::designLayerHide()
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    qDebug() << "slot_hideLayer() designs=" << designs.count();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->hideLayer(selectedLayer);
        }
    }
}

void DesignControl::designReposition(qreal x, qreal y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->setXseparation(d->getXseparation() + x);
            d->setYseparation(d->getYseparation() + y);

            qDebug() << "origin=" << d->getDesignInfo().getStartTile() << "xSep=" << d->getXseparation() << "ySep=" << d->getYseparation();

            d->repeat();
        }
    }
    workspace->update();
}

void DesignControl::designOffset(qreal x, qreal y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->setXoffset2(d->getXoffset2() + x);
            d->setYoffset2(d->getYoffset2() + y);
        }
    }
    designReposition(0,0);
}

void DesignControl::designToggleVisibility(int design)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (design < designs.count())
    {
        DesignPtr d = designs[design];
        d->setVisible(!d->isVisible());
        workspace->update();
    }
}

void DesignControl::designOrigin(int x, int y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            QPointF pt = d->getDesignInfo().getStartTile();
            pt.setX(pt.x() + x);
            pt.setY(pt.y()+ y);
            d->getDesignInfo().setStartTile(pt);
        }
    }
    designReposition(0,0);
}

void DesignControl::stopTimer()
{
    timer->stop();
}

void DesignControl::startTimer()
{
    stepsTaken = 0;
    timer->start(1000/FPS);
}

void DesignControl::setMaxStep(int max)
{
    maxStep = SECONDS(max);
}

void DesignControl::slot_nextStep()
{
    if (stepsTaken++ <= maxStep)
    {
        step(1);
    }
    else
    {
        stopTimer();
    }
}

bool DesignControl::step(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (designs.count() == 0) return false;

    qDebug() << "step delta" << delta << "step" << designs[0]->getStep() << "out of" << maxStep;

    bool rv = true;
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            rv = d->doStep();
            d->deltaStep(delta);
            if (!rv)
            {
                stopTimer();
            }
        }
    }

    return rv;
}

void DesignControl::setStep(int astep)
{
    qDebug() << "set step=" << astep;

    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->setStep(astep);
        }
    }
    step(0);
}

void DesignControl::ProcKeyUp(int delta, bool isALT)
{
    // up arrow
    switch (config->kbdMode)
    {
    case KBD_MODE_ZLEVEL:
        designLayerZPlus();
        break;
    case KBD_MODE_STEP:
        step(1);
        break;
    case KBD_MODE_SEPARATION:
        designReposition(0,-1);
        break;
    case KBD_MODE_OFFSET:
        designOffset(0,-1);
        break;
    case KBD_MODE_ORIGIN:
        if (isALT)
            designOrigin(0,-100);
        else
            designOrigin(0,-1);
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        designMoveY(delta);           // applies deltas to designs
        break;
    default:
        break;
    }
}

void DesignControl::ProcKeyDown(int delta, bool isALT)
{
    // down arrrow
    switch (config->kbdMode)
    {
    case KBD_MODE_ZLEVEL:
        designLayerZMinus();
        break;
    case KBD_MODE_STEP:
        step(-1);
        break;
    case KBD_MODE_SEPARATION:
        designReposition(0,1);
        break;
    case KBD_MODE_OFFSET:
        designOffset(0,1);
        break;
    case KBD_MODE_ORIGIN:
        if (isALT)
            designOrigin(0,100);
        else
            designOrigin(0,1);
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        designMoveY(-delta);
        break;
    default:
        break;
    }
}

void DesignControl::ProcKeyLeft(int delta, bool isALT)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_SEPARATION:
        designReposition(-1,0);
        break;
    case KBD_MODE_OFFSET:
        designOffset(-1,0);
        break;
    case KBD_MODE_ORIGIN:
        if (isALT)
            designOrigin(-100,0);
        else
            designOrigin(-1,0);
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        designMoveX(-delta);
        break;
    case KBD_MODE_ZLEVEL:
    case KBD_MODE_STEP:
    default:
        break;
    }
}

void DesignControl::ProcKeyRight(int delta, bool isALT)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_SEPARATION:
        designReposition(1,0);
        break;
    case KBD_MODE_OFFSET:
        designOffset(1,0);
        break;
    case KBD_MODE_ORIGIN:
        if (isALT)
            designOrigin(100,0);
        else
            designOrigin(1,0);
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        designMoveX( delta);
        break;
    case KBD_MODE_ZLEVEL:
    case KBD_MODE_STEP:
    default:
        break;
    }
}

void DesignControl::designScale(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (designs.size())
    {
        for (int i=0; i < designs.count(); i++)
        {
            DesignPtr d = designs[i];

            QSize sz =  d->getDesignInfo().getSize();
            qDebug() << "design: size=" << sz;
            if (delta > 0)
                sz +=  QSize(delta,delta);
            else
                sz -=  QSize(delta,delta);
            d->getDesignInfo().setSize(sz);
        }
        workspace->update();
    }
}

void DesignControl::designRotate(int delta, bool cw)
{
    Q_UNUSED(delta)
    Q_UNUSED(cw)
#if 0
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];

    }
    theScene.invalidate();
#endif
}

void DesignControl::designMoveY(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (designs.size())
    {
        for (int i=0; i < designs.count(); i++)
        {
            DesignPtr d = designs[i];
            qreal top = d->getYoffset2();
            top -= delta;
            d->setYoffset2(top);
        }
        workspace->update();
    }
}

void DesignControl::designMoveX(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (designs.size())
    {
        for (int i=0; i < designs.count(); i++)
        {
            DesignPtr d = designs[i];
            qreal left = d->getXoffset2();
            left -= delta;
            d->setXoffset2(left);
        }
        workspace->update();
    }
}
