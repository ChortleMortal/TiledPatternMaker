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

#include "base/cycler.h"
#include "base/view.h"
#include "base/shortcuts.h"
#include "base/tiledpatternmaker.h"
#include "base/transparentwidget.h"
#include "base/utilities.h"
#include "panels/panel.h"
#include "viewers/workspace_viewer.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "style/style.h"
#include "designs/design_control.h"

#ifdef __linux__
#define ALT_MODIFIER Qt::MetaModifier
#else
#define ALT_MODIFIER Qt::AltModifier
#endif

View::View()
{
    closed      = false;
    dragging    = false;
    mouseMode   = MOUSE_MODE_NONE;

    setMouseTracking(true);

    gridPen.setColor(QColor(Qt::red));

    QSettings s;
    QPoint pt = s.value("viewPos").toPoint();
    move(pt);

    QGridLayout * grid = new QGridLayout();
    setLayout(grid);

    eViewType evt = VIEW_MOSAIC;
    FrameSettings * vs = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_PROTOTYPE;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_DESIGN_ELEMENT;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_TILING;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_TILING_MAKER;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-5.0,5.0,10.0), QSize(1000,1000));

    evt = VIEW_FACE_SET;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize(1500,1100));

    evt = VIEW_FROTOTYPE_MAKER;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize( 900, 900));

    evt = VIEW_MAP_EDITOR;
    vs  = &frameSettings[evt];
    vs->init(evt, Bounds(-10.0,10.0,20.0), QSize( 900, 900));
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
    designCtrl  = DesignControl::getInstance();

    setKbdMode(KBD_MODE_UNDEFINED);

    Workspace  * workspace = Workspace::getInstance();
    connect(this,     &View::sig_viewWS,     workspace, &WorkspaceViewer::slot_viewWorkspace);

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
}

QVector<LayerPtr> View::getActiveLayers()
{
    return layers;
}

FrameSettings & View::getFrameSettings(eViewType e)
{
    return frameSettings[e];
}

QTransform View::getFrameTransform(eViewType e)
{
    QTransform t = frameSettings[e].getFrameTransform();
    //qDebug().noquote() << "WorkspaceViewer::getViewTransform" << sViewerType[e] << Transform::toInfoString(t);
    return t;
}

QSize View::getFrameSize(eViewType e)
{
    QSize sz =  frameSettings[e].getFrameSize();
    qDebug().noquote() << "WorkspaceViewer::getViewSize()" << sViewerType[e] << sz;
    return sz;
}

void View::setFrameSize(eViewType e, QSize sz)
{
    qDebug().noquote() << "WorkspaceViewer::setViewSize()" << sViewerType[e] << sz;
    frameSettings[e].setFrameSize(sz);
}

QSize View::getActiveSize(eViewType e)
{
    QSize sz =  frameSettings[e].getActiveSize();
    qDebug().noquote() << "WorkspaceViewer::getActiveSize()" << sViewerType[e] << sz;
    return sz;
}

void View::setActiveSize(eViewType e, QSize sz)
{
    qDebug().noquote() << "WorkspaceViewer::setActiveSize()" << sViewerType[e] << sz;
    frameSettings[e].setActiveSize(sz);
}


void View::paintEvent(QPaintEvent *event)
{
    //qDebug() << "++++START VIEW PAINT - Scene: items=" << layers.size() << "viewRect" << rect();

    QWidget::paintEvent(event);

    QPainter painter(this);

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevel);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (auto layer : layers)
    {
        if (layer->isVisible())
        {
            layer->forceLayerRecalc(false);
            layer->paint(&painter);
        }
    }

    QRectF r = rect();
    drawForeground(&painter,r);

    //qDebug() << "++++END PAINT";
}

void View::resize(QSize sz)
{
    QWidget::resize(sz);
}

void View::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    QSize oldSize  = getActiveSize(config->viewerType);
    QSize newSize  = size();
    qDebug() << "View::resizeEvent: old" << oldSize << "new" << newSize;

    if (oldSize == newSize)
    {
        return;
    }

    setActiveSize(config->viewerType, newSize);
    if (config->scaleToView)
    {
        setFrameSize(config->viewerType,newSize);
    }

    emit sig_reconstructBorder();
}

void  View::setAllMosaicFrameSizes(QSize sz)
{
    // this overwrites some set by tiling
    setFrameSize(VIEW_DESIGN,sz);
    setFrameSize(VIEW_MOSAIC,sz);
    setFrameSize(VIEW_PROTOTYPE,sz);
    setFrameSize(VIEW_DESIGN_ELEMENT,sz);
    setFrameSize(VIEW_TILING,sz);
}

void View::setAllTilingFrameSizes(QSize sz)
{
    // some of these are overwritten when mosaic is loaded/changed
    setFrameSize(VIEW_TILING,sz);
    setFrameSize(VIEW_TILING_MAKER,sz);
    setFrameSize(VIEW_DESIGN_ELEMENT,sz);
    setFrameSize(VIEW_PROTOTYPE,sz);
}

void  View::setAllMosaicActiveSizes(QSize sz)
{
    // this overwrites some set by tiling
    setActiveSize(VIEW_DESIGN,sz);
    setActiveSize(VIEW_MOSAIC,sz);
    setActiveSize(VIEW_PROTOTYPE,sz);
    setActiveSize(VIEW_DESIGN_ELEMENT,sz);
    setActiveSize(VIEW_TILING,sz);
}

void View::setAllTilingActiveSizes(QSize sz)
{
    // some of these are overwritten when mosaic is loaded/changed
    setActiveSize(VIEW_TILING,sz);
    setActiveSize(VIEW_TILING_MAKER,sz);
    setActiveSize(VIEW_DESIGN_ELEMENT,sz);
    setActiveSize(VIEW_PROTOTYPE,sz);
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
        config->showCenter = false;
    }
    else
    {
        config->showCenter = true;
    }
    update();
}


void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
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

void View::drawForeground(QPainter *painter, const QRectF & r)
{
    QRectF rect = r;    // rect is modifiable;

    Configuration * config = Configuration::getInstance();
    if (!config->showGrid)
    {
        return;
    }

    // draw a grid
    if (config->gridType == GRID_SCREEN)
    {
        gridPen.setWidth(config->gridScreenWidth);
        painter->setPen(gridPen);
        if (config->gridScreenCenter)
        {
            drawGridSceneUnitsCentered(painter,rect);
        }
        else
        {
            drawGridSceneUnits(painter,rect);
        }
    }
    else
    {
        Q_ASSERT(config->gridType == GRID_MODEL);
        gridPen.setWidth(config->gridModelWidth);
        painter->setPen(gridPen);
        if (config->gridModelCenter)
        {
            drawGridModelUnitsCentered(painter,rect);
        }
        else
        {
            drawGridModelUnits(painter,rect);
        }
    }

    // draw X and center
    painter->drawLine(rect.topLeft(),   rect.bottomRight());
    painter->drawLine(rect.bottomLeft(),rect.topRight());
    QPointF center = rect.center();
    painter->drawEllipse(center,10,10);
}

void View::drawGridModelUnits(QPainter *painter, const QRectF &r)
{
    // this centers on scene
    QTransform T;
    qreal step   = config->gridModelSpacing;
    if (layers.size())
    {
        LayerPtr l = layers.first();
        T = l->getLayerTransform();
    }
    GeoGraphics gg(painter,T);

    // horizontal
    for (qreal i = (-20.0 * step); i < (20 * step); i += step)
    {
        gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
    }

    // vertical
    for (qreal j = (-20.0 * step); j < (20 * step); j += step)
    {
        gg.drawLine(j, -r.height()/2, j, r.height()/2,gridPen);
    }
}

void View::drawGridModelUnitsCentered(QPainter *painter, QRectF &r)
{
    // this centers on layer center
    QTransform T;
    QPointF center;
    if (layers.size())
    {
        LayerPtr l  = layers.first();
        T           = l->getLayerTransform();
        center      = l->getCenterModel();
    }
    qreal step      = config->gridModelSpacing;
    qreal scale     = Transform::scalex(T);
    step *= scale;
    r.moveCenter(center);

    painter->setPen(gridPen);

    // horizontal
    for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
    {
        painter->drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y));
    }

    // vertical
    for (qreal x = center.x() + (-20.0 * step); x <  (center.x() +(20 * step)); x += step)
    {
        painter->drawLine(QPointF(x, r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
    }
}

void View::drawGridSceneUnits(QPainter *painter, const QRectF &r)
{
    QPointF center = r.center();
    qreal step = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    // draw horizontal lines
    qreal y = center.y() - ((r.height()/2) * step);
    while (y < r.height())
    {
        painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
        y += step;
    }

    // draw vertical lines
    qreal x = center.x() - ((r.width()/2) * step);
    while (x < r.width())
    {
        painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
        x += step;
    }
}

void View::drawGridSceneUnitsCentered(QPainter *painter, QRectF & r)
{
    QPointF center;
    if (layers.size())
    {
        LayerPtr l = layers.first();
        center = l->getCenterScreen();
    }
    r.moveCenter(center);

    qreal step = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    // draw horizontal lines
    qreal y = center.y() - ((r.height()/2) * step);
    while (y < r.height())
    {
        painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
        y += step;
    }

    // draw vertical lines
    qreal x = center.x() - ((r.width()/2) * step);
    while (x < r.width())
    {
        painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
        x += step;
    }
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
        for (auto layer : layers)
        {
            qDebug() << "Layer:" << layer->getName() << Utils::addr(layer.get());
        }
    }

    qDebug() << "Tilings:" << Tiling::refs << "Layers:" << Layer::refs  << "Styles:" << Style::refs << "Maps:" << Map::refs << "Protos:" << Prototype::refs << "DELs:" << DesignElement::refs  << "PDELs:" << PlacedDesignElement::refs2 << "Figures:" << Figure::refs << "Features:" << Feature::refs << "Edges:" << Edge::refs << "Vertices:"  << Vertex::refs;
}

void View::setKbdMode(eKbdMode mode)
{
    config->kbdMode = ControlPanel::getValidKbdMode(mode);
    qDebug().noquote() << sKbdMode[config->kbdMode];
    emit sig_kbdMode(config->kbdMode);
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

    bool isALT = ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER);

    bool rv = ProcNavKey(k->key(),delta,isALT);
    if (rv)
    {
        return true;
    }

    rv =  ProcKey(k,isALT);
    return rv;
}

bool View::ProcNavKey(int key, int delta, bool isALT)
{
    switch (key)
    {
    case Qt::Key_Up:    ProcKeyUp(delta,isALT);     return true;
    case Qt::Key_Down:  ProcKeyDown(delta,isALT);   return true;
    case Qt::Key_Left:  ProcKeyLeft(delta, isALT);  return true;
    case Qt::Key_Right: ProcKeyRight(delta, isALT); return true;

    case '.':           designCtrl->designScale(-delta); emit sig_deltaScale(delta); return true; // scale down
    case '>':           designCtrl->designScale(-delta); emit sig_deltaScale(delta); return true; // scale down

    case ',':           designCtrl->designScale( delta); emit sig_deltaScale(-delta); return true;  // scale up
    case '<':           designCtrl->designScale( delta); emit sig_deltaScale(-delta); return true;  // scale up

    case '-':           designCtrl->designRotate(-delta,true); emit sig_deltaRotate(-delta); return true; // rotate left
    case '_':           designCtrl->designRotate(-delta,true); emit sig_deltaRotate(-delta); return true; // rotate left
    case '=':           designCtrl->designRotate( delta,true); emit sig_deltaRotate( delta); return true; // rotate right
    case '+':           designCtrl->designRotate( delta,true); emit sig_deltaRotate( delta); return true; // rotate right

    default: return false;
    }
}

bool View::ProcKey(QKeyEvent *k, bool isALT)
{
#if 1
    static int val = 0;

    int key = k->key();
    switch (key)
    {
    case 'A':  setKbdMode(KBD_MODE_ORIGIN); break;
    case 'B':  setKbdMode(KBD_MODE_OFFSET); break;
    case 'D':  duplicateView(); break;
    case 'E':  emit sig_viewWS(); break;    // just for debug
    case 'F':  config->debugReplicate = !config->debugReplicate; emit sig_figure_changed(); break;
    case 'G':  config->showGrid = !config->showGrid; update(); break;
    case 'H':  config->hideCircles = !config->hideCircles; update(); break;
    case 'I':  designCtrl->designLayerShow(); break;  // I=in
    case 'K':  config->debugMapEnable = !config->debugMapEnable; emit sig_figure_changed(); emit sig_viewWS(); break;
    case 'L':  setKbdMode(KBD_MODE_LAYER); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  {ControlPanel * panel = ControlPanel::getInstance(); TiledPatternMaker * maker = panel->getMaker(); maker->slot_bringToPrimaryScreen(); break;}  //kludge
    case 'O':  designCtrl->designLayerHide(); break; // o=out
    case 'P':  emit sig_saveImage(); break;
    case 'Q':  if (Cycler::getInstance()->getMode() != CYCLE_NONE) { emit sig_cyclerQuit(); }
               else { QApplication::quit(); }
               break;
    case 'R':  if (isALT) { designCtrl->startTimer(); setKbdMode(KBD_MODE_UNDEFINED); } break;
    case 'S':  if (isALT) { designCtrl->stopTimer();  setKbdMode(KBD_MODE_STEP); }
                else { setKbdMode(KBD_MODE_SEPARATION); }
                break;
    case 'T':  setKbdMode(KBD_MODE_XFORM_TILING);break;
    case 'U':  setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  setKbdMode(KBD_MODE_XFORM_FEATURE);break;
    case 'X':  config->circleX = !config->circleX; emit sig_viewWS(); update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  setKbdMode(KBD_MODE_ZLEVEL); break;

    case Qt::Key_Return: if (config->kbdMode == KBD_MODE_STEP) designCtrl->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: setKbdMode(KBD_MODE_UNDEFINED); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Shortcuts");
        box->setText(Shortcuts::getCanvasShortcuts());
        box->setModal(false);
        box->show();
    }
        break;
    case Qt::Key_F2: setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F3: break;
    case Qt::Key_F4: dump(true); break;
    case Qt::Key_F5: drainTheSwamp(); break;
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
           designCtrl->designLayerSelect(key-'0');
        }
        else if (config->kbdMode == KBD_MODE_STEP)
        {
            val *= 10;
            val += (key - '0');
        }
        else
        {
            designCtrl->designToggleVisibility(key-'0');
        }
        break;
    default:
        return false;
    }
#endif
    return true;
}

void View::ProcKeyUp(int delta, bool isALT)
{
    // up arrow
    switch (config->kbdMode)
    {
    case KBD_MODE_CENTER:
    case KBD_MODE_UNDEFINED:
        break;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_FEATURE:
        emit sig_deltaMoveY(-delta);   // goes to Layer::slot_moveY  page_position::onEnter  tilingMaker:slot_moveY (for background)
    default:
        designCtrl->ProcKeyUp(delta,isALT);
        break;
    }
}

void View::ProcKeyDown(int delta, bool isALT)
{
    // down arrrow
    switch (config->kbdMode)
    {
    case KBD_MODE_CENTER:
    case KBD_MODE_UNDEFINED:
        break;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_FEATURE:
        emit sig_deltaMoveY(delta);
        break;
    default:
        designCtrl->ProcKeyDown(delta,isALT);
        break;
    }
}

void View::ProcKeyLeft(int delta, bool isALT)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_CENTER:
    case KBD_MODE_UNDEFINED:
        break;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_FEATURE:
        emit sig_deltaMoveX(-delta);
        break;
    default:
        designCtrl->ProcKeyLeft(delta,isALT);
        break;
    }
}

void View::ProcKeyRight(int delta, bool isALT)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_CENTER:
    case KBD_MODE_UNDEFINED:
        break;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_FEATURE:
        emit sig_deltaMoveX( delta);
        break;
    default:
        designCtrl->ProcKeyRight(delta,isALT);
        break;
    }
}

void View::drainTheSwamp()
{
    Workspace * wspace = Workspace::getInstance();

    dump(true);
    emit sig_unload();
    dump(true);
    wspace->slot_clearWorkspace();
    dump(true);
    clearView();
    dump(true);
    update();
}
