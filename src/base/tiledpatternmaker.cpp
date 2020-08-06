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

#include "base/tiledpatternmaker.h"
#include "base/canvas.h"
#include "base/cycler.h"
#include "base/fileservices.h"
#include "base/mosaic.h"
#include "base/tilingmanager.h"
#include "base/transparentwidget.h"
#include "designs/designs.h"
#include "style/style.h"
#include "panels/panel.h"
#include "panels/page_views.h"
#include "panels/splitscreen.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "viewers/workspace_viewer.h"

TiledPatternMaker::TiledPatternMaker() : QObject()
{
    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::startEverything, Qt::QueuedConnection);
    emit sig_start();
}

void TiledPatternMaker::startEverything()
{
    // instantiate everything
    config            = Configuration::getInstance();
    bool oldPanelLock = config->lockView;
    config->lockView  = true;    // disables view switching during init

    view      = View::getInstance();
    canvas    = Canvas::getInstance();
    workspace = Workspace::getInstance();
    wsViewer  = WorkspaceViewer::getInstance();

    // connect singleton classes
    view->init();
    canvas->init(this);
    workspace->init();
    wsViewer->init();

    TilingManager::getInstance();

    mapEditor   = MapEditor::getInstance();
    tilingMaker = TilingMaker::getInstance();

    config->availableDesigns.insert(DESIGN_5,make_shared<Design5>(DESIGN_5,"Pattern 1 (created)"));
    config->availableDesigns.insert(DESIGN_6,make_shared<Design6>(DESIGN_6,"Pattern 2 (re-ceated)"));
    config->availableDesigns.insert(DESIGN_7,make_shared<Design7>(DESIGN_7,"Pattern 3 (re-ceated)"));
    config->availableDesigns.insert(DESIGN_8,make_shared<Design8>(DESIGN_8,"The Hu Symbol"));
    config->availableDesigns.insert(DESIGN_9,make_shared<Design9>(DESIGN_9,"A packing of Hu symbols"));
    config->availableDesigns.insert(DESIGN_HU_INSERT,make_shared<DesignHuInsert>(DESIGN_HU_INSERT,"Hu Insert"));
    config->availableDesigns.insert(DESIGN_10,make_shared<DesignHuPacked>(DESIGN_10,"Fully packed Hu symbols"));
    config->availableDesigns.insert(DESIGN_11,make_shared<Design11>(DESIGN_11,"Attempt at woven Hu (unsuccessful)"));
    config->availableDesigns.insert(DESIGN_12,make_shared<Design12>(DESIGN_12,"Enneagram"));
    config->availableDesigns.insert(DESIGN_13,make_shared<Design13>(DESIGN_13,"Alhambra 1"));
    config->availableDesigns.insert(DESIGN_14,make_shared<Design14>(DESIGN_14,"Alhambra 2"));
    config->availableDesigns.insert(DESIGN_16,make_shared<Design16>(DESIGN_16,"Broug: Capella Palatina, Palermo"));
    config->availableDesigns.insert(DESIGN_17,make_shared<Design17>(DESIGN_17,"Broug: The Koran of Rashid al-Din"));
    config->availableDesigns.insert(DESIGN_18,make_shared<Design18>(DESIGN_18,"Broug: Mustansiriya Madrasa"));
    config->availableDesigns.insert(DESIGN_19,make_shared<Design19>(DESIGN_19,"NOT Broug: Esrefogulu Mosque"));
    config->availableDesigns.insert(DESIGN_KUMIKO1,make_shared<DesignKumiko1>(DESIGN_KUMIKO1,"Kumiko 1"));
    config->availableDesigns.insert(DESIGN_KUMIKO2,make_shared<DesignKumiko2>(DESIGN_KUMIKO2,"Kumiko 2"));

    qRegisterMetaType<MapPtr>("MapPtr");
    qRegisterMetaType<PolyPtr>("PolyPtr");

    connect(this, &TiledPatternMaker::sig_viewWS,         wsViewer, &WorkspaceViewer::slot_viewWorkspace);

    connect(canvas, &Canvas::sig_forceUpdateStyles,       this, &TiledPatternMaker::slot_forceUpdateStyles);
    connect(canvas, &Canvas::sig_raiseMenu,               this, &TiledPatternMaker::slot_raiseMenu);

    // create cycler
    QThread * thread = new QThread();
    thread->start();

    cyclerWindow = nullptr;
    cycler = Cycler::getInstance();
    cycler->init(thread);

    connect(this,   &TiledPatternMaker::sig_ready,         cycler,  &Cycler::slot_ready);
    connect(this,   &TiledPatternMaker::sig_loadedTiling,  tilingMaker, &TilingMaker::slot_setTiling);

    connect(cycler,    &Cycler::sig_loadXML,        this,  &TiledPatternMaker::slot_loadXMLSimple);
    connect(cycler,    &Cycler::sig_loadTiling,     this,  &TiledPatternMaker::slot_loadTilingSimple);
    connect(cycler,    &Cycler::sig_finished,       this,  &TiledPatternMaker::slot_cyclerFinished);
    connect(cycler,    &Cycler::sig_compare,        this,  &TiledPatternMaker::slot_compareImages);
    connect(cycler,    &Cycler::sig_viewImage,      this,  &TiledPatternMaker::slot_view_image);
    connect(cycler,    &Cycler::sig_show_png,     canvas,  &Canvas::slot_show_png);

    connect(canvas,   &Canvas::sig_cyclerStart,  cycler,  &Cycler::slot_startCycle, Qt::QueuedConnection);
    connect(canvas,   &Canvas::sig_cyclerKey,    cycler,  &Cycler::slot_psuedoKey);
    connect(canvas,   &Canvas::sig_cyclerQuit,   cycler,  &Cycler::slot_stopCycle);

    // pop-up control panel
    controlPanel = ControlPanel::getInstance();
    controlPanel->init(this);
    controlPanel->show();
    controlPanel->raise();
    controlPanel->activateWindow();

    // split-screen
    if (config->splitScreen)
    {
        slot_splitScreen(true);
    }

    // get started - kick off
    if (config->autoLoadStyles && !config->lastLoadedXML.isEmpty())
    {
        if (!config->lastLoadedXML.isEmpty())
        {
            slot_loadXML(config->lastLoadedXML);
        }
    }
    else if (config->autoLoadTiling)
    {
        if (!config->lastLoadedTileName.isEmpty())
        {
            slot_loadTiling(config->lastLoadedTileName);
        }
    }
    else if (config->autoLoadDesigns)
    {
        slot_loadDesign(config->lastLoadedDesignId);
    }

    config->lockView = oldPanelLock;    // restore
    panel_page * pp = controlPanel->getCurrentPage();
    page_views * pv = dynamic_cast<page_views*>(pp);
    if (pv)
    {
        pv->onEnter();  // refreshes lock status
    }

    emit sig_viewWS();
}

TiledPatternMaker::~TiledPatternMaker()
{
    View::releaseInstance();
    Canvas::releaseInstance();
    workspace->releaseInstance();
    controlPanel->closePages();
    controlPanel->close();
    controlPanel->releaseInstance();
    config->save();
    config->releaseInstance();      // performed last
}

void TiledPatternMaker::slot_loadDesign(eDesign design)
{
    DesignPtr d = config->availableDesigns.value(design);
    if (!d)
        return;
    d->init();
    slot_buildDesign(design);
}

void TiledPatternMaker::slot_buildDesign(eDesign design)
{
    qDebug().noquote() << "TiledPatternMaker::slot_buildDesign" << Design::getDesignName(design);

    view->dump(true);
    workspace->clearDesigns();
    view->dump(true);

    DesignPtr d = config->availableDesigns.value(design);
    //d->init();
    d->build();
    d->repeat();
    workspace->addDesign(d);

    config->lastLoadedDesignId = design;

    emit sig_viewWS();
    emit sig_loadedDesign(design);
}

void TiledPatternMaker::slot_loadXML(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_loadXML() <" << name << ">";
    view->setWindowTitle(QString("Loading: %1").arg(name));
    QCoreApplication::processEvents();
    view->dump(true);
    bool rv = workspace->loadMosaic(name);
    if (rv)
    {
        emit sig_viewWS();
        emit sig_ready();
        emit sig_loadedXML(name);
#if 0
        // removed 26APR2020
        QString tileName = FileServices::getTileNameFromDesignName(name);
        emit sig_loadedTiling(tileName);
#endif
    }
}

void TiledPatternMaker::slot_loadXMLSimple(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_loadXML() <" << name << ">";
    view->setWindowTitle(QString("Loading: %1").arg(name));
    bool rv = workspace->loadMosaic(name);
    if (rv)
    {
        emit sig_viewWS();
        emit sig_ready();
    }
}

void TiledPatternMaker::slot_saveXML(QString filename)
{
    qDebug() << "TiledPatternMaker::slot_saveXML()";
    QString savedFile;
    bool rv = workspace->saveMosaic(filename,savedFile,false);
    if (rv)
    {
        MapEditor::getInstance()->keepStash(savedFile);
        config->lastLoadedXML      = savedFile;
        config->currentlyLoadedXML = savedFile;
        emit sig_newXML();
        emit sig_loadedXML(savedFile);
    }
}

void TiledPatternMaker::slot_loadTiling(QString name)
{
    view->setWindowTitle(QString("Loading tiling: %1").arg(name));
    if (workspace->loadTiling(name))
    {
        config->lastLoadedTileName = name;
        emit sig_ready();
        emit sig_loadedTiling(name);
        emit sig_viewWS();
    }
    else
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Tile <%1> NOT FOUND").arg(name));
        box.exec();
    }
}

void TiledPatternMaker::slot_loadTilingSimple(QString name)
{
    view->setWindowTitle(QString("Loading tiling: %1").arg(name));
    bool rv = workspace->loadTiling(name);
    if (rv)
    {
        config->lastLoadedTileName = name;
        emit sig_viewWS();
        emit sig_ready();
    }
}

void TiledPatternMaker::slot_forceUpdateStyles()
{
    forceUpdateStyles();
}

void TiledPatternMaker::slot_render()
{
    resetProtos();
    resetStyles();

    emit sig_viewWS();
}

void TiledPatternMaker::resetProtos()
{
    // reset prototypes
    MosaicPtr mosaic = workspace->getMosaic();
    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        PrototypePtr pp = style->getPrototype();
        if (pp)
        {
            pp->createProtoMap();
        }
    }
}

void TiledPatternMaker::resetStyles()
{
    // reset styles
    MosaicPtr mosaic = workspace->getMosaic();
    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->resetStyleRepresentation();
    }
}

void TiledPatternMaker::forceUpdateStyles()
{
    MosaicPtr mosaic  = workspace->getMosaic();
    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        style->forceUpdateLayer();
    }
}

void TiledPatternMaker::slot_raiseMenu()
{
    controlPanel->setWindowState((controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    controlPanel->raise();
    controlPanel->activateWindow();
}

void TiledPatternMaker::slot_bringToPrimaryScreen()
{
    QScreen * primary = qApp->primaryScreen();

    QWindow * wh = view->windowHandle();
    if (wh)
    {
        wh->setScreen(primary);
        view->move(primary->geometry().center() - view->rect().center());
        view->setWindowState( (view->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        view->raise();  // for MacOS
        view->activateWindow();  // for windows
        controlPanel->move(primary->geometry().center() - view->rect().center());
        controlPanel->setWindowState( (controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        controlPanel->raise();  // for MacOS
        controlPanel->activateWindow();  // for windows
    }
}

void TiledPatternMaker::slot_splitScreen(bool checked)
{
    static SplitScreen  * splitter = nullptr;

    QSettings s;

    config->splitScreen = checked;

    if (config->splitScreen)
    {
        // split the screen

        if (!splitter)
        {
            splitter = new SplitScreen();
        }

        s.setValue("viewPos",view->pos());
        s.setValue("panelPos", controlPanel->pos());

        QScreen * sc = qApp->screenAt(controlPanel->pos());
        splitter->addWidgets(controlPanel,view);

        QRect  rec = sc->geometry();

        splitter->setFixedSize(rec.width(),rec.height());
        splitter->setMinimumSize(rec.width(),rec.height());
        splitter->setMaximumSize(rec.width(),rec.height());
        splitter->move(rec.topLeft());
        splitter->show();
    }
    else
    {
        // unsplit the screen
        controlPanel->setParent(nullptr);
        view->setParent(nullptr);

        controlPanel->show();
        controlPanel->move(s.value("panelPos").toPoint());
        controlPanel->adjustSize();

        view->show();
        view->move(s.value("viewPos").toPoint());

        if (splitter)
        {
            delete splitter;
            splitter = nullptr;
        }
    }
}

void TiledPatternMaker::slot_cyclerFinished()
{
    QMessageBox box(view);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}


void TiledPatternMaker::SplatCompareResult(QPixmap & pixmap, QString title)
{
    AQLabel * l = new AQLabel();
    l->resize(pixmap.size());
    l->setPixmap(pixmap);
    l->setWindowTitle(title);

    connect(l,  &AQLabel::sig_takeNext,     cycler, &Cycler::slot_ready, Qt::UniqueConnection);
    connect(l,  &AQLabel::sig_cyclerQuit,   cycler, &Cycler::slot_stopCycle,Qt::UniqueConnection);
    connect(l,  &AQLabel::sig_view_images,  cycler, &Cycler::slot_view_images,Qt::UniqueConnection);

    l->show();
}

void TiledPatternMaker::SplatShowImage(QImage & image, QString title)
{
    cyclerWindow = new AQLabel;

    QPixmap pixmap;
    pixmap.convertFromImage(image);

    cyclerWindow->resize(pixmap.size());
    cyclerWindow->setPixmap(pixmap);
    cyclerWindow->setWindowTitle(title);

    connect(cyclerWindow,  &AQLabel::sig_takeNext,     cycler, &Cycler::slot_ready, Qt::UniqueConnection);
    connect(cyclerWindow,  &AQLabel::sig_cyclerQuit,   cycler, &Cycler::slot_stopCycle,Qt::UniqueConnection);
    connect(cyclerWindow,  &AQLabel::sig_view_images,  cycler, &Cycler::slot_view_images,Qt::UniqueConnection);

    cyclerWindow->show();
}

void TiledPatternMaker::SplatCompareResultTransparent(QPixmap &pixmap, QString title)
{
    TransparentWidget * l = new TransparentWidget;
    l->resize(pixmap.size());
    l->setPixmap(pixmap);
    l->setWindowTitle(title);

    connect(l,  &AQLabel::sig_takeNext,     cycler, &Cycler::slot_ready, Qt::UniqueConnection);
    connect(l,  &TransparentWidget::sig_cyclerQuit,   cycler, &Cycler::slot_stopCycle,Qt::UniqueConnection);
    connect(l,  &TransparentWidget::sig_view_images,  cycler, &Cycler::slot_view_images,Qt::UniqueConnection);

    l->show();
}

void TiledPatternMaker::slot_compareImagesReplace(QString fileLeft, QString fileRight)
{
    if (cyclerWindow)
    {
        delete cyclerWindow;
    }
    slot_compareImages(fileLeft,fileRight);
}


void TiledPatternMaker::slot_compareImages(QString fileLeft, QString fileRight)
{
    //qDebug() << "fileLeft " << fileLeft;
    //qDebug() << "fileRight" << fileRight;

    static bool showFirst   = false;

    bool autoMode     = config->autoCycle;
    bool stopIfDiff   = config->stopIfDiff;
    bool differences  = config->display_differences;
    bool pingpong     = config->compare_ping_pong;

    emit sig_image0(fileLeft);      // sets page_debug status
    emit sig_image1(fileRight);     // sets page_debug status

    //QCoreApplication::processEvents();

    if (fileRight.isEmpty())
    {
        qDebug() << "different (no match)" << fileLeft;
        QString str = "No matching image found ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,fileLeft);
        if (!autoMode || stopIfDiff)
        {
            SplatCompareResult(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    QString pathLeft  = config->compareDir0 + "/" + fileLeft;
    QString pathRight = config->compareDir1 + "/" + fileRight;
    QImage left(pathLeft);
    QImage right(pathRight);

    if (left == right)
    {
        qDebug() << "same     " << pathLeft;
        QString str = "Images are the same ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,pathLeft,pathRight);
        if (!autoMode)
        {
            SplatCompareResult(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    // files are different

    qDebug() << "different" << pathLeft;
    QString str = "Images are different";
    emit sig_compareResult(str);    // sets page_debug status

    if (autoMode && !stopIfDiff)
    {
        emit sig_ready();
        return;
    }

    if  (differences)
    {
        // display differences
        if (left.size() != right.size())
        {
            QString str1 = QString("%1 = %2x%3").arg(pathLeft).arg(left.width()).arg(left.height());
            QString str2 = QString("%1 = %2x%3").arg(pathRight).arg(right.width()).arg(right.height());
            QPixmap  pm = makeTextPixmap("Images different sizes:",str1,str2);
            QString str = "Images are different sizes";
            emit sig_compareResult(str);    // sets page_debug status
            SplatCompareResult(pm,str);
            return;
        }

        QImage result(left.size(),left.format());

        int w = left.width();
        int h = left.height();

        for(int i=0; i<h; i++)
        {
            QRgb *rgbLeft   = reinterpret_cast<QRgb*>(left.scanLine(i));
            QRgb *rgbRigth  = reinterpret_cast<QRgb*>(right.scanLine(i));
            QRgb *rgbResult = reinterpret_cast<QRgb*>(result.scanLine(i));
            for( int j=0; j<w; j++)
            {
                rgbResult[j] = rgbLeft[j]-rgbRigth[j];
            }
        }

        QPixmap pixmap;
        pixmap.convertFromImage(result);

        if (config->compare_transparent)
        {
            SplatCompareResultTransparent(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
        }
        else
        {
            SplatCompareResult(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
        }
    }

    if (pingpong)
    {
        QPixmap pixmap;
        showFirst = !showFirst;
        if (showFirst)
        {
            SplatShowImage(left,pathLeft);
        }
        else
        {
            SplatShowImage(right,pathRight);
        }
    }
}

QPixmap TiledPatternMaker::makeTextPixmap(QString txt,QString txt2,QString txt3)
{
    QString filename = "sample.bmp";
    QString format = "bmp";

    QPixmap pixmap(1200,500);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);

    QFont serifFont("Times", 18, QFont::Normal);
    painter.setFont(serifFont);
    painter.setPen(Qt::black);

    painter.drawText(20,100,txt);
    painter.drawText(20,150,txt2);
    painter.drawText(20,200,txt3);

    return pixmap;
}

void TiledPatternMaker::slot_view_image(QString file)
{
    QPixmap pixmap(file);
    Q_ASSERT(!pixmap.isNull());

    AQLabel * l = new AQLabel;
    l->resize(pixmap.size());
    l->setPixmap(pixmap);
    l->setWindowTitle(file);
    l->show();
}

void TiledPatternMaker::drainTheSwamp()
{
    view->dump(true);
    emit sig_unload();
    view->dump(true);
    workspace->slot_clearWorkspace();
    view->dump(true);
    view->clearView();
    view->dump(true);
    wsViewer->clear();
    view->dump(true);
    view->update();
}
