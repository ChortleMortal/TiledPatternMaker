#include <QApplication>
#include <QMessageBox>

#include "engine/image_engine.h"
#include "engine/stepping_engine.h"
#include "legacy/design_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/shortcuts.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/view.h"
#include "viewers/view_controller.h"
#include "widgets/mouse_mode_widget.h"
#include "widgets/transparent_widget.h"

extern class TiledPatternMaker * theApp;

using std::make_shared;

View::View()
{
    config       = Configuration::getInstance();
    isShown      = false;
    dragging     = false;
    iMouseMode   = MOUSE_MODE_NONE;
    keyboardMode = KBD_MODE_XFORM_VIEW;

    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
    setCanPaint(true);
    setAppPaint(true);

    // AFIK this is only used by show original PNGs - but is harmless
    QGridLayout * grid = new QGridLayout();
    setLayout(grid);
}

View::~View()
{
//    qDebug() << "View destructor";
}

void View::init(ViewController *parent)
{
    viewControl = parent;
    tilingMaker = TilingMaker::getInstance();
    designMaker = DesignMaker::getInstance();
    panel       = ControlPanel::getInstance();

    viewSize= QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);
    if (config->splitScreen)
    {
        setFixedSize(viewSize);
    }
    else
    {
        resize(viewSize);
    }

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::init tl =" << _tl << "br =" << _br;
}

void View::addLayer(LayerPtr layer)
{
    qDebug().noquote() << "adding layer :" << layer->getLayerName();
    activeLayers.push_back(layer.get());
}

void View::addLayer(Layer * layer)
{
    qDebug().noquote() << "adding layer :" << layer->getLayerName();
    activeLayers.push_back(layer);
}

void View::addTopLayer(LayerPtr layer)
{
    qDebug().noquote() << "adding top layer :" << layer->getLayerName();
    activeLayers.push_front(layer.get());
}

void View::unloadView()
{
    setCanPaint(false);
    clearLayers();
    clearLayout();
    setCanPaint(true);
}

bool View::isActiveLayer(Layer * l)
{
    for (const auto layer : activeLayers)
    {
        if (layer == l)
        {
            return true;
        }
    }
    return false;
}

QVector<Layer*> View::getActiveLayers()
{
    return activeLayers;
}

Layer * View::getActiveLayer(eViewType type)
{
    for (Layer * layer: std::as_const(activeLayers))
    {
        if (layer->iamaLayer() == type)
        {
            return layer;
        }
    }
    return nullptr;
}

void View::setPaintEnable(bool enable)
{
    setAttribute(Qt::WA_NoSystemBackground,!enable);
    setAutoFillBackground(enable);
    setCanPaint(enable);
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

bool View::getMouseMode(eMouseMode mode)
{
    if (mode == MOUSE_MODE_NONE)
        return iMouseMode == 0;
    else
        return (iMouseMode & mode) > 0;
}

void View::setMouseMode(eMouseMode newMode, bool set)
{
    static unsigned int lastMode = 0;

    qDebug() << "MouseMode:" << newMode;

    if (set)
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            iMouseMode = 0;
            break;

        case MOUSE_MODE_CENTER:
            lastMode = iMouseMode;
            iMouseMode = MOUSE_MODE_CENTER;
            break;
        case MOUSE_MODE_TRANSLATE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
            iMouseMode &= ~MOUSE_MODE_SCALE;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
            iMouseMode &= ~MOUSE_MODE_ROTATE;
            iMouseMode |= newMode;
            break;
        }
    }
    else
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            break;

        case MOUSE_MODE_CENTER:
            iMouseMode = lastMode;
            break;

        case MOUSE_MODE_TRANSLATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~newMode;
            break;
        }
    }

    if (iMouseMode)
        Sys::showCenterMouse = true;
    else
        Sys::showCenterMouse = false;

    update();
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
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void View::setKbdMode(eKbdMode mode)
{
    eKbdMode newMode = panel->getValidKbdMode(mode);
    if (newMode != keyboardMode)
    {
        keyboardMode = newMode;
        qDebug().noquote() << "Keyboard Mode is:" << getKbdModeStr();
        emit sig_kbdMode(keyboardMode);
    }
}

void View::resetKbdMode()
{
    setKbdMode(keyboardMode);   // converts if necessary
    emit sig_kbdMode(keyboardMode);
}

bool View::getKbdMode(eKbdMode mode)
{
    return (mode == keyboardMode);
}

QString View::getKbdModeStr()
{
    return sKbdMode[keyboardMode];
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
        update();
        return true;  // scale down

    case ',':
    case '<':
        emit sig_deltaRotate(-delta);
        update();
        return true;  // scale up

    case '-':
    case '_':
        emit sig_deltaScale(-delta);
        update();
        return true; // rotate left

    case '=':
    case '+':
        emit sig_deltaScale(delta);
        update();
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
    case 'A':  setKbdMode(KBD_MODE_DES_ORIGIN); break;
    case 'B':  setKbdMode(KBD_MODE_DES_OFFSET); break;
    case 'D':  duplicateView(); break;
    case 'E':  viewControl->slot_reconstructView(); break;    // just for debug
    case 'F':  break;
    case 'G':  config->showGrid = !config->showGrid; viewControl->slot_reconstructView(); break;
    case 'H':  Sys::hideCircles = !Sys::hideCircles; config->showCenterDebug = !config->showCenterDebug; update(); break;
    case 'I':  designMaker->designLayerShow(); break;  // I=in
    case 'J':  emit sig_saveMenu(); break;
    case 'K':  Sys::debugMapEnable = !Sys::debugMapEnable; emit sig_rebuildMotif(); break;
    case 'L':  setKbdMode(KBD_MODE_DES_LAYER_SELECT); break;
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
                    QApplication::quit();
               break;
    case 'R':  Sys::dontReplicate = !Sys::dontReplicate; emit sig_rebuildMotif(); break;
    case 'S':  setKbdMode(KBD_MODE_DES_SEPARATION); break;
    case 'T':  setKbdMode(KBD_MODE_XFORM_TILING); break;
    case 'U':  setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  setKbdMode(KBD_MODE_XFORM_UNIQUE_TILE);break;
    case 'X':  Sys::circleX = !Sys::circleX; viewControl->slot_reconstructView(); update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  setKbdMode(KBD_MODE_DES_ZLEVEL); break;

    case Qt::Key_Return: if (getKbdMode(KBD_MODE_DES_STEP)) designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: setKbdMode(KBD_MODE_XFORM_VIEW); break;
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
    case Qt::Key_F2: setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F3: break;
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
        if (getKbdMode(KBD_MODE_DES_LAYER_SELECT))
        {
            designMaker->designLayerSelect(key-'0');
        }
        else if (getKbdMode(KBD_MODE_DES_STEP))
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
    update();
}

void View::ProcKeyDown(int delta)
{
    emit sig_deltaMoveY(delta);
    update();
}

void View::ProcKeyLeft(int delta)
{
    emit sig_deltaMoveX(-delta);
    update();
}

void View::ProcKeyRight(int delta)
{
    emit sig_deltaMoveX( delta);
    update();
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
}

void View::closeEvent(QCloseEvent *event)
{
    qDebug() << "View::closeEvent";
    if (!config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue((QString("viewPos/%1").arg(Sys::appInstance)),pt);
        qDebug() << "View::closeEvent pos" << pt;
    }

    QWidget::closeEvent(event);
    qApp->quit();
}

void View::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!getCanPaint() || !getAppPaint())
    {
        //qDebug() << "View::paintEvent - discarded";
        return;
    }

    qDebug() << "View::paintEvent: layers=" << activeLayers.size() << "viewRect" << rect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    std::stable_sort(activeLayers.begin(),activeLayers.end(),Layer::sortByZlevelP);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (Layer * layer : activeLayers)
    {
        if (layer->isVisible())
        {
            layer->forceLayerRecalc(false);
            painter.save();
            layer->paint(&painter);
            painter.restore();
        }
    }

    if (loadUnit.loadTimer.isValid())
    {
        qint64 delta = loadUnit.loadTimer.elapsed();
        double qdelta = delta /1000.0;
        QString str = QString("%1").arg(qdelta, 8, 'f', 3, QChar(' '));

        qInfo().noquote() << "Load operation for" << loadUnit.getLoadName() << "took" << str << "seconds";
        loadUnit.loadTimer.invalidate();
    }

    //qDebug() << "View::paintEvent: end";
}

// every resize event has a move event afterwards
// but every move event does not necessarily have a resize event
void View::moveEvent(QMoveEvent *event)
{
    qDebug().noquote() << "View::moveEvent from =" << event->oldPos() << "to =" << event->pos();
    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::moveEvent tl =" << _tl << "br =" << _br;

    emit sig_viewMoved();   // for borders
}

void View::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    qDebug() << "View::resizeEvent - start";
    qDebug() <<  "+++ from =" << event->oldSize() << "to = " << event->size() << ((event->spontaneous()) ? "from sys" : "from app");


    QSize oldSize = viewSize;
    viewSize      = size();

    if (viewSize == QSize(0,0))
    {
        return;
    }

    QRect rect = geometry();
    QPointF tl = mapToGlobal(rect.topLeft());
    QPointF br = mapToGlobal(rect.bottomRight());

    if  ((viewSize == oldSize) || !event->spontaneous())
    {
        _tl = tl;
        _br = br;
        qDebug() << "+++ no change tl =" << _tl << "br =" << _br;
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
        qDebug() << "+++ left changed" << deltaLEFT;
    }
    if (br.x() != _br.x())
    {
        deltaRIGHT = br.x() - _br.x();
        qDebug() << "+++ right changed" << deltaRIGHT;
        // do nothing
    }
    if (tl.y() != _tl.y())
    {
        deltaTOP = -(tl.y() - _tl.y());
        qDebug() << "+++ top changed" << deltaTOP;
    }
    if (br.y() != _br.y())
    {
        deltaBOTTOM = br.y() - _br.y();
        qDebug() << "+++ bottom changed" << deltaBOTTOM;
        // do nothing
    }

    QSize deltaSize = viewSize - oldSize;
    qDebug() << "+++ old" << oldSize << "new" << viewSize << "delta-size" << deltaSize;

    _tl = tl;
    _br = br;
    //qDebug() << "View::resize change to tl =" << _tl << "br =" << _br;

    if (config->scaleToView)
    {
        qDebug() << "+++ old" << oldSize << "new" << viewSize << "delta-size-adjusted" << deltaSize;
        Canvas & canvas = viewControl->getCanvas();
        canvas.setDeltaCanvasSize(QSizeF(deltaSize));
    }
    else
    {
        if (deltaLEFT)
            emit sig_deltaMoveX(deltaLEFT);
        if (deltaTOP)
            emit sig_deltaMoveY(deltaTOP);
    }

    emit sig_viewSizeChanged(oldSize,viewSize);      // for splitscreen, backgroundImageView, and Borders

    qDebug() << "View::resizeEvent - end";

    update();
}

void View::keyPressEvent( QKeyEvent *k )
{
    if (tilingMaker->procKeyEvent(k))        // tiling maker
    {
        return;
    }
    if (MapEditor::getInstance()->procKeyEvent(k))    // map editor
    {
        return;
    }
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

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    if (getMouseMode(MOUSE_MODE_CENTER) && event->button() == Qt::LeftButton)
    {
        emit sig_setCenter(event->position());
        setMouseMode(MOUSE_MODE_CENTER,false);
        viewControl->slot_reconstructView();
        panel->getMouseModeWidget()->display();
    }
    else if (getMouseMode(MOUSE_MODE_TRANSLATE))
    {
        sLast = event->globalPosition();
    }
    else
    {
       emit sig_mousePressed(event->position(),event->button());
    }
#else
    if (getMouseMode(MOUSE_MODE_CENTER) && event->button() == Qt::LeftButton)
    {
        emit sig_setCenter(event->localPos());
        setMouseMode(MOUSE_MODE_CENTER,false);
        panel->getMouseModeWidget()->display();
    }
    else if (getMouseMode(MOUSE_MODE_TRANSLATE))
    {
        sLast = event->globalPos();
    }
    else
    {
        emit sig_mousePressed(event->localPos(),event->button());
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

        if (getMouseMode(MOUSE_MODE_TRANSLATE))
        {
            QPointF spt  = event->globalPosition();
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(translate);
            update();
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

        if (getMouseMode(MOUSE_MODE_TRANSLATE))
        {
            QPointF spt  = event->globalPos();
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(translate);
            update();
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
    update();
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

    if (getMouseMode(MOUSE_MODE_SCALE))
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
        update();
    }
    else if (getMouseMode(MOUSE_MODE_ROTATE))
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
        update();
    }
}

void View::setViewTitle(const QString & s)
{
    QWidget::setWindowTitle(s);
}

void View::update()
{
    QWidget::update();
}

void View::repaint()
{
    QWidget::repaint();
}

void View::resize(QSize sz)
{
    qDebug() << "View::resize to =" << sz;

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

    QWidget::resize(sz);
}

LoadUnit::LoadUnit()
{
    loadState = LOADING_NONE;
    lastState = LOADING_NONE;

    config = Configuration::getInstance();
}

void LoadUnit::setLoadState(eLoadState state, QString name)
{
    loadState = state;
    loadName  = name;

    lastState = state;
    lastName  = name;

    switch (state)
    {
    case LOADING_MOSAIC:
        config->lastLoadedMosaic = name;
        break;

    case LOADING_TILING:
        config->lastLoadedTiling = name;
        break;

    case LOADING_LEGACY:
        config->lastLoadedLegacyDes = name;
        break;

    default:
        break;
    }
};
