#include "design_maker.h"
#include "base/shared.h"
#include "viewers/view.h"
#include "designs/design.h"

#ifdef __linux__
#define ALT_MODIFIER Qt::MetaModifier
#else
#define ALT_MODIFIER Qt::AltModifier
#endif

DesignMaker * DesignMaker::mpThis = nullptr;

DesignMaker * DesignMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new DesignMaker();
    }
    return mpThis;
}

void DesignMaker::releaseInstance()
{
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

DesignMaker::DesignMaker()
{
    selectedLayer   = 0;
    maxStep         = 0;
    stepsTaken      = 0;

    view   = View::getInstance();
    config = Configuration::getInstance();

    connect(view, &View::sig_deltaScale,    this, &DesignMaker::designScale);
    connect(view, &View::sig_deltaRotate,   this, &DesignMaker::designRotate);
    connect(view, &View::sig_deltaMoveY,    this, &DesignMaker::designMoveY);
    connect(view, &View::sig_deltaMoveX,    this, &DesignMaker::designMoveX);

#if 0
    connect(view, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);
    connect(view, &View::sig_mouseTranslate,    this, &Layer::slot_mouseTranslate);
    connect(view, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(view, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rotate);
    connect(view, &View::sig_setCenter,         this, &Layer::slot_setCenterScreen);
#endif
}


void DesignMaker::addDesign(DesignPtr d)
{
    // the View controller adds it to the view
    activeDesigns.push_back(d);
    designName = QString("Design: %1").arg(d->getTitle());
    qDebug() << designName << "addded";
}

QVector<DesignPtr> & DesignMaker::getDesigns()
{
    return activeDesigns;
}

void DesignMaker::clearDesigns()
{
    for (auto design : qAsConst(activeDesigns))
    {
        design->destoryPatterns();
    }
    activeDesigns.clear();
}

void DesignMaker::designLayerSelect(int layer)
{
    selectedLayer = layer;
}

void DesignMaker::designLayerZPlus()
{
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->zPlus(selectedLayer);
        }
    }
}

void DesignMaker::designLayerZMinus()
{
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->zMinus(selectedLayer);
        }
    }
}

void DesignMaker::designLayerShow()
{
    qDebug() << "slot_showLayer() designs=" << activeDesigns.count();
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->showLayer(selectedLayer);
        }
    }
}

void DesignMaker::designLayerHide()
{
    qDebug() << "slot_hideLayer() designs=" << activeDesigns.count();
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->hideLayer(selectedLayer);
        }
    }
}

void DesignMaker::designReposition(qreal x, qreal y)
{
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->setXseparation(design->getXseparation() + x);
            design->setYseparation(design->getYseparation() + y);

            qDebug() << "origin=" << design->getDesignInfo()->getStartTile() << "xSep=" << design->getXseparation() << "ySep=" << design->getYseparation();

            design->repeat();
        }
    }
    view->update();
}

void DesignMaker::designOffset(qreal x, qreal y)
{
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->setXoffset2(design->getXoffset2() + x);
            design->setYoffset2(design->getYoffset2() + y);
        }
    }
    designReposition(0,0);
}

void DesignMaker::designToggleVisibility(int design)
{
    if (design < activeDesigns.count())
    {
        DesignPtr d = activeDesigns[design];
        d->setVisible(!d->isVisible());
        view->update();
    }
}

void DesignMaker::designOrigin(int x, int y)
{
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            QPointF pt = design->getDesignInfo()->getStartTile();
            pt.setX(pt.x() + x);
            pt.setY(pt.y()+ y);
            design->getDesignInfo()->setStartTile(pt);
        }
    }
    designReposition(0,0);
}

void DesignMaker::setMaxStep(int max)
{
    maxStep = SECONDS(max);
}

bool DesignMaker::step(int delta)
{
    qDebug() << "step delta" << delta << "step" << activeDesigns[0]->getStep() << "out of" << maxStep;

    bool rv = true;
    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            rv = design->doStep();
            design->deltaStep(delta);
        }
    }

    return rv;
}

void DesignMaker::setStep(int astep)
{
    qDebug() << "set step=" << astep;

    for (auto design : qAsConst(activeDesigns))
    {
        if (design->isVisible())
        {
            design->setStep(astep);
        }
    }
    step(0);
}

void DesignMaker::ProcKeyUp()
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
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(0,-100);
        else
            designOrigin(0,-1);
    }
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        qCritical("Should not happen");
        break;
    default:
        break;
    }
}

void DesignMaker::ProcKeyDown()
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
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(0,100);
        else
            designOrigin(0,1);
    }
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        qCritical("Should not happen");
        break;
    default:
        break;
    }
}

void DesignMaker::ProcKeyLeft()
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
    {   Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(-100,0);
        else
            designOrigin(-1,0);
    }
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        qCritical("Should not happen");
        break;
    case KBD_MODE_ZLEVEL:
    case KBD_MODE_STEP:
    default:
        break;
    }
}

void DesignMaker::ProcKeyRight()
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
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(100,0);
        else
            designOrigin(1,0);
    }
        break;
    case KBD_MODE_POS:
    case KBD_MODE_LAYER:
        qCritical("Should not happen");
        break;
    case KBD_MODE_ZLEVEL:
    case KBD_MODE_STEP:
    default:
        break;
    }
}

void DesignMaker::designScale(int delta)
{
    for (auto design : qAsConst(activeDesigns))
    {
        QSize sz =  design->getDesignInfo()->getSize();
        qDebug() << "design: size=" << sz;
        if (delta > 0)
            sz +=  QSize(delta,delta);
        else
            sz -=  QSize(delta,delta);
        design->getDesignInfo()->setSize(sz);
    }
    view->update();
}

void DesignMaker::designRotate(int delta)
{
    Q_UNUSED(delta);
    qWarning("DesignMaker::designRotate - not implemented");
}

void DesignMaker::designMoveY(int delta)
{
    if (config->kbdMode == KBD_MODE_POS || config->kbdMode == KBD_MODE_LAYER )
    {
        for (auto design : qAsConst(activeDesigns))
        {
            qreal top = design->getYoffset2();
            top -= delta;
            design->setYoffset2(top);
        }
        view->update();
    }
    else
    {
        if (delta > 0)
        {
            ProcKeyUp();
        }
        else
        {
            ProcKeyDown();
        }
    }
}

void DesignMaker::designMoveX(int delta)
{
    if (config->kbdMode == KBD_MODE_POS || config->kbdMode == KBD_MODE_LAYER )
    {
        for (auto design : qAsConst(activeDesigns))
        {
            qreal left = design->getXoffset2();
            left -= delta;
            design->setXoffset2(left);
        }
        view->update();
    }
    else
    {
        if (delta > 0)
        {
            ProcKeyRight();
        }
        else
        {
            ProcKeyLeft();
        }

    }
}
