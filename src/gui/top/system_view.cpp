#include <QApplication>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/panels/shortcuts.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_viewer.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/mouse_mode_widget.h"
#include "gui/widgets/transparent_widget.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "sys/engine/image_engine.h"
#include "sys/engine/stepping_engine.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/debug_map.h"
#include "sys/sys.h"
#include "sys/sys/load_unit.h"
#include "sys/tiledpatternmaker.h"

extern class TiledPatternMaker * theApp;

using std::make_shared;

SystemView::SystemView() : QWidget()
{
    parent       = nullptr;
    isShown      = false;
    dragging     = false;

    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

    _suspendPaintApp   = 0;
    _suspendPaintDebug = 0;
    _suspendPaintView  = 0;

    constrained        = false;
    requestedSize      = QSize();

    // AFAIK this is only used by show original PNGs - but is harmless
    QGridLayout * grid = new QGridLayout();
    setLayout(grid);
}

SystemView::~SystemView()
{
    if (!Sys::config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue((QString("viewPos/%1").arg(Sys::appInstance)),pt);
        qInfo() << "SystemView::~SystemView" << "pos" << pt;
    }
    else
    {
        qInfo() << "SystemView::~SystemView";
    }
}

void SystemView::init(SystemViewController *parent)
{
    this->parent  = parent;

    connect(this, &SystemView::sig_messageBox, Sys::controlPanel, &ControlPanel::slot_messageBox, Qt::QueuedConnection);
    connect(this, &SystemView::sig_raiseMenu,  Sys::controlPanel, &ControlPanel::slot_raisePanel);
    connect(this, &SystemView::sig_testSize,   this,              &SystemView::testSize,          Qt::QueuedConnection);

    QSize sz(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);
    setSize(sz);
    if (Sys::config->splitScreen)
    {
        setFixedSize(sz);
    }
    else
    {
        setSize(sz);
    }

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::init tl =" << _tl << "br =" << _br;
}

void SystemView::raiseView()
{
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    QWidget::raise();
    activateWindow();
}

void SystemView::unloadViewers()
{
    qDebug() << "SystemView::unloadView";
    viewSuspendPaint(true);
    clearLayers();
    clearLayout();
    _painterCrop.reset();
    viewSuspendPaint(false);
}

void SystemView::duplicateView()
{
    QPixmap pixmap(size());
    pixmap.fill((Qt::transparent));

    QPainter painter(&pixmap);
    render(&painter);

    QImage img =  pixmap.toImage();
    QPixmap pixmap2 = ImageEngine::createTransparentPixmap(img);
    
    TransparentImageWidget * tw = new TransparentImageWidget("Duplicate");
    tw->setContentSize(size());
    tw->setPixmap(pixmap2);
    tw->show();
}

void SystemView::setBackgroundColor(QColor color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    //setAutoFillBackground(true);
    setPalette(pal);
}

QColor SystemView::getBackgroundColor()
{
    QPalette pal = palette();
    QColor c = pal.color(QPalette::Window);
    return c;
}

void SystemView::clearLayout()
{
    clearLayout(layout(),true);
}

void SystemView::clearLayout(QLayout* layout, bool deleteWidgets)
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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Painting
///
////////////////////////////////////////////////////////////////////////////////////////////////////

void SystemView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);     // required to paint children such as TPMSplash

    if (!viewCanPaint())
    {
        //qDebug() << "View::paintEvent - discarded";
        return;
    }

    //qDebug() << "View::paintEvent: layers=" << activeLayers.size() << "viewRect" << rect();

    Sys::debugMapPaint->wipeout();  // erase everything at start of each paint (assumes debug view is topmost/last)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto crop = getPainterClip();
    if (crop)
    {
        auto transform = Sys::cropViewer->getLayerTransform();
        crop->setPainterClip(&painter,transform);
    }

    // paints the View
    activeLayers.paint(painter);

    processLoadState(Sys::designMaker->getLoadUnit());
    processLoadState(Sys::tilingMaker->getLoadUnit());
    processLoadState(Sys::mosaicMaker->getLoadUnit());
}

bool SystemView::viewCanPaint()
{
    return (( _suspendPaintView || _suspendPaintApp || _suspendPaintDebug) ? false : true) ;
}

bool SystemView::splashCanPaint()
{
    return ((_suspendPaintApp || _suspendPaintDebug) ? false : true) ;
}

void SystemView::setPaintDisable(bool disable)
{
    setAttribute(Qt::WA_NoSystemBackground,disable);
    setAutoFillBackground(!disable);
    viewSuspendPaint(disable);
}

void SystemView::appSuspendPaint(bool suspend)
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

void SystemView::debugSuspendPaint(bool suspend)
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

void SystemView::viewSuspendPaint(bool suspend)
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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
///     Keybaord
///
////////////////////////////////////////////////////////////////////////////////////////////////////

void SystemView::keyPressEvent(QKeyEvent *k)
{
    // Image (pixmap) views
    if (Sys::imageViewer->isLoaded() && parent->isEnabled(VIEW_BMP_IMAGE))
    {
        if (Sys::imageEngine->slot_imageKeyPressed(k))
            return;     // handled
    }

    if (parent->isEnabled(VIEW_LEGACY))
    {
        if (procLegacyKeyEvent(k))
            return;     // handled
    }

    // tiling maker
    if (Sys::tilingMaker->procKeyEvent(k))
        return;

    // map editor
    if (Sys::mapEditor->procKeyEvent(k))
        return;

    // common navigation
    if (procNavigationKey(k))
        return;

    // common keys
    procCommonKey(k);
}

bool SystemView::procNavigationKey(QKeyEvent * k)
{
    int  delta = 1;
    if ((k->modifiers() & (Qt::SHIFT | Qt::CTRL)) == (Qt::SHIFT | Qt::CTRL))
        delta = 50;
    else if ((k->modifiers() & Qt::SHIFT) == Qt::SHIFT)
        delta = 10;

    switch (k->key())
    {
    case Qt::Key_Up:    procKeyUp(delta);    return true;
    case Qt::Key_Down:  procKeyDown(delta);  return true;
    case Qt::Key_Left:  procKeyLeft(delta);  return true;
    case Qt::Key_Right: procKeyRight(delta); return true;

    case '.':
    case '>':
        emit sig_deltaRotate(Sys::nextSigid(), delta);
        QWidget::update();
        return true;  // scale down

    case ',':
    case '<':
        emit sig_deltaRotate(Sys::nextSigid(), -delta);
        QWidget::update();
        return true;  // scale up

    case '-':
    case '_':
        emit sig_deltaScale(Sys::nextSigid(), -delta);
        QWidget::update();
        return true; // rotate left

    case '=':
    case '+':
        emit sig_deltaScale(Sys::nextSigid(), delta);
        QWidget::update();
        return true; // rotate right

    default:
        return false;
    }
}

bool SystemView::procCommonKey(QKeyEvent *k)
{
    int key = k->key();
    switch (key)
    {
    case 'D':  duplicateView(); break;
    case 'E':  parent->slot_reconstructView(); break;    // just for debug
    case 'G':  Sys::controlPanel->delegateView(VIEW_GRID,!parent->isEnabled(VIEW_GRID)); break;
    case 'H':  Sys::config->showCenterDebug = !Sys::config->showCenterDebug; QWidget::update(); break;
    case 'J':  emit sig_saveMenu(); break;
    case 'K':  Sys::controlPanel->delegateView(VIEW_DEBUG,!parent->isEnabled(VIEW_DEBUG)); emit sig_rebuildMotif(); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  theApp->slot_bringToPrimaryScreen(); break;
    case 'P':  if (SteppingEngine::isRunning())
                {
                    emit sig_stepperPause();
                }
                else
                {
                    if (k->modifiers() & Qt::ControlModifier)
                    {
                        qDebug() << "p-control";
                        emit sig_print();
                    }
                    else
                    {
                        qDebug() << "p-alone";
                        emit sig_saveImage();
                    }
                }
                break;
    case 'Q':   if (Sys::localCycle == true)
                    Sys::localCycle = false;
                else if (SteppingEngine::isRunning())
                    emit sig_stepperEnd();
               else
                    emit sig_close();
               break;
    case 'R':  Sys::dontReplicate = !Sys::dontReplicate; emit sig_rebuildMotif(); break;
    case 'T':  Sys::guiModes->setTMKbdMode(TM_MODE_XFORM_TILING); break;
    case 'V':  Sys::guiModes->setTMKbdMode(TM_MODE_XFORM_ALL); break;
    case 'Y':  emit sig_saveSVG(); break;

    case Qt::Key_Escape: Sys::guiModes->setTMKbdMode(TM_MODE_XFORM_ALL); break;
    case Qt::Key_F1: Shortcuts::popup(VIEW_MOSAIC); break;
    case Qt::Key_F2: Sys::guiModes->setTMKbdMode(TM_MODE_XFORM_ALL); break;
    case Qt::Key_F4: Sys::dumpRefs(); break;
    case Qt::Key_Space: if (SteppingEngine::isRunning()) emit sig_stepperKey(key); break;
    default:
        return false;
    }
    return true;
}

bool SystemView::procLegacyKeyEvent(QKeyEvent *k)
{
    static int val = 0;

    int key = k->key();
    switch (key)
    {
    case 'A':  Sys::guiModes->setLegacyKbdMode(LEGACY_MODE_DES_ORIGIN); break;
    case 'B':  Sys::guiModes->setLegacyKbdMode(LEGACY_MODE_DES_OFFSET); break;
    case 'H':  Sys::hideCircles = !Sys::hideCircles; QWidget::update(); break;
    case 'I':  Sys::designMaker->designLayerShow(); break;  // I=in
    case 'L':  Sys::guiModes->setLegacyKbdMode(LEGACY_MODE_DES_LAYER_SELECT); break;
    case 'O':  Sys::designMaker->designLayerHide(); break; // o=out
    case 'S':  Sys::guiModes->setLegacyKbdMode(LEGACY_MODE_MODE_DES_SEPARATION); break;

    case Qt::Key_Return: if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_STEP)) Sys::designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_F1: Shortcuts::popup(VIEW_LEGACY); break;
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
        if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_LAYER_SELECT))
        {
            Sys::designMaker->designLayerSelect(key-'0');
        }
        else if (Sys::guiModes->getLegacyKbdMode(LEGACY_MODE_DES_STEP))
        {
            val *= 10;
            val += (key - '0');
        }
        else
        {
            Sys::designMaker->designToggleVisibility(key-'0');
        }
        break;
    default:
        return false;
    }
    return true;
}

void SystemView::procKeyUp(int delta)
{
    emit sig_deltaMoveY(Sys::nextSigid(), -delta);
}

void SystemView::procKeyDown(int delta)
{
    emit sig_deltaMoveY(Sys::nextSigid(), delta);
}

void SystemView::procKeyLeft(int delta)
{
    emit sig_deltaMoveX(Sys::nextSigid(), -delta);
}

void SystemView::procKeyRight(int delta)
{
    emit sig_deltaMoveX(Sys::nextSigid(),  delta);
}

/////////////////////////////////////////////////////////////////////////////////
///
///     Mouse
///
/////////////////////////////////////////////////////////////////////////////////

void SystemView::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "View::mousePressEvent";
    if (event->button() == Qt::MiddleButton)
    {
        return; // discards middle button on wheel
    }

    dragging = true;

    QPointF gPos;   // local  Position
    QPointF lPos;   // global Position

    gPos = event->globalPosition();
    lPos = event->position();

    if (Sys::guiModes->getMouseMode(MOUSE_MODE_TRANSLATE))
    {
        sLast = gPos;
    }
    else
    {
        emit sig_mousePressed(lPos,event->button());
    }

    QWidget::update();
}

void SystemView::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "View::mouseMoveEvent";

    QPointF gPos;   // local  Position
    QPointF lPos;   // global Position

    gPos = event->globalPosition();
    lPos = event->position();

    if (dragging)
    {
        emit sig_mouseDragged(lPos);

        if (Sys::guiModes->getMouseMode(MOUSE_MODE_TRANSLATE))
        {
            QPointF spt  = gPos;
            QPointF translate = spt - sLast;
            sLast = spt;
            qDebug() << "dragged" << translate;
            emit sig_mouseTranslate(Sys::nextSigid(),translate);
            QWidget::update();
        }
    }
    else
    {
        emit sig_mouseMoved(lPos);
    }
}

void SystemView::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "View::mouseReleaseEvent";
    dragging = false;
    emit sig_mouseReleased(event->position());
    QWidget::update();
}

void SystemView::mouseDoubleClickEvent(QMouseEvent * event)
{
    dragging = false;

    emit sig_mouseDoublePressed(event->position(),event->button());
}

void SystemView::wheelEvent(QWheelEvent *event)
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
            emit sig_wheel_scale(Sys::nextSigid(), delta);
        else
            emit sig_wheel_scale(Sys::nextSigid(), -delta);
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
            emit sig_wheel_rotate(Sys::nextSigid(), delta); // degrees
        else
            emit sig_wheel_rotate(Sys::nextSigid(), -delta); // degrees
        QWidget::update();
    }
}

//////////////////////////////////////////////////
///
///  Other Events
///
//////////////////////////////////////////////////

void SystemView::showEvent(QShowEvent *event)
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

void SystemView::closeEvent(QCloseEvent *event)
{
    qInfo() << "SystemView::closeEvent";
    QWidget::closeEvent(event);
    emit sig_close();
}

void SystemView::processLoadState(LoadUnit * loadUnit)
{
    eLoadUnitState state = loadUnit->getLoadState();
    if (state == LS_LOADED || state == LS_FAILED)
    {
        if (loadUnit->loadTimer.isValid())
        {
            qint64 delta = loadUnit->loadTimer.elapsed();
            double qdelta = delta /1000.0;
            QString str = QString("%1").arg(qdelta, 8, 'f', 3, QChar(' '));

            qInfo().noquote() << "Load operation for" << loadUnit->getLoadFile().getVersionedName().get() << "took" << str << "seconds";
            loadUnit->loadTimer.invalidate();
        }
        loadUnit->ready();
        emit sig_loadComplete();
    }
}

// every resize event has a move event afterwards
// but every move event does not necessarily have a resize event
void SystemView::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    //qDebug().noquote() << "View::moveEvent from =" << event->oldPos() << "to =" << event->pos();
    QRect rect = geometry();
    _tl = mapToGlobal(rect.topLeft());
    _br = mapToGlobal(rect.bottomRight());
    //qDebug() << "View::moveEvent tl =" << _tl << "br =" << _br;

    emit sig_viewMoved();   // for borders
}

void SystemView::resizeEvent(QResizeEvent *event)
{
    if (event->spontaneous() && requestedSize.isValid() && (requestedSize != event->size()))
    {
        // this is (unwanted) automatic resizing
        requestedSize = QSize();
        qInfo() << "System resizing from" << event->oldSize() << "to" << event->size() << "IGNORED";
        return;
    }

    bool dbg = false;
    if (dbg)
    {
        qInfo() << "SystemView::resizeEvent" << " - start";
        qInfo() <<  "    from =" << event->oldSize() << "to = " << event->size() << ((event->spontaneous()) ? "from SYS" : "from APP");
    }

    QSize oldSize = parent->getCanvasViewSize();
    QSize newSize = size();

    parent->getCanvas().setViewSize(newSize);

    if (newSize == QSize(0,0))
    {
        return;
    }

    QRect rect = geometry();
    QPointF tl = mapToGlobal(rect.topLeft());
    QPointF br = mapToGlobal(rect.bottomRight());

    if  ((newSize == oldSize) || !event->spontaneous())
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

    QSize deltaSize = newSize - oldSize;
    if (dbg) qDebug() << "    old" << oldSize << "new" << newSize << "delta-size" << deltaSize;

    _tl = tl;
    _br = br;
    //qDebug() << "View::resize change to tl =" << _tl << "br =" << _br;

    if (Sys::config->scaleToView)
    {
        if (dbg) qDebug() << "    old" << oldSize << "new" << newSize << "delta-size-adjusted" << deltaSize;
        Canvas & canvas = parent->getCanvas();
        canvas.setDeltaCanvasSize(deltaSize);
    }
    else
    {
        if (deltaLEFT)
            emit sig_deltaMoveX(Sys::nextSigid(), deltaLEFT);
        if (deltaTOP)
            emit sig_deltaMoveY(Sys::nextSigid(), deltaTOP);
    }

    emit sig_viewSizeChanged(oldSize,newSize);      // for splitscreen, backgroundImageView, and Borders

    if (dbg) qDebug() << "SystemView::resizeEvent" << "- end";

    QWidget::update();
}

void SystemView::setWindowTitle(const QString & s)
{
    QWidget::setWindowTitle(s);
    QWidget::update();
}

void SystemView::setFixedSize(QSize sz)
{
    qInfo() << "SystemView::setFixedSize" << sz;
    requestedSize = sz;
    constrained   = true;
    QWidget::setFixedSize(sz);
    QLayout::SizeConstraint sc = this->layout()->sizeConstraint();
    qInfo() << sc;
    emit sig_testSize();
}

void SystemView::setSize(QSize sz)
{
    if (Sys::config->limitViewSize)
    {
        QScreen * pri = QGuiApplication::primaryScreen();
        QSize size = pri->availableSize();
        //qDebug() << "Available size" << size;
        if (sz.width() > size.width())
        {
            sz.setWidth(size.width());
        }
        if (sz.height() > size.height())
        {
            sz.setHeight(size.height());
        }
    }

    if (constrained)
    {
        constrained = false;
        this->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
    }

    //qDebug() << "SystemView::setSize" << sz;
    requestedSize = sz;
    QWidget::resize(sz);

    emit sig_testSize();
}

void SystemView::testSize()
{
    static bool primed = false;

    if (!requestedSize.isValid())
        return;

    QSize sz = size();
    if (sz == requestedSize)
    {
        requestedSize = QSize();
        //qDebug() << "Size OK";
    }
    else if (primed == false)
    {
        qDebug() << "setting fixed size to" << requestedSize;
        primed = true;
        setFixedSize(requestedSize);
    }
    else
    {
        primed = false;
        //qDebug() << "size" << sz << "requested" << requestedSize;
        QString astring;
        QDebug ts(&astring);
        ts << "Requested size" << requestedSize << "Actual size" << sz;

        requestedSize = QSize();

        if (!Sys::config->disableResizeNotify)
        {
            emit sig_messageBox("View has been resized",astring);
        }
    }
}

void SystemView::flash(QColor color)
{
    // flash new color
    setBackgroundColor(color);
    QWidget::repaint();

    // wait
    QThread::msleep(350);

    // restore color
    auto viewControl = Sys::viewController;
    color = viewControl->getCanvas().getBkgdColor();
    setBackgroundColor(color);
    QWidget::repaint();
}

////////////////////////////////////////////////////////////////////////////////
///
/// ActiveLayers
///
////////////////////////////////////////////////////////////////////////////////

void ActiveLayers::add(LayerPtr layer)
{
    //qDebug().noquote() << "adding layer :" << layer->getLayerName();
    WeakLayerPtr wlp = layer;
    layers.push_back(wlp);
}

void  ActiveLayers::clear()
{
    layers.clear();
}

void ActiveLayers::unloadContent()
{
    for (const WeakLayerPtr & wlayer : std::as_const(layers))
    {
        auto layer = wlayer.lock();
        if (layer)
        {
            layer->unloadLayerContent();
        }
    }
}

bool ActiveLayers::contains(eViewType type)
{
    for (const WeakLayerPtr & wlayer : std::as_const(layers))
    {
        auto layer = wlayer.lock();
        if (layer && layer->viewType() == type)
        {
            return true;
        }
    }
    return false;
}

Layer * ActiveLayers::get(eViewType type)
{
    for (const WeakLayerPtr & wlayer: std::as_const(layers))
    {
        auto layer = wlayer.lock();
        if (layer && layer->viewType() == type)
        {
            return layer.get();
        }
    }
    return nullptr;
}

const  QVector<Layer*> ActiveLayers::get()
{
    QVector<Layer*> vec;
    for (const WeakLayerPtr & wlayer: std::as_const(layers))
    {
        auto layer = wlayer.lock();
        if (layer)
        {
            vec.push_back(layer.get());
        }
    }
    return vec;
}

void ActiveLayers::paint(QPainter & painter)
{
#if 0
    int  count = layers.size();
    qDebug() <<  "ActiveLayers::paint" << "count = " << count;

    for (Layer * layer : std::as_const(layers))
    {
        QString name =  layer->getLayerName();
        qDebug() <<  "ActiveLayers::paint" << "before sort" << name;
    }
#endif

    QVector<Layer *> layers2 = get();
    std::stable_sort(layers2.begin(),layers2.end(),Layer::sortByZlevelP);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (Layer * layer : std::as_const(layers2))
    {
        if (layer->isVisible())
        {
            //QString name =  layer->getLayerName();
            //qDebug() <<  "ActiveLayers::paint" << name;
            layer->forceLayerRecalc(false);
            painter.save();
            layer->paint(&painter);
            painter.restore();
        }
    }
}
