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
#include "viewers/grid.h"
#include "base/cycler.h"
#include "base/shortcuts.h"
#include "base/tiledpatternmaker.h"
#include "base/transparentwidget.h"
#include "base/utilities.h"
#include "designs/design_maker.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"
#include "panels/view_panel.h"
#include "style/style.h"
#include "tapp/design_element.h"
#include "tapp/figure.h"
#include "tapp/prototype.h"
#include "tile/feature.h"

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
    canPaint    = true;
    closed      = false;
    dragging    = false;
    mouseMode   = MOUSE_MODE_NONE;

    setMouseTracking(true);

    QSettings s;
    QPoint pt = s.value("viewPos").toPoint();
    move(pt);

    QGridLayout * grid = new QGridLayout();
    setLayout(grid);
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
    mapEditor   = MapEditor::getSharedInstance();
    tilingMaker = TilingMaker::getSharedInstance();
    designMaker = DesignMaker::getInstance();
    panel       = ControlPanel::getInstance();

    setKbdMode(KBD_MODE_XFORM_VIEW);

    ViewControl  * vcontrol = ViewControl::getInstance();
    connect(this,     &View::sig_refreshView,     vcontrol, &ViewControl::slot_refreshView);

    Cycler * cycler = Cycler::getInstance();
    connect(this,     &View::sig_cyclerQuit,   cycler,  &Cycler::slot_stopCycle);
    connect(this,     &View::sig_cyclerKey,    cycler,  &Cycler::slot_psuedoKey);

    resize(QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));

    show();
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

void  View::paintEnable(bool enable)
{
    setAttribute(Qt::WA_NoSystemBackground,!enable);
    setAutoFillBackground(enable);
    canPaint = enable;
}

void View::resize(QSize sz)
{
    QWidget::resize(sz);
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

void View::setMouseMode(eMouseMode newMode)
{
    qDebug() << "MouseMode:" << newMode;

    if (newMode == MOUSE_MODE_NONE)
    {
        config->showCenterMouse = false;
    }
    else
    {
        config->showCenterMouse = true;
    }

    ViewPanel * vp = panel->getViewPanel();

    if (mouseMode == MOUSE_MODE_CENTER && newMode == MOUSE_MODE_NONE)
    {
        switch (lastMouseMode)
        {
        case MOUSE_MODE_ROTATE:
            vp->btnRot->setChecked(true);
            vp->setRotateMode(true);
            return;

        case MOUSE_MODE_SCALE:
            vp->btnZoom->setChecked(true);
            panel->getViewPanel()->setScaleMode(true);
            return;

        case MOUSE_MODE_TRANSLATE:
            vp->btnPan->setChecked(true);
            panel->getViewPanel()->setTranslateMode(true);
            return;

        case MOUSE_MODE_CENTER:
            break;

        case MOUSE_MODE_NONE:
            break;
        }
    }

    lastMouseMode = mouseMode;
    mouseMode     = newMode;


    update();
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

    qDebug() << "Tilings:"  << Tiling::refs
             << "Layers:"   << Layer::refs
             << "Styles:"   << Style::refs
             << "Maps:"     << Map::refs
             << "Protos:"   << Prototype::refs
             << "DELs:"     << DesignElement::refs
             << "PDELs:"    << PlacedDesignElement::refs2
             << "Figures:"  << Figure::refs
             << "Features:" << Feature::refs
             << "Edges:"    << Edge::refs
             << "Vertices:" << Vertex::refs
             << "DCELs"     << DCEL::refs
             << "faces"     << Face::refs;

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
    case 'A':  setKbdMode(KBD_MODE_DES_ORIGIN); break;
    case 'B':  setKbdMode(KBD_MODE_DES_OFFSET); break;
    case 'D':  duplicateView(); break;
    case 'E':  emit sig_refreshView(); break;    // just for debug
    case 'F':  config->debugReplicate = !config->debugReplicate; emit sig_figure_changed(); break;
    case 'G':  config->showGrid = !config->showGrid; Grid::getSharedInstance()->create(); emit sig_refreshView(); break;
    case 'H':  config->hideCircles = !config->hideCircles; config->showCenterDebug = !config->showCenterDebug; update(); break;
    case 'I':  designMaker->designLayerShow(); break;  // I=in
    case 'K':  config->debugMapEnable = !config->debugMapEnable; emit sig_figure_changed(); break;
    case 'L':  setKbdMode(KBD_MODE_DES_LAYER_SELECT); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  theApp->slot_bringToPrimaryScreen(); break;
    case 'O':  designMaker->designLayerHide(); break; // o=out
    case 'P':  emit sig_saveImage(); break;
    case 'Q':  if (Cycler::getInstance()->getMode() != CYCLE_NONE) { emit sig_cyclerQuit(); }
        else { QApplication::quit(); }
        break;
    case 'R':  break;
    case 'S':  setKbdMode(KBD_MODE_DES_SEPARATION); break;
    case 'T':  setKbdMode(KBD_MODE_XFORM_TILING); break;
    case 'U':  setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  setKbdMode(KBD_MODE_XFORM_UNIQUE_FEATURE);break;
    case 'X':  config->circleX = !config->circleX; emit sig_refreshView(); update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  setKbdMode(KBD_MODE_DES_ZLEVEL); break;

    case Qt::Key_Return: if (config->kbdMode == KBD_MODE_DES_STEP) designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Shortcuts");
        if (config->getViewerType() == VIEW_DESIGN)
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
        if (config->kbdMode == KBD_MODE_DES_LAYER_SELECT)
        {
            designMaker->designLayerSelect(key-'0');
        }
        else if (config->kbdMode == KBD_MODE_DES_STEP)
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



//////////////////////////////////////////////////
///
/// Events
///
//////////////////////////////////////////////////

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


void View::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!canPaint)
    {
        qDebug() << "View::paintEvent() - discarded";
        //QWidget::paintEvent(event);
        return;
    }

    //qDebug() << "++++START VIEW PAINT - Scene: items=" << layers.size() << "viewRect" << rect();
    //qDebug() << "View::paintEvent()";

    //QWidget::paintEvent(event);

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

    if (loadUnit.loadTimer.isValid())
    {
        qint64 delta = loadUnit.loadTimer.elapsed();
        double qdelta = delta /1000.0;
        QString str = QString("%1").arg(qdelta, 8, 'f', 3, QChar(' '));

        qInfo().noquote() << "Load operation for" << loadUnit.name << "took" << str << "seconds";
        loadUnit.loadTimer.invalidate();
        loadUnit.name.clear();
    }

    //qDebug() << "++++END PAINT";
}

#if 0
// every resize event has a move event afterwards
// but every move event does not necessarily have a resize eventy
void View::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
}
#endif

void View::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QSize oldSize   = frameSettings.getCropSize(config->getViewerType());
    QSize newSize   = size();
    QSize deltaSize = newSize - oldSize;

    qDebug() << "View::resizeEvent: old" << oldSize << "new" << newSize << "delta-size" << deltaSize;

    if (oldSize == newSize)
    {
        return;
    }

    switch(config->getViewerType())
    {
    case VIEW_MOTIF_MAKER:
    case VIEW_DESIGN:
        frameSettings.setDeltaSize(config->getViewerType(),deltaSize);
        break;

    case VIEW_TILING_MAKER:
    case VIEW_TILING:
    case VIEW_MOSAIC:
    case VIEW_PROTOTYPE:
    case VIEW_MAP_EDITOR:
        frameSettings.setCommonDeltaSizes(deltaSize);
        break;

    default:
        break;
    }

    emit sig_viewSizeChanged(newSize);      // for outer border (if any)
    Grid::getSharedInstance()->create();    // re-create
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

void View::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "View::mousePressEvent";
    if (event->button() == Qt::MiddleButton)
    {
        return; // discards middle button on wheel
    }

    dragging = true;

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    switch (mouseMode)
    {
    case MOUSE_MODE_CENTER:
        emit sig_mousePressed(event->position(),event->button());
        panel->getViewPanel()->btnCenter->setChecked(false);
        setMouseMode(MOUSE_MODE_NONE);
        break;

    case MOUSE_MODE_TRANSLATE:
        sLast = event->globalPosition();
        break;

    case MOUSE_MODE_ROTATE:
    case MOUSE_MODE_SCALE:
    case MOUSE_MODE_NONE:
        emit sig_mousePressed(event->position(),event->button());
        break;
    }
#else
    switch (mouseMode)
    {
    case MOUSE_MODE_CENTER:
        emit sig_mousePressed(event->localPos(),event->button());
        panel->getViewPanel()->btnCenter->setChecked(false);
        setMouseMode(MOUSE_MODE_NONE);
        break;

    case MOUSE_MODE_TRANSLATE:
        sLast = event->globalPos();
        break;

    case MOUSE_MODE_SCALE:
    case MOUSE_MODE_ROTATE:
    case MOUSE_MODE_NONE:
        emit sig_mousePressed(event->localPos(),event->button());
        break;
    }
#endif
    update();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "View::mouseMoveEvent";

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    if (dragging)
    {
        emit sig_mouseDragged(event->position());

        if (mouseMode ==  MOUSE_MODE_TRANSLATE)
        {
            QPointF spt  = event->globalPosition();
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(translate);
        }
    }
    else
    {
        emit sig_mouseMoved(event->position());
    }
#else
    if (dragging)
    {
        emit sig_mouseDragged(event->pos());

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
        emit sig_mouseMoved(event->pos());
    }

#endif
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "View::mouseReleaseEvent";
    dragging = false;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    emit sig_mouseReleased(event->position());
#else
    emit sig_mouseReleased(event->localPos());
#endif
}

void View::mouseDoubleClickEvent(QMouseEvent * event)
{
    dragging = false;

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    emit sig_mouseDoublePressed(event->position(),event->button());
#else
    emit sig_mouseDoublePressed(event->localPos(),event->button());
#endif
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
