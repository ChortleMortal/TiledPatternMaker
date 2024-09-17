#include <QApplication>
#include <QMessageBox>

#include "gui/top/view.h"
#include "gui/map_editor/map_editor.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/crop_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/widgets/mouse_mode_widget.h"
#include "gui/widgets/transparent_widget.h"
#include "legacy/design_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "sys/engine/image_engine.h"
#include "sys/engine/stepping_engine.h"
#include "sys/geometry/crop.h"
#include "sys/sys.h"
#include "sys/sys/load_unit.h"
#include "sys/tiledpatternmaker.h"

extern class TiledPatternMaker * theApp;

using std::make_shared;

View::View() : QWidget()
{
    config       = Sys::config;
    isShown      = false;
    dragging     = false;

    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

    _suspendPaintApp   = 0;
    _suspendPaintDebug = 0;
    _suspendPaintView  = 0;

    // AFAIK this is only used by show original PNGs - but is harmless
    QGridLayout * grid = new QGridLayout();
    setLayout(grid);
}

View::~View()
{
    qInfo() << __FUNCTION__;

    if (!config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue((QString("viewPos/%1").arg(Sys::appInstance)),pt);
        qInfo() << __FUNCTION__ << "pos" << pt;
    }

}

void View::init(ViewController *parent)
{
    viewControl = parent;
    tilingMaker = Sys::tilingMaker;
    designMaker = Sys::designMaker;
    panel       = Sys::controlPanel;

    connect(this, &View::sig_messageBox, panel, &ControlPanel::slot_messageBox, Qt::QueuedConnection);

    _viewSize = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);
    if (config->splitScreen)
    {
        setFixedSize(_viewSize);
    }
    else
    {
        setSize(_viewSize);
    }

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::init tl =" << _tl << "br =" << _br;
}

void View::slot_raiseView()
{
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();
}

void View::addLayer(LayerPtr layer)
{
    activeLayers.add(layer.get());
}

void View::addLayer(Layer * layer)
{
    activeLayers.add(layer);
}

void View::unloadView()
{
    qDebug() << __FUNCTION__;
    viewSuspendPaint(true);
    clearLayers();
    clearLayout();
    _painterCrop.reset();
    viewSuspendPaint(false);
}

// Required for Boders and Crops
// NOTE ws not thred safe when was used by Tiling Monitor
bool View::isActiveLayer(eViewType type)
{
    return activeLayers.contains(type);
}

Layer * View::getActiveLayer(eViewType type)
{
    return activeLayers.get(type);
}

bool View::viewCanPaint()
{
    return (( _suspendPaintView || _suspendPaintApp || _suspendPaintDebug) ? false : true) ;
}

bool View::splashCanPaint()
{
    return ((_suspendPaintApp || _suspendPaintDebug) ? false : true) ;
}

void View::setPaintDisable(bool disable)
{
    setAttribute(Qt::WA_NoSystemBackground,disable);
    setAutoFillBackground(!disable);
    viewSuspendPaint(disable);
}

void View::appSuspendPaint(bool suspend)
{
    if (suspend)
    {
        _suspendPaintApp++;
    }
    else if (_suspendPaintApp > 0)
    {
        _suspendPaintApp--;
    }
}

void View::debugSuspendPaint(bool suspend)
{
    if (suspend)
    {
        _suspendPaintDebug++;
    }
    else if (_suspendPaintDebug > 0)
    {
        _suspendPaintDebug--;
    }
}

void View::viewSuspendPaint(bool suspend)
{
    if (suspend)
    {
        _suspendPaintView++;
    }
    else if (_suspendPaintView > 0)
    {
        _suspendPaintView--;
    }
}

void View::duplicateView()
{
    QPixmap pixmap(size());
    pixmap.fill((Qt::transparent));

    QPainter painter(&pixmap);
    render(&painter);

    QImage img =  pixmap.toImage();
    QPixmap pixmap2 = ImageEngine::createTransparentPixmap(img);
    
    TransparentImageWidget * tw = new TransparentImageWidget("Duplicate");
    tw->resize(size());
    tw->setPixmap(pixmap2);
    tw->show();
}

void View::setViewBackgroundColor(QColor color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    //setAutoFillBackground(true);
    setPalette(pal);
}

QColor View::getViewBackgroundColor()
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
    if (layout == nullptr)
        return;

    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
            {
                widget->deleteLater();
            }
        }
        if (QLayout* childLayout = item->layout())
        {
            clearLayout(childLayout, deleteWidgets);
        }
        delete item;
    }
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
    case Qt::Key_Up:    ProcKeyUp(delta);    return true;
    case Qt::Key_Down:  ProcKeyDown(delta);  return true;
    case Qt::Key_Left:  ProcKeyLeft(delta);  return true;
    case Qt::Key_Right: ProcKeyRight(delta); return true;

    case '.':
    case '>':
        emit sig_deltaRotate( delta);
        QWidget::update();
        return true;  // scale down

    case ',':
    case '<':
        emit sig_deltaRotate(-delta);
        QWidget::update();
        return true;  // scale up

    case '-':
    case '_':
        emit sig_deltaScale(-delta);
        QWidget::update();
        return true; // rotate left

    case '=':
    case '+':
        emit sig_deltaScale(delta);
        QWidget::update();
        return true; // rotate right

    default: return false;
    }
}

bool View::ProcKey(QKeyEvent *k)
{
    static int val = 0;

    int key = k->key();
    switch (key)
    {
    case 'A':  Sys::guiModes->setKbdMode(KBD_MODE_DES_ORIGIN); break;
    case 'B':  Sys::guiModes->setKbdMode(KBD_MODE_DES_OFFSET); break;
    case 'D':  duplicateView(); break;
    case 'E':  viewControl->slot_reconstructView(); break;    // just for debug
    case 'F':  break;
    case 'G':  config->showGrid = !config->showGrid; viewControl->slot_reconstructView(); break;
    case 'H':  Sys::hideCircles = !Sys::hideCircles; config->showCenterDebug = !config->showCenterDebug; QWidget::update(); break;
    case 'I':  designMaker->designLayerShow(); break;  // I=in
    case 'J':  emit sig_saveMenu(); break;
    case 'K':  Sys::debugView->show(!Sys::debugView->getShow()); emit sig_rebuildMotif(); break;
    case 'L':  Sys::guiModes->setKbdMode(KBD_MODE_DES_LAYER_SELECT); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  theApp->slot_bringToPrimaryScreen(); break;
    case 'O':  designMaker->designLayerHide(); break; // o=out
    case 'P':  if (SteppingEngine::isRunning())
                    emit sig_stepperPause();
                else
                    emit sig_saveImage();
                break;
    case 'Q':   if (Sys::localCycle == true)
                    Sys::localCycle = false;
                else if (SteppingEngine::isRunning())
                    emit sig_stepperEnd();
               else
                    emit sig_close();
               break;
    case 'R':  Sys::dontReplicate = !Sys::dontReplicate; emit sig_rebuildMotif(); break;
    case 'S':  Sys::guiModes->setKbdMode(KBD_MODE_DES_SEPARATION); break;
    case 'T':  Sys::guiModes->setKbdMode(KBD_MODE_XFORM_TILING); break;
    case 'U':  Sys::guiModes->setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  Sys::guiModes->setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  Sys::guiModes->setKbdMode(KBD_MODE_XFORM_UNIQUE_TILE);break;
    case 'X':  Sys::circleX = !Sys::circleX; viewControl->slot_reconstructView(); QWidget::update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  Sys::guiModes->setKbdMode(KBD_MODE_DES_ZLEVEL); break;

    case Qt::Key_Return: if (Sys::guiModes->getKbdMode(KBD_MODE_DES_STEP)) designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: Sys::guiModes->setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Shortcuts");
        if (viewControl->isEnabled(VIEW_DESIGN))
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
    case Qt::Key_F2: Sys::guiModes->setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F3: Sys::guiModes->setKbdMode(KBD_MODE_MOVE); break;
    case Qt::Key_F4: Sys::dumpRefs(); break;
    case Qt::Key_F5: break;
    case Qt::Key_Space: if (SteppingEngine::isRunning()) emit sig_stepperKey(key); break;
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
        if (Sys::guiModes->getKbdMode(KBD_MODE_DES_LAYER_SELECT))
        {
            designMaker->designLayerSelect(key-'0');
        }
        else if (Sys::guiModes->getKbdMode(KBD_MODE_DES_STEP))
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
    if (Sys::guiModes->getKbdMode(KBD_MODE_MOVE))
    {
        auto p = pos();
        move(p.x(),p.y()-1);
    }
    else
    {
        emit sig_deltaMoveY(-delta);
    }
    QWidget::update();
}

void View::ProcKeyDown(int delta)
{
    if (Sys::guiModes->getKbdMode(KBD_MODE_MOVE))
    {
        auto p = pos();
        move(p.x(),p.y()+1);
    }
    else
    {
        emit sig_deltaMoveY(delta);
    }
    QWidget::update();
}

void View::ProcKeyLeft(int delta)
{
    if (Sys::guiModes->getKbdMode(KBD_MODE_MOVE))
    {
        auto p = pos();
        move(p.x()-1,p.y());
    }
    else
    {
        emit sig_deltaMoveX(-delta);
    }
    QWidget::update();
}

void View::ProcKeyRight(int delta)
{
    if (Sys::guiModes->getKbdMode(KBD_MODE_MOVE))
    {
        auto p = pos();
        move(p.x()+1,p.y());
    }
    else
    {
        emit sig_deltaMoveX( delta);
    }
    QWidget::update();
}

//////////////////////////////////////////////////
///
/// Events
///
//////////////////////////////////////////////////

void View::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!isShown)
    {
        // first time only
        QSettings s;
        QPoint pt = s.value(QString("viewPos/%1").arg(Sys::appInstance)).toPoint();
        setGeometry(pt.x(),pt.y(),Sys::DEFAULT_WIDTH,Sys::DEFAULT_HEIGHT);
        qDebug() << "View::showEvent moving to:" << pt;
        move(pt);
        isShown = true;
    }
    QWidget::showEvent(event);
}

void View::closeEvent(QCloseEvent *event)
{
    qInfo() << __FUNCTION__;
    QWidget::closeEvent(event);
    emit sig_close();
}

void View::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);     // required to paint children such as TPMSplash

    if (!viewCanPaint())
    {
        //qDebug() << "View::paintEvent - discarded";
        return;
    }

    //qDebug() << "View::paintEvent: layers=" << activeLayers.size() << "viewRect" << rect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto crop = getClip();
    if (crop)
    {
        auto transform = Sys::cropViewer->getLayerTransform();
        switch (crop->getCropType())
        {
        case CROP_RECTANGLE:
        {
            auto rect = crop->getRect();
            rect = transform.mapRect(rect);
            qInfo() << "Painter Clip:" << rect;
            painter.setClipRect(rect);
        }   break;

        case CROP_POLYGON:
        {
            QPolygonF p = crop->getAPolygon().get();
            QPolygonF p2 = transform.map(p);
            //painter.setClipRegion(p2);
            QPainterPath pp;
            pp.addPolygon(p2);
            painter.setClipPath(pp);
        }   break;

        case CROP_CIRCLE:
        {
            Circle c = crop->getCircle();
            QRectF rect = c.boundingRect();
            rect = transform.mapRect(rect);
            QPainterPath p;
            p.addEllipse(rect.x(),rect.y(),rect.width(),rect.height());
            painter.setClipPath(p);
        }   break;

        case CROP_UNDEFINED:
            break;
        }
    }

    // paints the View
    activeLayers.paint(painter);

    if (Sys::loadUnit->loadTimer.isValid())
    {
        qint64 delta = Sys::loadUnit->loadTimer.elapsed();
        double qdelta = delta /1000.0;
        QString str = QString("%1").arg(qdelta, 8, 'f', 3, QChar(' '));

        qInfo().noquote() << "Load operation for" << Sys::loadUnit->getLoadFile().getVersionedName().get() << "took" << str << "seconds";
        Sys::loadUnit->loadTimer.invalidate();
    }

    //qDebug() << "View::paintEvent: end";
}

// every resize event has a move event afterwards
// but every move event does not necessarily have a resize event
void View::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    //qDebug().noquote() << "View::moveEvent from =" << event->oldPos() << "to =" << event->pos();
    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::moveEvent tl =" << _tl << "br =" << _br;

    emit sig_viewMoved();   // for borders
}

void View::resizeEvent(QResizeEvent *event)
{
    bool dbg = false;
    if (dbg)
    {
        qDebug() << __FUNCTION__ << " - start";
        qDebug() <<  "    from =" << event->oldSize() << "to = " << event->size() << ((event->spontaneous()) ? "from sys" : "from app");
    }

    QSize oldSize = _viewSize;
    _viewSize      = size();

    if (_viewSize == QSize(0,0))
    {
        return;
    }

    QRect rect = geometry();
    QPointF tl = mapToGlobal(rect.topLeft());
    QPointF br = mapToGlobal(rect.bottomRight());

    if  ((_viewSize == oldSize) || !event->spontaneous())
    {
        _tl = tl;
        _br = br;
        if (dbg) qDebug() << "    no change tl =" << _tl << "br =" << _br;
        return;
    }

    // size has changed

    // NOTE: a + is expansion
    int deltaLEFT   = 0;
    int deltaTOP    = 0;
    int deltaRIGHT  = 0;
    int deltaBOTTOM = 0;

    if (tl.x()!= _tl.x())
    {
        deltaLEFT = -(tl.x() - _tl.x());
        if (dbg) qDebug() << "    left changed" << deltaLEFT;
    }
    if (br.x() != _br.x())
    {
        deltaRIGHT = br.x() - _br.x();
        if (dbg) qDebug() << "    right changed" << deltaRIGHT;
        // do nothing
    }
    if (tl.y() != _tl.y())
    {
        deltaTOP = -(tl.y() - _tl.y());
        if (dbg) qDebug() << "    top changed" << deltaTOP;
    }
    if (br.y() != _br.y())
    {
        deltaBOTTOM = br.y() - _br.y();
        if (dbg) qDebug() << "    bottom changed" << deltaBOTTOM;
        // do nothing
    }

    QSize deltaSize = _viewSize - oldSize;
    if (dbg) qDebug() << "    old" << oldSize << "new" << _viewSize << "delta-size" << deltaSize;

    _tl = tl;
    _br = br;
    //qDebug() << "View::resize change to tl =" << _tl << "br =" << _br;

    if (config->scaleToView)
    {
        if (dbg) qDebug() << "    old" << oldSize << "new" << _viewSize << "delta-size-adjusted" << deltaSize;
        Canvas & canvas = viewControl->getCanvas();
        canvas.setDeltaCanvasSize(deltaSize);
    }
    else
    {
        if (deltaLEFT)
            emit sig_deltaMoveX(deltaLEFT);
        if (deltaTOP)
            emit sig_deltaMoveY(deltaTOP);
    }

    emit sig_viewSizeChanged(oldSize,_viewSize);      // for splitscreen, backgroundImageView, and Borders

    if (dbg) qDebug() << __FUNCTION__ << "- end";

    QWidget::update();
}

void View::keyPressEvent( QKeyEvent *k )
{
    // Image (pixmap) views
    if (viewControl->hasImages() && ImageEngine::validViewKey(k))
    {
        viewControl->procImgViewKey(k);
        return;
    }

    // tiling maker
    if (tilingMaker->procKeyEvent(k))
    {
        return;
    }

    // map editor
    if (Sys::mapEditor->procKeyEvent(k))
    {
        return;
    }

    // system keys
    procKeyEvent(k);
}

void View::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "View::mousePressEvent";
    if (event->button() == Qt::MiddleButton)
    {
        return; // discards middle button on wheel
    }

    dragging = true;

    QPointF gPos;   // local  Position
    QPointF lPos;   // global Position

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    gPos = event->globalPosition();
    lPos = event->position();
#else
    gPos = event->globalPos();
    lPos = event->localPos();
#endif

    if (Sys::guiModes->getMouseMode(MOUSE_MODE_CENTER) && event->button() == Qt::LeftButton)
    {
        emit sig_setCenter(lPos);
        Sys::guiModes->setMouseMode(MOUSE_MODE_CENTER,false);
        viewControl->slot_reconstructView();
        panel->getMouseModeWidget()->display();
    }
    else if (Sys::guiModes->getMouseMode(MOUSE_MODE_TRANSLATE))
    {
        sLast = gPos;
    }
    else
    {
       emit sig_mousePressed(lPos,event->button());
    }

    QWidget::update();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "View::mouseMoveEvent";

    QPointF gPos;   // local  Position
    QPointF lPos;   // global Position

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    gPos = event->globalPosition();
    lPos = event->position();
#else
    gPos = event->globalPos();
    lPos = event->localPos();
#endif

    if (dragging)
    {
        emit sig_mouseDragged(lPos);

        if (Sys::guiModes->getMouseMode(MOUSE_MODE_TRANSLATE))
        {
            QPointF spt  = gPos;
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(translate);
            QWidget::update();
        }
    }
    else
    {
        emit sig_mouseMoved(lPos);
    }
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
    QWidget::update();
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

    if (Sys::guiModes->getMouseMode(MOUSE_MODE_SCALE))
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
        QWidget::update();
    }
    else if (Sys::guiModes->getMouseMode(MOUSE_MODE_ROTATE))
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
        QWidget::update();
    }
}

void View::setWindowTitle(const QString & s)
{
    QWidget::setWindowTitle(s);
    QWidget::update();
}

void View::slot_update()
{
    QWidget::update();
}

void View::slot_repaint()
{
    QWidget::repaint();
}

void View::setFixedSize(QSize sz)
{
    QWidget::setFixedSize(sz);
}

void View::setSize(QSize sz)
{
    qDebug() << "View::setSize:" << sz;

    if (config->limitViewSize)
    {
        QScreen * pri = QGuiApplication::primaryScreen();
        QSize size = pri->availableSize();
        qDebug() << "Available size" << size;
        if (sz.width() > size.width())
        {
            sz.setWidth(size.width());
        }
        if (sz.height() > size.height())
        {
            sz.setHeight(size.height());
        }
    }

#if defined(Q_OS_WINDOWS)

    QWidget::resize(sz);

    viewSuspendPaint(true);
    qApp->processEvents();
    viewSuspendPaint(false);
    if (sz != size())
    {
        QString astring;
        QDebug ts(&astring);
        ts << "Requested size" << sz << "Actual size" << size();
        emit sig_messageBox("View has been resized by Windows",astring);
    }
#else
    QWidget::resize(sz);
#endif
}


void View::flash(QColor color)
{
    // flash new color
    setViewBackgroundColor(color);
    QWidget::repaint();

    // wait
    QThread::msleep(350);

    // restore color
    auto viewControl = Sys::viewController;
    color = viewControl->getCanvas().getBkgdColor();
    setViewBackgroundColor(color);
    QWidget::repaint();
}

////////////////////////////////////////////////////////////////////////////////
///
/// ActiveLayers
///
////////////////////////////////////////////////////////////////////////////////

void ActiveLayers::add(Layer * layer)
{
    QMutexLocker locker(&mutex);

    //qDebug().noquote() << "adding layer :" << layer->getLayerName();
    layers.push_back(layer);
}

void  ActiveLayers::  clear()
{
    QMutexLocker locker(&mutex);

    layers.clear();
}

bool ActiveLayers::contains(eViewType type)
{
    QMutexLocker locker(&mutex);

    for (Layer * layer : std::as_const(layers))
    {
        if (layer->iamaLayer() == type)
        {
            return true;
        }
    }
    return false;
}

Layer * ActiveLayers::get(eViewType type)
{
    QMutexLocker locker(&mutex);

    for (Layer * layer: std::as_const(layers))
    {
        if (layer->iamaLayer() == type)
        {
            return layer;
        }
    }
    return nullptr;
}

const  QVector<Layer*> ActiveLayers::get()
{
    QMutexLocker locker(&mutex);

    return layers;
}

void ActiveLayers::paint(QPainter & painter)
{
    QMutexLocker locker(&mutex);

#if 0
    int  count = layers.size();
    qDebug() <<  __FUNCTION__ << "count = " << count;

    for (Layer * layer : std::as_const(layers))
    {
        QString name =  layer->getLayerName();
        qDebug() <<  __FUNCTION__ << "before sort" << name;
    }
#endif

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevelP);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (Layer * layer : std::as_const(layers))
    {
        if (layer->isVisible())
        {
            //QString name =  layer->getLayerName();
            //qDebug() <<  __FUNCTION__ << name;
            layer->forceLayerRecalc(false);
            painter.save();
            layer->paint(&painter);
            painter.restore();
        }
    }
}

