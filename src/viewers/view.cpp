#include <QApplication>
#include <QMessageBox>

#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "legacy/design_maker.h"
#include "motifs/motif.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/cycler.h"
#include "misc/defaults.h"
#include "misc/runguard.h"
#include "misc/shortcuts.h"
#include "mosaic/mosaic.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tile.h"
#include "tiledpatternmaker.h"
#include "widgets/transparentwidget.h"
#include "widgets/mouse_mode_widget.h"

extern class TiledPatternMaker * theApp;
extern RunGuard * guard;

View::View(ViewControl *parent)
{
    config       = Configuration::getInstance();
    this->parent = parent;
    isShown      = false;
    canPaint     = true;
    dragging     = false;
    iMouseMode   = MOUSE_MODE_NONE;
    keyboardMode = KBD_MODE_XFORM_VIEW;

    setFocusPolicy(Qt::ClickFocus);
   	setMouseTracking(true);
}

View::~View()
{
//    qDebug() << "View destructor";
}

void View::init()
{
    tilingMaker = TilingMaker::getInstance();
    designMaker = DesignMaker::getInstance();
    panel       = ControlPanel::getInstance();

    Cycler * cycler = Cycler::getInstance();
    connect(this,     &View::sig_cyclerQuit,   cycler,  &Cycler::slot_stopCycle);
    connect(this,     &View::sig_cyclerKey,    cycler,  &Cycler::slot_psuedoKey);

    resize(QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

void View::addLayer(LayerPtr layer)
{
    activeLayers.push_back(layer.get());
}

void View::addLayer(Layer * layer)
{
    activeLayers.push_back(layer);
}

void View::addTopLayer(LayerPtr layer)
{
    activeLayers.push_front(layer.get());
}

void View::unloadView()
{
    canPaint = false;
    clearLayers();
    clearLayout();
    canPaint = true;
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

void  View::paintEnable(bool enable)
{
    setAttribute(Qt::WA_NoSystemBackground,!enable);
    setAutoFillBackground(enable);
    canPaint = enable;
}

void View::resize(QSize sz)
{
    qDebug() << "View set size :" << sz;
    QWidget::resize(sz);
}

void View::duplicateView()
{
    QPixmap pixmap(size());
    pixmap.fill((Qt::transparent));

    QPainter painter(&pixmap);
    render(&painter);

    QImage img =  pixmap.toImage();
    QPixmap pixmap2 = TiledPatternMaker::createTransparentPixmap(img);

    TransparentWidget * tw = new TransparentWidget("Duplicate");
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
        config->showCenterMouse = true;
    else
        config->showCenterMouse = false;

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

void View::dumpRefs()
{
    qDebug() << "Mosaics:"  << Mosaic::refs
             << "Styles:"   << Style::refs
             << "Protos:"   << Prototype::refs
             << "Maps:"     << Map::refs
             << "DCELs"     << DCEL::refs
             << "faces"     << Face::refs
             << "DELs:"     << DesignElement::refs
             << "Motifs:"   << Motif::refs
             << "Tilings:"  << Tiling::refs
             << "Tiles:"    << Tile::refs
             << "Edges:"    << Edge::refs
             << "Vertices:" << Vertex::refs;
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

    case '.':           emit sig_deltaScale(delta);  update(); return true;  // scale down
    case '>':           emit sig_deltaScale(delta);  update(); return true;  // scale down
    case ',':           emit sig_deltaScale(-delta); update(); return true;  // scale up
    case '<':           emit sig_deltaScale(-delta); update(); return true;  // scale up

    case '-':           emit sig_deltaRotate(-delta); update(); return true; // rotate left
    case '_':           emit sig_deltaRotate(-delta); update(); return true; // rotate left
    case '=':           emit sig_deltaRotate( delta); update(); return true; // rotate right
    case '+':           emit sig_deltaRotate( delta); update(); return true; // rotate right

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
    case 'E':  parent->slot_refreshView(); break;    // just for debug
    case 'F':  break;
    case 'G':  config->showGrid = !config->showGrid; parent->slot_refreshView(); break;
    case 'H':  config->hideCircles = !config->hideCircles; config->showCenterDebug = !config->showCenterDebug; update(); break;
    case 'I':  designMaker->designLayerShow(); break;  // I=in
    case 'J':  emit sig_saveMenu(); break;
    case 'K':  config->debugMapEnable = !config->debugMapEnable; emit sig_rebuildMotif(); break;
    case 'L':  setKbdMode(KBD_MODE_DES_LAYER_SELECT); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'N':  theApp->slot_bringToPrimaryScreen(); break;
    case 'O':  designMaker->designLayerHide(); break; // o=out
    case 'P':  emit sig_saveImage(); break;
    case 'Q':  if (Cycler::getInstance()->getMode() != CYCLE_NONE)
                    emit sig_cyclerQuit();      // does not work for local cycle
               else
                    QApplication::quit();
               break;
    case 'R':  config->dontReplicate = !config->dontReplicate; emit sig_rebuildMotif(); break;
    case 'S':  setKbdMode(KBD_MODE_DES_SEPARATION); break;
    case 'T':  setKbdMode(KBD_MODE_XFORM_TILING); break;
    case 'U':  setKbdMode(KBD_MODE_XFORM_BKGD); break;
    case 'V':  setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case 'W':  setKbdMode(KBD_MODE_XFORM_UNIQUE_TILE);break;
    case 'X':  config->circleX = !config->circleX; parent->slot_refreshView(); update(); break;
    case 'Y':  emit sig_saveSVG(); break;
    case 'Z':  setKbdMode(KBD_MODE_DES_ZLEVEL); break;

    case Qt::Key_Return: if (getKbdMode(KBD_MODE_DES_STEP)) designMaker->setStep(val); val = 0; break; // always val=0
    case Qt::Key_Escape: setKbdMode(KBD_MODE_XFORM_VIEW); break;
    case Qt::Key_F1:
    {
        QMessageBox  * box = new QMessageBox();
        box->setWindowTitle("Shortcuts");
        if (parent->isEnabled(VIEW_DESIGN))
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
    case Qt::Key_F4: dumpRefs(); break;
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
    if (!isShown)
    {
        // first time only
        QSettings s;
        QPoint pt = s.value(QString("viewPos/%1").arg(config->appInstance)).toPoint();
        setGeometry(pt.x(),pt.y(),DEFAULT_WIDTH,DEFAULT_HEIGHT);
        qDebug() << "View::showEvent moving to:" << pt;
        move(pt);
        _geometry = geometry();
        isShown = true;
    }
    QWidget::showEvent(event);
}

void View::closeEvent(QCloseEvent *event)
{
    qDebug() << "View::closeEvent";
    if (!config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue((QString("viewPos/%1").arg(config->appInstance)),pt);
        qDebug() << "View::closeEvent pos" << pt;
    }

    guard->release();
    QWidget::closeEvent(event);
    qApp->quit();
}

void View::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!canPaint)
    {
        qDebug() << "View::paintEvent - discarded";
        return;
    }

    //qDebug() << "View::paintEvent: layers=" << activeLayers.size() << "viewRect" << rect();

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

        qInfo().noquote() << "Load operation for" << loadUnit.name << "took" << str << "seconds";
        loadUnit.loadTimer.invalidate();
        loadUnit.name.clear();
    }

    //qDebug() << "View::paintEvent: end";
}

#if 0
// every resize event has a move event afterwards
// but every move event does not necessarily have a resize event
void View::moveEvent(QMoveEvent *event)
{
    qDebug().noquote() << "View::move from =" << event->oldPos() << "to =" << event->pos();
    QWidget::moveEvent(event);
}
#endif

void View::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    auto mostRecent = parent->getMostRecent();

    QSize oldSize   = viewSettings.getCropSize(mostRecent);
    QSize newSize   = size();

    emit sig_viewSizeChanged(oldSize,newSize);      // for splitscreen and outer border (if any)

    if (oldSize == newSize)
    {
        return;
    }

    QSize deltaSize = newSize - oldSize;
    //qDebug() << "View::resizeEvent: old" << oldSize << "new" << newSize << "delta-size" << deltaSize;

    QRect rect = geometry();
    int deltaX = _geometry.x() - rect.x();
    int deltaY = _geometry.y() - rect.y();
    _geometry  = rect;

    switch(mostRecent)
    {
    case VIEW_MOTIF_MAKER:
    case VIEW_DESIGN:
        viewSettings.setDeltaSize(mostRecent,deltaSize);
        break;

    case VIEW_TILING_MAKER:
    case VIEW_TILING:
    case VIEW_MOSAIC:
    case VIEW_PROTOTYPE:
    case VIEW_MAP_EDITOR:
        viewSettings.setCommonDeltaSizes(deltaSize);
        break;

    case VIEW_MEASURE:
    case VIEW_CENTER:
    case VIEW_BKGD_IMG:
    case VIEW_GRID:
    case VIEW_BORDER:
    case VIEW_CROP:
    case VIEW_IMAGE:
        // these are not primary so cant be most recent
        break;
    }

    if (!config->scaleToView)
    {
        if (deltaX) emit sig_deltaMoveX(deltaX);
        if (deltaY) emit sig_deltaMoveY(deltaY);
    }
}

void View::keyPressEvent( QKeyEvent *k )
{
    if (tilingMaker->procKeyEvent(k))        // tiling maker
    {
        return;
    }
    else if (MapEditor::getInstance()->procKeyEvent(k))    // map editor
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
    if (getMouseMode(MOUSE_MODE_CENTER) && event->button() == Qt::LeftButton)
    {
        emit sig_setCenter(event->position());
        setMouseMode(MOUSE_MODE_CENTER,false);
        parent->slot_refreshView();
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
