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

#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "base/shortcuts.h"
#include "base/tiledpatternmaker.h"
#include "base/transparentwidget.h"
#include "base/utilities.h"
#include "panels/panel.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "style/style.h"
#include "designs/design_maker.h"

View * View::mpThis = nullptr;

View * View::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new View();
    }
    return mpThis;
}

void View::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}


View::View()
{
    closed      = false;
    dragging    = false;
    mouseMode   = MOUSE_MODE_NONE;

    setMouseTracking(true);

    QSettings s;
    QPoint pt = s.value("viewPos").toPoint();
    move(pt);

    QGridLayout * grid = new QGridLayout();
    setLayout(grid);

    addFrameSetting(VIEW_DESIGN,            Bounds(-10.0,10.0,20.0), QSize(1500,1100));
    addFrameSetting(VIEW_MOSAIC,            Bounds(-10.0,10.0,20.0), QSize(1500,1100));
    addFrameSetting(VIEW_PROTOTYPE,         Bounds(-10.0,10.0,20.0), QSize(1500,1100));
    addFrameSetting(VIEW_DESIGN_ELEMENT,    Bounds(-10.0,10.0,20.0), QSize(1500,1100));
    addFrameSetting(VIEW_MOTIF_MAKER,       Bounds(-10.0,10.0,20.0), QSize( 900, 900));
    addFrameSetting(VIEW_TILING,            Bounds(-10.0,10.0,20.0), QSize(1500,1100));
    addFrameSetting(VIEW_TILING_MAKER,      Bounds(-10.0,10.0,20.0), QSize(1000,1000));
    addFrameSetting(VIEW_MAP_EDITOR,        Bounds(-10.0,10.0,20.0), QSize( 900, 900));
    addFrameSetting(VIEW_FACE_SET,          Bounds(-10.0,10.0,20.0), QSize(1500,1100));
}

View::~View()
{
    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue("viewPos", pos());
    }
}

void View::init()
{
    config      = Configuration::getInstance();
    mapEditor   = MapEditor::getInstance();
    tilingMaker = TilingMaker::getInstance();
    designMaker = DesignMaker::getInstance();

    setKbdMode(KBD_MODE_UNDEFINED);

    ViewControl  * vcontrol = ViewControl::getInstance();
    connect(this,     &View::sig_refreshView,     vcontrol, &ViewControl::slot_refreshView);

    Cycler * cycler = Cycler::getInstance();
    connect(this,     &View::sig_cyclerQuit,   cycler,  &Cycler::slot_stopCycle);
    connect(this,     &View::sig_cyclerKey,    cycler,  &Cycler::slot_psuedoKey);

    resize(QSize(1500,1000));

    show();
}

void View::closeEvent(QCloseEvent *event)
{
    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue("viewPos", pos());
        qDebug() <<  "closeEvent pos" << pos();
        closed = true;
    }

    QWidget::closeEvent(event);

    qApp->quit();
}

void View::addLayer(LayerPtr layer)
{
    layers.push_back(layer);
}

void View::addTopLayer(LayerPtr layer)
{
    layers.push_front(layer);
}

void View::clearView()
{
    clearLayers();
    clearLayout();
    //reInitFrameSettings();
    //update();
}

QVector<LayerPtr> View::getActiveLayers()
{
    return layers;
}

void  View::reInitFrameSettings()
{
    reInitFrameSetting(VIEW_DESIGN);
    reInitFrameSetting(VIEW_MOSAIC);
    reInitFrameSetting(VIEW_PROTOTYPE);
    reInitFrameSetting(VIEW_DESIGN_ELEMENT);
    reInitFrameSetting(VIEW_MOTIF_MAKER);
    reInitFrameSetting(VIEW_TILING);
    reInitFrameSetting(VIEW_TILING_MAKER);
    reInitFrameSetting(VIEW_MAP_EDITOR);
    reInitFrameSetting(VIEW_FACE_SET);
}

void  View::addFrameSetting(eViewType evt, Bounds bounds, QSize size)
{
    frameSettings.insert(evt, FrameSettings(bounds,size));
}

void  View::reInitFrameSetting(eViewType evt)
{
    frameSettings[evt].reInit();
}

QTransform View::getDefinedFrameTransform(eViewType e)
{
    QTransform t = frameSettings[e].getDefinedFrameTransform();
    //qDebug().noquote() << "View::getDefinedFrameTransform" << sViewerType[e] << Transform::toInfoString(t);
    return t;
}

QTransform View::getActiveFrameTransform(eViewType e)
{
    QTransform t = frameSettings[e].getActiveFrameTransform();
    //qDebug().noquote() << "View::getActiveFrameTransform" << sViewerType[e] << Transform::toInfoString(t);
    return t;
}


QSize View::getDefinedFrameSize(eViewType e)
{
    QSize sz =  frameSettings[e].getDefinedFrameSize();
    //qDebug().noquote() << "View::getDefinedFrameSize()" << sViewerType[e] << sz;
    return sz;
}

void View::setDefinedFrameSize(eViewType e, QSize sz)
{
    qDebug().noquote() << "View::setDefinedFrameSize()" << sViewerType[e] << sz;
    frameSettings[e].setDefinedFrameSize(sz);
}

QSize View::getActiveFrameSize(eViewType e)
{
    QSize sz =  frameSettings[e].getActiveFrameSize();
    //qDebug().noquote() << "View::getActiveFrameSize()" << sViewerType[e] << sz;
    return sz;
}

void View::setActiveFrameSize(eViewType e, QSize sz)
{
    qDebug().noquote() << "View::setActiveFrameSize()" << sViewerType[e] << sz;
    frameSettings[e].setActiveFrameSize(sz);
}


void View::paintEvent(QPaintEvent *event)
{
    //qDebug() << "++++START VIEW PAINT - Scene: items=" << layers.size() << "viewRect" << rect();

    QWidget::paintEvent(event);

    QPainter painter(this);

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevel);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (auto layer : qAsConst(layers))
    {
        if (layer->isVisible())
        {
            layer->forceLayerRecalc(false);
            layer->paint(&painter);
        }
    }

    //qDebug() << "++++END PAINT";
}

void View::resize(QSize sz)
{
    QWidget::resize(sz);
}

void View::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    QSize oldSize  = getActiveFrameSize(config->viewerType);
    QSize newSize  = size();
    qDebug() << "View::resizeEvent: old" << oldSize << "new" << newSize;

    if (oldSize == newSize)
    {
        return;
    }

    frameSettings[config->viewerType].setActiveFrameSize(newSize);
    if (config->scaleToView)
    {
        frameSettings[config->viewerType].setDefinedFrameSize(newSize);
    }

    emit sig_viewSizeChanged(newSize);
}

void  View::setAllMosaicDefinedSizes(QSize sz)
{
    // this overwrites some set by tiling
    frameSettings[VIEW_DESIGN].setDefinedFrameSize(sz);
    frameSettings[VIEW_MOSAIC].setDefinedFrameSize(sz);
    frameSettings[VIEW_PROTOTYPE].setDefinedFrameSize(sz);
    frameSettings[VIEW_DESIGN_ELEMENT].setDefinedFrameSize(sz);
    frameSettings[VIEW_TILING].setDefinedFrameSize(sz);
}

void View::setAllTilingDefinedSizes(QSize sz)
{
    // some of these are overwritten when mosaic is loaded/changed
    frameSettings[VIEW_TILING].setDefinedFrameSize(sz);
    frameSettings[VIEW_TILING_MAKER].setDefinedFrameSize(sz);
    frameSettings[VIEW_DESIGN_ELEMENT].setDefinedFrameSize(sz);
    frameSettings[VIEW_PROTOTYPE].setDefinedFrameSize(sz);
}

void  View::setAllMosaicActiveSizes(QSize sz)
{
    // this overwrites some set by tiling
    frameSettings[VIEW_DESIGN].setActiveFrameSize(sz);
    frameSettings[VIEW_MOSAIC].setActiveFrameSize(sz);
    frameSettings[VIEW_PROTOTYPE].setActiveFrameSize(sz);
    frameSettings[VIEW_DESIGN_ELEMENT].setActiveFrameSize(sz);
    frameSettings[VIEW_TILING].setActiveFrameSize(sz);
}

void View::setAllTilingActiveSizes(QSize sz)
{
    // some of these are overwritten when mosaic is loaded/changed
    frameSettings[VIEW_TILING].setActiveFrameSize(sz);
    frameSettings[VIEW_TILING_MAKER].setActiveFrameSize(sz);
    frameSettings[VIEW_DESIGN_ELEMENT].setActiveFrameSize(sz);
    frameSettings[VIEW_PROTOTYPE].setActiveFrameSize(sz);
}

void View::keyPressEvent( QKeyEvent *k )
{
    if (tilingMaker->procKeyEvent(k))        // tiling maker
    {
        return;
    }
    else if (mapEditor->procKeyEvent(k))    // map editor
    {
        return;
    }
    else
    {
        procKeyEvent(k);
    }
}

void View::duplicateView()
{
    QPixmap pixmap(size());
    pixmap.fill((Qt::transparent));

    QPainter painter(&pixmap);
    render(&painter);

    TransparentWidget * tw = new TransparentWidget();
    tw->resize(size());
    tw->setPixmap(pixmap);
    tw->show();
}

void View::setMouseMode(eMouseMode mode)
{
    qDebug() << "MouseMode:" << mode;

    mouseMode = mode;

    if (mouseMode == MOUSE_MODE_NONE)
    {
        config->showCenterMouse = false;
    }
    else
    {
        config->showCenterMouse = true;
    }
    update();
}


void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        return; // discards middle button on wheel
    }

    dragging = true;

    emit sig_mousePressed(event->localPos(),event->button());

    switch (mouseMode)
    {
    case MOUSE_MODE_TRANSLATE:
        sLast = event->globalPos();
        break;

    case MOUSE_MODE_SCALE:
    case MOUSE_MODE_ROTATE:
        emit sig_setCenter(event->localPos());
        break;

    case MOUSE_MODE_NONE:
        break;
    }
    update();
}

void View::mouseDoubleClickEvent(QMouseEvent * event)
{
    dragging = false;

    emit sig_mouseDoublePressed(event->localPos(),event->button());
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging)
    {
        emit sig_mouseDragged(event->localPos());
        if (mouseMode ==  MOUSE_MODE_TRANSLATE)
        {
            QPointF spt  = event->globalPos();
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(translate);
        }
    }
    else
    {
        emit sig_mouseMoved(event->localPos());
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;

    emit sig_mouseReleased(event->localPos());
}

void View::wheelEvent(QWheelEvent *event)
{

    if (event->buttons() != 0)
    {
        return;  // discards middle button on wheel
    }

    Qt::KeyboardModifiers kms = event->modifiers();
    bool shift = (kms & Qt::SHIFT);


    if (mouseMode == MOUSE_MODE_SCALE)
    {
        qreal delta = 0.01;
        if (shift)
        {
            delta = 0.1;
        }
        if (event->angleDelta().y() >=  0)
            emit sig_wheel_scale(delta);
        else
            emit sig_wheel_scale(-delta);
    }
    else if (mouseMode == MOUSE_MODE_ROTATE)
    {
        qreal delta = 0.5;
        if (shift)
        {
            delta = 5.0;
        }
        if (event->angleDelta().y() >=  0)
            emit sig_wheel_rotate(delta); // degrees
        else
            emit sig_wheel_rotate(-delta); // degrees
    }
}

void View::setBackgroundColor(QColor color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    //setAutoFillBackground(true);
    setPalette(pal);
}

QColor View::getBackgroundColor()
{
    QPalette pal = palette();
    QColor c = pal.color(QPalette::Window);
    return c;
}

void View::clearLayout()
{
    clearLayout(layout(),true);
}

void View::clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void View::dump(bool summary)
{
    qDebug() << "View: layers =" << numLayers();

    if (!summary)
    {
        for (auto layer : qAsConst(layers))
        {
            qDebug() << "Layer:" << layer->getName() << layer.get();
        }
    }

    qDebug() << "Tilings:" << Tiling::refs << "Layers:" << Layer::refs  << "Styles:" << Style::refs << "Maps:" << Map::refs << "Protos:" << Prototype::refs << "DELs:" << DesignElement::refs  << "PDELs:" << PlacedDesignElement::refs2 << "Figures:" << Figure::refs << "Features:" << Feature::refs << "Edges:" << Edge::refs << "Vertices:"  << Vertex::refs;
}

void View::setKbdMode(eKbdMode mode)
{
    eKbdMode newMode = ControlPanel::getValidKbdMode(mode);
    if (newMode != config->kbdMode)
    {
        config->kbdMode = newMode;
        qDebug().noquote() << "Keyboard Mode is:" << getKbdModeStr();
        emit sig_kbdMode(config->kbdMode);
    }
}

QString View::getKbdModeStr()
{
    return sKbdMode[config->kbdMode];
}

bool View::procKeyEvent(QKeyEvent *k)
{
    int  delta = 1;
    if ((k->modifiers() & (Qt::SHIFT | Qt::CTRL)) == (Qt::SHIFT | Qt::CTRL))
        delta = 50;
    else if ((k->modifiers() & Qt::SHIFT) == Qt::SHIFT)
        delta = 10;

    bool rv = ProcNavKey(k->key(),delta);
    if (rv)
    {
        return true;
    }

    rv =  ProcKey(k);
    return rv;
}

bool View::ProcNavKey(int key, int delta)
{
    switch (key)
    {
    case Qt::Key_Up:    ProcKeyUp(delta);     return true;
    case Qt::Key_Down:  ProcKeyDown(delta);   return true;
    case Qt::Key_Left:  ProcKeyLeft(delta);  return true;
    case Qt::Key_Right: ProcKeyRight(delta); return true;

    case '.':           emit sig_deltaScale(delta);  return true;  // scale down
    case '>':           emit sig_deltaScale(delta);  return true;  // scale down
    case ',':           emit sig_deltaScale(-delta); return true;  // scale up
    case '<':           emit sig_deltaScale(-delta); return true;  // scale up

    case '-':           emit sig_deltaRotate(-delta); return true; // rotate left
    case '_':           emit sig_deltaRotate(-delta); return true; // rotate left
    case '=':           emit sig_deltaRotate( delta); return true; // rotate right
    case '+':           emit sig_deltaRotate( delta); return true; // rotate right

    default: return false;
    }
}

bool View::ProcKey(QKeyEvent *k)
{
    static int val = 0;

    int key = k->key();
    switch (key)
    {
    case 'A':  setKbdMode(KBD_MODE_ORIGIN); break;
    case 'B':  setKbdMode(KBD_MODE_OFFSET); break;
    case 'D':  duplicateView(); break;
    case 'E':  emit sig_refreshView(); break;    // just for debug
    case 'F':  config->debugReplicate = !config->debugReplicate; emit sig_figure_changed(); break;
    case 'G':  config->showGrid = !config->showGrid; emit sig_refreshView(); break;
    case 'H':  config->hideCircles = !config->hideCircles; config->showCenterDebug = !config->showCenterDebug; update(); break;
    case 'I':  designMaker->designLayerShow(); break;  // I=in
    case 'K':  config->debugMapEnable = !config->debugMapEnable; emit sig_figure_changed(); break;
    case 'L':  setKbdMode(KBD_MODE_LAYER); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  theApp->slot_bringToPrimaryScreen(); break;
    case 'O':  designMaker->designLayerHide(); break; // o=out
    case 'P':  emit sig_saveImage(); break;
    case 'Q':  if (Cycler::getInstance()->getMode() != CYCLE_NONE) { emit sig_cyclerQuit(); }
               else { QApplication::quit(); }
               break;
    case 'R':  break;
    case 'S':  setKbdMode(KBD_MODE_SEPARATION); break;
    case 'T':  setKbdMode(KBD_MODE_XFORM_TILING); break;
    case 'U':  setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  setKbdMode(KBD_MODE_XFORM_UNIQUE_FEATURE);break;
    case 'X':  config->circleX = !config->circleX; emit sig_refreshView(); update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  setKbdMode(KBD_MODE_ZLEVEL); break;

    case Qt::Key_Return: if (config->kbdMode == KBD_MODE_STEP) designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: setKbdMode(KBD_MODE_UNDEFINED); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Shortcuts");
        if (config->viewerType == VIEW_DESIGN)
        {
            box->setText(Shortcuts::getDesignShortcuts());
        }
        else
        {
            box->setText(Shortcuts::getMosaicShortcuts());
        }
        box->setModal(false);
        box->show();
    }
        break;
    case Qt::Key_F2: setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F3: break;
    case Qt::Key_F4: dump(true); break;
    case Qt::Key_F5: break;
    case Qt::Key_Space: if (Cycler::getInstance()->getMode() != CYCLE_NONE) emit sig_cyclerKey(key); break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        // keys 0-9
        if (config->kbdMode == KBD_MODE_LAYER)
        {
           designMaker->designLayerSelect(key-'0');
        }
        else if (config->kbdMode == KBD_MODE_STEP)
        {
            val *= 10;
            val += (key - '0');
        }
        else
        {
            designMaker->designToggleVisibility(key-'0');
        }
        break;
    default:
        return false;
    }
    return true;
}

void View::ProcKeyUp(int delta)
{
    emit sig_deltaMoveY(-delta);
}

void View::ProcKeyDown(int delta)
{
    emit sig_deltaMoveY(delta);
}

void View::ProcKeyLeft(int delta)
{
    emit sig_deltaMoveX(-delta);
}

void View::ProcKeyRight(int delta)
{
    emit sig_deltaMoveX( delta);
}

