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

#include "base/canvas.h"
#include "base/border.h"
#include "base/scene.h"
#include "base/configuration.h"
#include "base/patterns.h"
#include "base/shortcuts.h"
#include "base/transparentwidget.h"
#include "panels/panel.h"
#include "style/Style.h"
#include "viewers/workspaceviewer.h"
#include "makers/figure_maker.h"
#include "base/tiledpatternmaker.h"

#ifdef __linux__
#define ALT_MODIFIER Qt::MetaModifier
#else
#define ALT_MODIFIER Qt::AltModifier
#endif

static QString sCanvasMode[]  = {
    E2STR(MODE_NONE),
    E2STR(MODE_LAYER),
    E2STR(MODE_ZLEVEL),
    E2STR(MODE_STEP),
    E2STR(MODE_SEPARATION),
    E2STR(MODE_ORIGIN),
    E2STR(MODE_OFFSET),     // row/col offsets
    E2STR(MODE_TRANSFORM),
    E2STR(MODE_CYCLE)
};

Canvas * Canvas::mpThis = nullptr;

Canvas * Canvas::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new Canvas();
    }
    return mpThis;
}

void Canvas::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

Canvas::Canvas()
{
    // some defaults
    selectedLayer   = 0;
    maxStep         = 0;
    stepsTaken      = 0;
    dragging        = false;
    scene           = nullptr;

    setMode(MODE_DEFAULT);

    // create timer before build
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Canvas::slot_nextStep);

    qRegisterMetaType<eCycleMode>();
}

Canvas::~Canvas()
{
    clearCanvas();
}

void Canvas::init()
{
    config    = Configuration::getInstance();
    workspace = Workspace::getInstance();
    viewer    = WorkspaceViewer::getInstance();

    connect(this, &Canvas::sig_viewWS, viewer,  &WorkspaceViewer::slot_viewWorkspace);
}

Scene * Canvas::swapScenes()
{
    if (!scene)
    {

        View * view = View::getInstance();
        sceneA = new Scene(view);
        sceneB = new Scene(view);
        scene = sceneA;
    }

    scene = (scene == sceneA) ? sceneB : sceneA;

    return scene;
}

void Canvas::update()
{
    if (scene)
        scene->invalidate();
}

void Canvas::invalidate()
{
    if (scene)
        scene->invalidate();
}

void Canvas::setCanvasSettings(CanvasSettings cset)
{
    scene->setBackgroundBrush(QBrush(cset.getBackgroundColor()));

    setSceneRect(cset.getRectF());

    BorderPtr bp = cset.getBorder();
    if (bp)
    {
        // when border cleared from scene, its children become disconnected
        // this reconnects them by re-parenting
        bp->reconnectChildren();
        // adds to the scene
        scene->addItem(bp.get());
    }

    settings = cset;     // makes local copy
}

void Canvas::addDesign(Design * d)
{
    int before = scene->items().size();
    QVector<PatternPtr>&  pats = d->getPatterns();
    for (int i=0; i < pats.count(); i++)
    {
        scene->addItem(pats[i].get());
    }
    qDebug() << "Canvas::addDesign before=" << before << "after="  << scene->items().size();

    invalidate();

    dump(true);
}

void Canvas::clearCanvas()
{
    if (!scene)
    {
        return;
    }

    // removes from scene but does not delete
    QList<QGraphicsItem*> ql = scene->items();
    for (int i=0; i < ql.count(); i++)
    {
        QGraphicsItem * g = ql[i];
        scene->removeItem(g);
    }
}

void Canvas::drainTheSwamp()
{
    dump(true);
    clearCanvas();
    viewer->clear();
    workspace->slot_clearWorkspace();
    emit sig_unload();
    dump(true);
}

void Canvas::setSceneRect(const QRectF & rect)
{
    QRectF old =  scene->sceneRect();
    if (rect != old)
    {
        scene->setSceneRect(rect);
    }
}

void Canvas::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    QRectF old  =  scene->sceneRect();
    QRectF rect(x,y,w,h);
    if (rect != old)
    {
        scene->setSceneRect(x,y,w,h);
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// Slots, etc
//
/////////////////////////////////////////////////////////////////////////////

void Canvas::stopTimer()
{
    timer->stop();
}

void Canvas::slot_startTimer()
{
    stepsTaken = 0;
    timer->start(1000/FPS);
}

void Canvas::setMaxStep(int max)
{
    maxStep = SECONDS(max);
}

void Canvas::slot_nextStep()
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

bool Canvas::step(int delta)
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

void Canvas::slot_setStep(int astep)
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

void Canvas::selectLayer(int layer)
{
    selectedLayer = layer;
}

void Canvas::zPlus()
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

void Canvas::zMinus()
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

void Canvas::showLayer()
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

void Canvas::hideLayer()
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

void Canvas::reposition(qreal x, qreal y)
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
    invalidate();
}

void Canvas::offset2(qreal x, qreal y)
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
    reposition(0,0);
}

void Canvas::origin(int x, int y)
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
    reposition(0,0);
}

void Canvas::slot_repositionAbs(qreal x, qreal y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    qDebug() << "slot_repositionAbs";
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->setXseparation(x);
            d->setYseparation(y);

            qDebug() << "origin=" <<d->getDesignInfo().getStartTile() << "xSep=" << d->getXseparation() << "ySep=" << d->getYseparation();

            d->repeat();
        }
    }
    invalidate();   // scene
}

void Canvas::slot_offsetAbs2(qreal x, qreal y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            d->setXoffset2(x);
            d->setYoffset2(y);
        }
    }
    reposition(0,0);
}

void Canvas::slot_originAbs(int x, int y)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    qDebug() << "slot_originAbs" << x << y;
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        if (d->isVisible())
        {
            QPointF pt(x,y);
            d->getDesignInfo().setStartTile(pt);
        }
    }
    reposition(0,0);
}

void Canvas::slot_toggleDesignVisibility(int design)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    if (design < designs.count())
    {
        DesignPtr d = designs[design];
        d->setVisible(!d->isVisible());
        invalidate();
    }
}


void Canvas::slot_png(QString file, int row, int col)
{
    QString name = config->examplesDir + file;
    QImage image(name);
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    item->setPos(col * 300, row* 200);
    scene->addItem(item);

    View * view = View::getInstance();
    view->show();
}

void Canvas::deltaScale(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];

        QSizeF sz =  d->getDesignInfo().getSizeF();
        qDebug() << "design: size=" << sz;
        if (delta > 0)
            sz *= 1.1;
        else
            sz *= 0.9;
        d->getDesignInfo().setSizeF(sz);
    }
    emit sig_forceUpdateStyles();
    invalidate();
}


void Canvas::deltaRotate(int delta, bool cw)
{
    Q_UNUSED(delta);
    Q_UNUSED(cw);
#if 0
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];

    }
    theScene.invalidate();
#endif
}

void Canvas::deltaMoveV(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        qreal top = d->getYoffset2();
        top -= delta;
        d->setYoffset2(top);
    }
    emit sig_forceUpdateStyles();
    invalidate();
}

void Canvas::deltaMoveH(int delta)
{
    QVector<DesignPtr> & designs = workspace->getDesigns();
    for (int i=0; i < designs.count(); i++)
    {
        DesignPtr d = designs[i];
        qreal left = d->getXoffset2();
        left -= delta;
        d->setXoffset2(left);
    }
    emit sig_forceUpdateStyles();
    invalidate();
}

void Canvas::dump(bool force)
{
    if (scene)
    {
        qDebug() << "Scene Items:" << scene->items().size();
        QList<QGraphicsItem*> ql = scene->items();
        for (int i=0; i < ql.count(); i++)
        {
            QGraphicsItem * g = ql[i];
            qDebug() << g;
        }
    }
    if (force)
        qDebug() << "*** Designs:" << Design::refs << "Patterns:" << Pattern::refs << "Layers:" << Layer::refs  << "Styles:" << Style::refs << "Maps:" << Map::refs << "Protos:" << Prototype::refs << "DELs:" << DesignElement::refs << "Figures:" << Figure::refs << "Edges:" << Edge::refs << "Vertices:"  << Vertex::refs;
}

void Canvas::dumpGraphicsInfo()
{
    qDebug() << "Scene: items=" << scene->items().size() << "bounding=" << scene->itemsBoundingRect() << "sceneRect" << scene->sceneRect();
}

void Canvas::setMode(eMode mode)
{
    _mode2 = mode;
    qDebug().noquote() << sCanvasMode[mode];
}

QString Canvas::getModeStr()
{
    return sCanvasMode[_mode2];
}

void Canvas::procKeyEvent(QKeyEvent *k)
{
    int key = k->key();

    int  multiplier = 1;
    if ((k->modifiers() & (Qt::SHIFT | Qt::CTRL)) == (Qt::SHIFT | Qt::CTRL))
        multiplier = 50;
    else if ((k->modifiers() & Qt::SHIFT) == Qt::SHIFT)
        multiplier = 10;

    ProcKey(k);

    switch (key)
    {
    case Qt::Key_Up:    deltaMoveV(1 * multiplier);  emit sig_deltaMoveV(-1 * multiplier); break;
    case Qt::Key_Down:  deltaMoveV(-1 * multiplier); emit sig_deltaMoveV(1 * multiplier);  break;
    case Qt::Key_Left:  deltaMoveH(-1 * multiplier); emit sig_deltaMoveH(1 * multiplier);  break;
    case Qt::Key_Right: deltaMoveH(1 * multiplier);  emit sig_deltaMoveH(-1 * multiplier); break;

    case '.':           deltaScale(-1 * multiplier); emit sig_deltaScale(1 * multiplier); break; // scale down
    case '>':           deltaScale(-1 * multiplier); emit sig_deltaScale(1 * multiplier); break; // scale down

    case ',':           deltaScale(1 * multiplier);  emit sig_deltaScale(-1 * multiplier); break;  // scale up
    case '<':           deltaScale(1 * multiplier);  emit sig_deltaScale(-1 * multiplier); break;  // scale up

    case '-':           deltaRotate(-1 * multiplier,true); emit sig_deltaRotate(-1 * multiplier); break; // rotate left
    case '_':           deltaRotate(-1 * multiplier,true); emit sig_deltaRotate(-1 * multiplier); break; // rotate left
    case '=':           deltaRotate(1 * multiplier,true);  emit sig_deltaRotate(1 * multiplier); break;  // rotate right
    case '+':           deltaRotate(1 * multiplier,true);  emit sig_deltaRotate(1 * multiplier); break;  // rotate right

    default: break;
    }
}

void Canvas::ProcKey(QKeyEvent *k)
{
    static int val = 0;

    int key = k->key();
    switch (key)
    {
    case 'A':  setMode(MODE_ORIGIN); break;
    case 'B':  setMode(MODE_OFFSET); break;
    case 'C':  emit sig_startCycle();  break;
    case 'D':  duplicate(); break;
    case 'F':  config->debugReplicate = !config->debugReplicate; emit sig_viewWS(); break;
    case 'G':  config->sceneGrid = !config->sceneGrid; invalidate(); break;
    case 'H':  config->hideCircles = !config->hideCircles; invalidate(); break;
    case 'I':  showLayer(); break;  // I=in
    case 'K':  config->debugMapEnable = !config->debugMapEnable; emit sig_figure_changed(); emit sig_viewWS(); break;
    case 'L':  setMode(MODE_LAYER); break;
    case 'M':  emit sig_raiseMenu(); break;
    case 'O':  hideLayer(); break; // o=out
    case 'P':  saveImage(); break;
    case 'Q':  if (getMode() == MODE_CYCLE) { emit sig_cyclerQuit(); setMode(MODE_DEFAULT); }
               else                         { QApplication::quit(); }
               break;
    case 'R':  if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER) { emit slot_startTimer(); setMode(MODE_DEFAULT); } break;
    case 'S':  if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER) { emit stopTimer(); setMode(MODE_STEP); }
               else { setMode(MODE_SEPARATION); }
               break;
    case 'T': config->boundingRects = !config->boundingRects;  invalidate(); break;
    case 'X': config->circleX = !config->circleX; emit sig_viewWS(); invalidate(); break;
    case 'Z': setMode(MODE_ZLEVEL); break;
    case Qt::Key_Up:    ProcKeyUp(k); break;
    case Qt::Key_Down:  ProcKeyDown(k); break;
    case Qt::Key_Left:  ProcKeyLeft(k); break;
    case Qt::Key_Right: ProcKeyRight(k); break;
    case Qt::Key_Return:
        if (getMode() == MODE_STEP)
        {
            slot_setStep(val);
        }
        val = 0;    // always
        break;
   case Qt::Key_Escape: setMode(MODE_NONE); break;
   case Qt::Key_F1:
        {
            QMessageBox  * box = new QMessageBox();
            box->setWindowTitle("Shortcuts");
            box->setText(Shortcuts::getCanvasShortcuts());
            box->setModal(false);
            box->show();
        }
        break;
    case Qt::Key_F2: setMode(MODE_TRANSFORM); break;
    case Qt::Key_F3: break;
    case Qt::Key_F4: dump(true); break;
    case Qt::Key_F5: drainTheSwamp(); break;
    case Qt::Key_Space: if (getMode() == MODE_CYCLE) emit sig_cyclerKey(key); break;
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
        if (getMode() == MODE_LAYER)
        {
            emit selectLayer(key-'0');
        }
        else if (getMode() == MODE_STEP)
        {
            val *= 10;
            val += (key - '0');
        }
        else
        {
            emit slot_toggleDesignVisibility(key-'0');
        }
        break;
    }
}

void Canvas::ProcKeyUp(QKeyEvent *k)
{
    // up arrow
    switch (getMode())
    {
    case MODE_ZLEVEL:
        emit zPlus();
        break;
    case MODE_STEP:
        emit step(1);
        break;
    case MODE_SEPARATION:
        reposition(0,-1);
        break;
    case MODE_OFFSET:
        offset2(0,-1);
        break;
    case MODE_ORIGIN:
        if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER)
            origin(0,-100);
        else
            origin(0,-1);
        break;
    default:
        break;
    }
}

void Canvas::ProcKeyDown(QKeyEvent *k)
{
    // down arrrow
    switch (getMode())
    {
        case MODE_ZLEVEL:
            emit zMinus();
            break;
        case MODE_STEP:
            emit step(-1);
            break;
        case MODE_SEPARATION:
            reposition(0,1);
            break;
        case MODE_OFFSET:
            offset2(0,1);
            break;
        case MODE_ORIGIN:
            if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER)
                origin(0,100);
            else
                origin(0,1);
            break;
        default:
            break;
    }
}

void Canvas::ProcKeyLeft(QKeyEvent *k)
{
    if (getMode() == MODE_SEPARATION)
        reposition(-1,0);
    else if (getMode() == MODE_OFFSET)
        offset2(-1,0);
    else if (getMode() == MODE_ORIGIN)
    {
        if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER)
            origin(-100,0);
        else
            origin(-1,0);
    }
}

void Canvas::ProcKeyRight(QKeyEvent *k)
{
    if (getMode() == MODE_SEPARATION)
        reposition(1,0);
    else if (getMode() == MODE_OFFSET)
        offset2(1,0);
    else if (getMode() == MODE_ORIGIN)
    {
        if ((k->modifiers() & ALT_MODIFIER) == ALT_MODIFIER)
            origin(100,0);
        else
            origin(1,0);
    }
}

void Canvas::slot_cycler_finished()
{
    qDebug() << "cycler finished";
    setMode(MODE_DEFAULT);
    emit sig_viewWS();
}

void Canvas::duplicate()
{
    View * view = View::getInstance();

    QPixmap pixmap(view->size());
    pixmap.fill((Qt::transparent));

    scene->paintBackground = false;
    QPainter painter(&pixmap);
    scene->render(&painter);
    scene->paintBackground = true;

    TransparentWidget * tw = new TransparentWidget();
    tw->resize(view->size());
    tw->setPixmap(pixmap);
    tw->show();
}

void Canvas::saveImage()
{
    QPixmap pixmap = View::getInstance()->grab();

    QString name = config->lastLoadedXML;
    Q_ASSERT(!name.contains(".xml"));

    QSettings s;
    QString path = s.value("picPath2",QCoreApplication::applicationDirPath()).toString();
    qDebug() << "path=" << path;

    QString nameList;
    if (path.contains(".png"))
    {
        nameList = "PNG (*.png);;BMP Files (*.bmp);;JPG (*.jpg)";
    }
    else if (path.contains(".jpg"))
    {
        nameList = "JPG (*.jpg);;PNG (*.png);;BMP Files (*.bmp)";
    }
    else
    {
        nameList = "BMP Files (*.bmp);;JPG (*.jpg);;PNG (*.png)";
    }

    QFileDialog dlg(View::getInstance(), "Save image", path, nameList);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.selectFile(name);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    QStringList fileList = dlg.selectedFiles();
    if (fileList.isEmpty())
    {
        qDebug() << "No file selected";
        return;
    }
    QString file = fileList.at(0);
    QString flt  = dlg.selectedNameFilter();
    QString ext;
    if (flt.contains(".png"))
    {
        ext = ".png";
    }
    else if (flt.contains(".jpg"))
    {
        ext = ".jpg";
    }
    else
    {
        ext = ".bmp";
    }
    if (!file.contains(ext))
    {
        file = file + ext;
    }

    qDebug() << "saving:" << file;
    bool rv = pixmap.save(file);

    if (rv)
    {
        qDebug() << file << "saved OK";
        QFileInfo fileInfo(file);
        path = fileInfo.path();
        s.setValue("picPath2",path);
    }
    else
    {
        qDebug() << file << "save ERROR";
    }
}

void Canvas::saveBMP(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    QPixmap pixmap = View::getInstance()->grab();

    QString subdir;
    switch (config->repeatMode)
    {
    case REPEAT_SINGLE:
        subdir = "single/";
        break;
    case REPEAT_PACK:
        subdir = "pack/";
        break;
    case REPEAT_DEFINED:
        subdir = "defined/";
        break;
    }

    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("yyyy-MM-dd");

    QString path = config->rootImageDir;
    if (config->viewerType == VIEW_TILING)
        path += "tilings/" + subdir + date;
    else
        path += subdir + date;

    QDir adir(path);
    if (!adir.exists())
    {
        if (!adir.mkpath(path))
        {
            qFatal("could not make path");
        }
    }
    QString file = path + "/" + name + ".bmp";
    qDebug() << "saving" << file;

    //QFileInfo fileInfo(file);
    //QString path = fileInfo.filePath();
    //s.setValue("picPath",path);

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}
