#include <QSize>
#include <QApplication>
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/image_view.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "legacy/designs.h"
#include "model/settings/canvas_settings.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/sys/load_unit.h"

#ifdef __linux__
#define ALT_MODIFIER Qt::MetaModifier
#else
#define ALT_MODIFIER Qt::AltModifier
#endif

using std::make_shared;

DesignMaker::DesignMaker()
{
    selectedLayer   = 0;
    stepsTaken      = 0;

    loadUnit        = new LoadUnit(LT_LEGACY);
}

void DesignMaker::init()
{
    viewControl = Sys::viewController;
    config      = Sys::config;

    availableDesigns.insert(DESIGN_5,make_shared<Design5>(DESIGN_5,"Pattern 1 (created)"));
    availableDesigns.insert(DESIGN_6,make_shared<Design6>(DESIGN_6,"Pattern 2 (re-ceated)"));
    availableDesigns.insert(DESIGN_7,make_shared<Design7>(DESIGN_7,"Pattern 3 (re-ceated)"));
    availableDesigns.insert(DESIGN_8,make_shared<Design8>(DESIGN_8,"The Hu Symbol"));
    availableDesigns.insert(DESIGN_9,make_shared<Design9>(DESIGN_9,"A packing of Hu symbols"));
    availableDesigns.insert(DESIGN_HU_INSERT,make_shared<DesignHuInsert>(DESIGN_HU_INSERT,"Hu Insert"));
    availableDesigns.insert(DESIGN_10,make_shared<DesignHuPacked>(DESIGN_10,"Fully packed Hu symbols"));
    availableDesigns.insert(DESIGN_11,make_shared<Design11>(DESIGN_11,"Woven Hu (unsuccessful)"));
    availableDesigns.insert(DESIGN_12,make_shared<Design12>(DESIGN_12,"Enneagram"));
    availableDesigns.insert(DESIGN_13,make_shared<Design13>(DESIGN_13,"Alhambra 1"));
    availableDesigns.insert(DESIGN_14,make_shared<Design14>(DESIGN_14,"Alhambra 2"));
    availableDesigns.insert(DESIGN_16,make_shared<Design16>(DESIGN_16,"Broug: Capella Palatina Palermo"));
    availableDesigns.insert(DESIGN_17,make_shared<Design17>(DESIGN_17,"Broug: Koran of Rashid al-Din"));
    availableDesigns.insert(DESIGN_18,make_shared<Design18>(DESIGN_18,"Broug: Mustansiriya Madrasa"));
    availableDesigns.insert(DESIGN_19,make_shared<Design19>(DESIGN_19,"NOT Broug: Esrefogulu Mosque"));
    availableDesigns.insert(DESIGN_KUMIKO1,make_shared<DesignKumiko1>(DESIGN_KUMIKO1,"Kumiko 1"));
    availableDesigns.insert(DESIGN_KUMIKO2,make_shared<DesignKumiko2>(DESIGN_KUMIKO2,"Kumiko 2"));

    connect(Sys::sysview, &SystemView::sig_deltaScale,    this, &DesignMaker::designScale);
    connect(Sys::sysview, &SystemView::sig_deltaRotate,   this, &DesignMaker::designRotate);
    connect(Sys::sysview, &SystemView::sig_deltaMoveY,    this, &DesignMaker::designMoveY);
    connect(Sys::sysview, &SystemView::sig_deltaMoveX,    this, &DesignMaker::designMoveX);
    connect(this,         &DesignMaker::sig_updateView,      viewControl, &SystemViewController::slot_updateView);
    connect(this,         &DesignMaker::sig_reconstructView, viewControl, &SystemViewController::slot_reconstructView);
}

void DesignMaker::addDesign(DesignPtr d)
{
    // the View controller adds it to the view
    activeDesigns.push_back(d);
    designName = QString("Design: %1").arg(d->getTitle());
    qDebug() << designName << "addded";
}

void DesignMaker::unload()
{
    for (auto & design : std::as_const(activeDesigns))
    {
        design->destoryPatterns();
    }
    activeDesigns.clear();
}

void DesignMaker::slot_loadDesign(eDesign design)
{
    DesignPtr d = availableDesigns.value(design);
    if (!d)
    {
        return;
    }

    slot_buildDesign(design);

    if (!config->lockView)
    {
        Sys::controlPanel->delegateView(VIEW_LEGACY,true);
    }
    
    emit sig_reconstructView();
}

void DesignMaker::slot_buildDesign(eDesign design)
{
    QString desName = Design::getDesignName(design);
    qInfo().noquote() << "TiledPatternMaker::slot_buildDesign" << desName;

    VersionedName vn(desName);
    VersionedFile vf;
    vf.updateFromVersionedName(vn);
    loadUnit->start(vf);

    DesignMaker * designMaker = Sys::designMaker;

    Sys::dumpRefs();
    designMaker->unload();
    Sys::dumpRefs();

    DesignPtr d = availableDesigns.value(design);
    designMaker->addDesign(d);

    d->build();
    d->repeat();

    // size view to design
    QSize size = d->getDesignInfo().getViewSize();
    auto & canvas = viewControl->getCanvas();
    canvas.setCanvasSize(size);

    viewControl->setSize(size);

    Sys::imageViewer->unloadLayerContent();

    loadUnit->end(LS_LOADED);

    emit sig_reconstructView();

    emit sig_loadedDesign(design);
}

void DesignMaker::reload()
{
    VersionedFile vf = loadUnit->getLoadFile();
    if (!vf.isEmpty())
    {
        eDesign design = (eDesign)designs.key(vf.getVersionedName().get());
        Sys::designMaker->slot_loadDesign(design);
    }
}

void DesignMaker::designLayerSelect(int layer)
{
    selectedLayer = layer;
}

void DesignMaker::designLayerZPlus()
{
    for (auto & design : std::as_const(activeDesigns))
    {
        if (design->isVisible())
        {
            design->zPlus(selectedLayer);
        }
    }
}

void DesignMaker::designLayerZMinus()
{
    for (auto & design : std::as_const(activeDesigns))
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
    for (auto & design : std::as_const(activeDesigns))
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
    for (auto & design : std::as_const(activeDesigns))
    {
        if (design->isVisible())
        {
            design->hideLayer(selectedLayer);
        }
    }
}

void DesignMaker::designReposition(qreal x, qreal y)
{
    for (auto & design : std::as_const(activeDesigns))
    {
        if (design->isVisible())
        {
            design->setXseparation(design->getXseparation() + x);
            design->setYseparation(design->getYseparation() + y);

            qDebug() << "origin=" << design->getStartTile() << "xSep=" << design->getXseparation() << "ySep=" << design->getYseparation();

            design->repeat();
        }
    }

    emit sig_updateView();
}

void DesignMaker::designOffset(qreal x, qreal y)
{
    for (auto & design : std::as_const(activeDesigns))
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
        emit sig_updateView();
    }
}

void DesignMaker::designOrigin(int x, int y)
{
    for (auto & design : std::as_const(activeDesigns))
    {
        if (design->isVisible())
        {
            QPointF pt = design->getStartTile();
            pt.setX(pt.x() + x);
            pt.setY(pt.y()+ y);
            design->setStartTile(pt);
        }
    }
    designReposition(0,0);
}

bool DesignMaker::step(int delta)
{
    qDebug() << "step delta" << delta << "step" << activeDesigns[0]->getStep();

    bool rv = true;
    for (auto &  design : std::as_const(activeDesigns))
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

    for (auto &  design : std::as_const(activeDesigns))
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
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ZLEVEL))
        designLayerZPlus();
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_STEP))
        step(1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_MODE_DES_SEPARATION))
        designReposition(0,-1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_OFFSET))
        designOffset(0,-1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ORIGIN))
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(0,-100);
        else
            designOrigin(0,-1);
    }
}

void DesignMaker::ProcKeyDown()
{
    // down arrrow
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ZLEVEL))
        designLayerZMinus();
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_STEP))
        step(-1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_MODE_DES_SEPARATION))
        designReposition(0,1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_OFFSET))
        designOffset(0,1);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ORIGIN))
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(0,100);
        else
            designOrigin(0,1);
    }
}

void DesignMaker::ProcKeyLeft()
{
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_MODE_DES_SEPARATION))
        designReposition(-1,0);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_OFFSET))
        designOffset(-1,0);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ORIGIN))
    {   Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(-100,0);
        else
            designOrigin(-1,0);
    }
}

void DesignMaker::ProcKeyRight()
{
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_MODE_DES_SEPARATION))
        designReposition(1,0);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_OFFSET))
        designOffset(1,0);
    else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_ORIGIN))
    {
        Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if ((mod & ALT_MODIFIER) == ALT_MODIFIER)
            designOrigin(100,0);
        else
            designOrigin(1,0);
    }
}

void DesignMaker::designScale(int delta)
{
    for (auto & design : std::as_const(activeDesigns))
    {
        CanvasSettings info = design->getDesignInfo();
        QSize sz =  info.getViewSize();
        qDebug() << "design: size=" << sz;
        if (delta > 0)
            sz +=  QSize(delta,delta);
        else
            sz -=  QSize(delta,delta);
        info.setViewSize(sz);
        design->setDesignInfo(info);
    }
    emit sig_updateView();
}

void DesignMaker::designRotate(int delta)
{
    Q_UNUSED(delta);
    //qWarning("DesignMaker::designRotate - not implemented");
}

void DesignMaker::designMoveY(int delta)
{
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_POS) || Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_LAYER_SELECT))
    {
        for (auto & design : std::as_const(activeDesigns))
        {
            qreal top = design->getYoffset2();
            top -= delta;
            design->setYoffset2(top);
        }
        emit sig_updateView();
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
    if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_POS) || Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_LAYER_SELECT))
    {
        for (auto & design : std::as_const(activeDesigns))
        {
            qreal left = design->getXoffset2();
            left -= delta;
            design->setXoffset2(left);
        }
        emit sig_updateView();
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
