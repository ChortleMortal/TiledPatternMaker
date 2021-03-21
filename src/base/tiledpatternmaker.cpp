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
#include "base/fileservices.h"
#include "base/mosaic.h"
#include "base/mosaic_manager.h"
#include "base/transparentwidget.h"
#include "designs/design_maker.h"
#include "designs/designs.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"
#include "panels/splitscreen.h"
#include "panels/view_panel.h"
#include "style/style.h"
#include "tile/tiling_manager.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

TiledPatternMaker::TiledPatternMaker() : QObject()
{
    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::startEverything, Qt::QueuedConnection);
}

void TiledPatternMaker::startEverything()
{
    // instantiate everything
    config            = Configuration::getInstance();

    bool oldPanelLock = config->lockView;
    config->lockView  = true;    // disables view switching during init

    vcontrol        = ViewControl::getInstance();
    view            = View::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    decorationMaker = DecorationMaker::getInstance();
    mapEditor       = MapEditor::getInstance();

    vcontrol->init();
    view->init();

    config->availableDesigns.insert(DESIGN_5,make_shared<Design5>(DESIGN_5,"Pattern 1 (created)"));
    config->availableDesigns.insert(DESIGN_6,make_shared<Design6>(DESIGN_6,"Pattern 2 (re-ceated)"));
    config->availableDesigns.insert(DESIGN_7,make_shared<Design7>(DESIGN_7,"Pattern 3 (re-ceated)"));
    config->availableDesigns.insert(DESIGN_8,make_shared<Design8>(DESIGN_8,"The Hu Symbol"));
    config->availableDesigns.insert(DESIGN_9,make_shared<Design9>(DESIGN_9,"A packing of Hu symbols"));
    config->availableDesigns.insert(DESIGN_HU_INSERT,make_shared<DesignHuInsert>(DESIGN_HU_INSERT,"Hu Insert"));
    config->availableDesigns.insert(DESIGN_10,make_shared<DesignHuPacked>(DESIGN_10,"Fully packed Hu symbols"));
    config->availableDesigns.insert(DESIGN_11,make_shared<Design11>(DESIGN_11,"Woven Hu (unsuccessful)"));
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
    qRegisterMetaType<FigurePtr>("FigurePtr");

    ViewControl * vcontrol = ViewControl::getInstance();
    connect(this, &TiledPatternMaker::sig_refreshView, vcontrol, &ViewControl::slot_refreshView);

    connect(view, &View::sig_raiseMenu, this, &TiledPatternMaker::slot_raiseMenu);

    // create cycler
    QThread * thread = new QThread();
    thread->start();

    cycler = Cycler::getInstance();
    cycler->init(thread);

    connect(this,  &TiledPatternMaker::sig_ready,        cycler, &Cycler::slot_ready);
    connect(this,  &TiledPatternMaker::sig_takeNext,     cycler, &Cycler::slot_ready);
    connect(this,  &TiledPatternMaker::sig_cyclerQuit,   cycler, &Cycler::slot_stopCycle);

    connect(cycler,    &Cycler::sig_cycleLoadMosaic,this,  &TiledPatternMaker::slot_cycleLoadMosaic);
    connect(cycler,    &Cycler::sig_cycleLoadTiling,this,  &TiledPatternMaker::slot_cyclerLoadTiling);
    connect(cycler,    &Cycler::sig_finished,       this,  &TiledPatternMaker::slot_cyclerFinished);
    connect(cycler,    &Cycler::sig_compare,        this,  &TiledPatternMaker::slot_compareImages);
    connect(cycler,    &Cycler::sig_show_png,       this,  &TiledPatternMaker::slot_show_png);

    // pop-up control panel
    controlPanel = ControlPanel::getInstance();
    controlPanel->init(this);
    controlPanel->show();
    controlPanel->raise();
    controlPanel->activateWindow();

    decorationMaker->init();
    motifMaker->init();
    tilingMaker->init();

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
            slot_loadMosaic(config->lastLoadedXML);
        }
    }
    else if (config->autoLoadTiling)
    {
        if (!config->lastLoadedTileName.isEmpty())
        {
            slot_loadTiling(config->lastLoadedTileName,SM_LOAD_SINGLE);
        }
    }
    else if (config->autoLoadDesigns)
    {
        slot_loadDesign(config->lastLoadedDesignId);
    }

    config->lockView = oldPanelLock;    // restore
    emit sig_lockStatus();

    emit sig_refreshView();
}

TiledPatternMaker::~TiledPatternMaker()
{
    view->releaseInstance();
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

    DesignMaker * designMaker = DesignMaker::getInstance();

    view->dump(true);
    designMaker->clearDesigns();
    view->dump(true);

    DesignPtr d = config->availableDesigns.value(design);
    //d->init();
    d->build();
    d->repeat();
    designMaker->addDesign(d);

    config->lastLoadedDesignId = design;

    // size view to design
    QSize size = d->getDesignInfo()->getSize();
    view->setAllCommonActiveSizes(size);
    if (config->scaleToView)
    {
        view->setAllCommonDefinedSizes(size);
    }

    emit sig_refreshView();
    emit sig_loadedDesign(design);
}

void TiledPatternMaker::slot_loadMosaic(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_loadXML() <" << name << ">";
    view->setWindowTitle(QString("Loading: %1").arg(name));
    view->dump(true);

    config->currentlyLoadedXML.clear();
    config->currentlyLoadingXML = name;

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->currentlyLoadingXML.clear();
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;
        emit sig_refreshView();
        emit sig_ready();
        emit sig_mosaicLoaded(name);
    }
}

void TiledPatternMaker::slot_cycleLoadMosaic(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_cycleLoadMosaic() <" << name << ">";
    view->setWindowTitle(QString("Loading: %1").arg(name));

    config->currentlyLoadedXML.clear();
    config->currentlyLoadingXML = name;

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->currentlyLoadingXML.clear();
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;
        emit sig_refreshView();
        emit sig_ready();
    }
}

void TiledPatternMaker::slot_saveMosaic(QString filename)
{
    qDebug() << "TiledPatternMaker::slot_saveMosaic()";

    QString savedFile;
    MosaicManager mm;
    bool rv = mm.saveMosaic(filename,savedFile,false);
    if (rv)
    {
        MapEditor::getInstance()->keepStash(savedFile);
        config->lastLoadedXML      = savedFile;
        config->currentlyLoadedXML = savedFile;
        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(savedFile);
    }
}

void TiledPatternMaker::slot_loadTiling(QString name, eSM_Event mode)
{
    view->setWindowTitle(QString("Loading tiling: %1").arg(name));
    TilingManager tm;
    if (tm.loadTiling(name,mode))
    {
        config->lastLoadedTileName = name;
        emit sig_ready();
        emit sig_tilingLoaded(name);
        emit sig_refreshView();
    }
    else
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Tile <%1> NOT FOUND").arg(name));
        box.exec();
    }
}

void TiledPatternMaker::slot_cyclerLoadTiling(QString name)
{
    view->setWindowTitle(QString("Loading tiling: %1").arg(name));
    TilingManager tm;
    if (tm.loadTiling(name,SM_LOAD_SINGLE))
    {
        config->lastLoadedTileName = name;
        emit sig_refreshView();
        emit sig_ready();
    }
}

void TiledPatternMaker::slot_saveTiling(QString name)
{
    TilingPtr tiling = tilingMaker->getSelected();

    // Tiling Manager haandles OK/FAIL status
    TilingManager tm;
    if (tm.saveTiling(name,tiling))
    {
        emit sig_tilingWritten();
    }
}

void TiledPatternMaker::slot_render()
{
    TilingPtr tp;   // not initialised
    motifMaker->sm_take(tp, SM_RENDER);

    if (!config->lockView)
    {
        controlPanel->selectViewer(VIEW_MOSAIC);
    }
    emit sig_refreshView();
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
    emit sig_closeAllImageViewers();

    QMessageBox box(controlPanel);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void TiledPatternMaker::slot_show_png(QString file, int row, int col)
{
    QString name = config->examplesDir + file;
    QPixmap pix(name);
    QLabel  * label = new QLabel("Put PNG Here");
    label->setPixmap(pix);

    QLayout * l = view->layout();
    QGridLayout * grid = dynamic_cast<QGridLayout*>(l);
    grid->addWidget(label,row,col);

    view->show();
}

void TiledPatternMaker::slot_compareImages(QString nameLeft, QString nameRight, bool autoMode)
{
    //qDebug() << "fileLeft " << nameLeft;
    //qDebug() << "fileRight" << nameRight;

    emit sig_image0(nameLeft);      // sets page_debug status
    emit sig_image1(nameRight);     // sets page_debug status

    if (nameRight.isEmpty())
    {
        qDebug() << "different (no match)" << nameLeft;
        QString str = "No matching image found ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,nameLeft);
        if (!autoMode || config->stopIfDiff)
        {
            showPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    pathLeft  = config->compareDir0 + "/" + nameLeft  + ".bmp";
    pathRight = config->compareDir1 + "/" + nameRight + ".bmp";

    QImage img_left(pathLeft);
    QImage img_right(pathRight);

    if (img_left == img_right)
    {
        qDebug() << "same     " << pathLeft;
        QString str = "Images are the same ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,pathLeft,pathRight);
        if (!autoMode)
        {
            showPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    // files are different
    if ( (cycler->getMode() == CYCLE_COMPARE_ALL_IMAGES || cycler->getMode() == CYCLE_COMPARE_WORKLIST_IMAGES) && config->generate_workList)
    {
        config->workList << nameLeft;
    }

    qDebug() << "different" << pathLeft;

    QString str = "Images are different";
    emit sig_compareResult(str);    // sets page_debug status

    if (autoMode && !config->stopIfDiff)
    {
        emit sig_ready();
        return;
    }

    if  (config->display_differences)
    {
        // display differences
        if (img_left.size() != img_right.size())
        {
            QString str1 = QString("%1 = %2 x %3").arg(pathLeft).arg(img_left.width()).arg(img_left.height());
            QString str2 = QString("%1 = %2 x %3").arg(pathRight).arg(img_right.width()).arg(img_right.height());
            QPixmap  pm = makeTextPixmap("Images different sizes:",str1,str2);
            QString str = "Images are different sizes";
            emit sig_compareResult(str);    // sets page_debug status
            showPixmap(pm, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
        }
        else
        {
            int w = qMin(img_left.width(),img_right.width());
            int h = qMin(img_left.height(),img_right.height());
            QImage result(QSize(w,h),img_left.format());

            for(int i=0; i<h; i++)
            {
                QRgb *rgbLeft   = reinterpret_cast<QRgb*>(img_left.scanLine(i));
                QRgb *rgbRigth  = reinterpret_cast<QRgb*>(img_right.scanLine(i));
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
                showTransparentPixmap(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
            }
            else
            {
               showPixmap(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
            }
        }
    }

    showFirst = false;      // first is not currently being  shown
}

QPixmap TiledPatternMaker::makeTextPixmap(QString txt,QString txt2,QString txt3)
{
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

void TiledPatternMaker::imageKeyPressed(QKeyEvent * k)
{
    int key = k->key();
    if (key == Qt::Key_Space)
    {
        emit sig_closeAllImageViewers();
        emit sig_takeNext();
    }
    else if (key == 'Q')
    {
        emit sig_closeAllImageViewers();
        emit sig_cyclerQuit();
    }
    else if (key == 'P')
    {
        // ping-pong
        emit sig_closeAllImageViewers();
        slot_view_image(pathLeft,pathRight);
    }
    else if (key == 'S')
    {
        QPixmap pm(pathLeft);
        ImageWidget * widget = showPixmap(pm,pathLeft);
        widget->move(widget->pos().x()-100,widget->pos().y());

        QPixmap pm2(pathRight);
        widget = showPixmap(pm2,pathRight);
        widget->move(widget->pos().x()+100,widget->pos().y());
    }
    else if (key == 'L')
    {
        qWarning() << "FILE LOGGED (needs attention)";
        emit sig_closeAllImageViewers();
        emit sig_takeNext();
    }
}

void TiledPatternMaker::slot_view_image(QString left, QString right)
{
    showFirst = !showFirst;
    if (showFirst)
    {
        QPixmap pm(left);
        if (!config->compare_transparent)
            showPixmap(pm,left);
        else
            showTransparentPixmap(pm,left);
    }
    else
    {
        QPixmap pm(right);
        if (!config->compare_transparent)
            showPixmap(pm,right);
        else
            showTransparentPixmap(pm,right);
    }
}

ImageWidget *  TiledPatternMaker::showPixmap(QPixmap & pixmap,QString title)
{
    ImageWidget * widget = new ImageWidget;
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->setWindowTitle(title);
    widget->show();

    return widget;
}

TransparentWidget * TiledPatternMaker::showTransparentPixmap(QPixmap & pixmap,QString title)
{
    TransparentWidget * widget = new TransparentWidget;
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->setWindowTitle(title);
    widget->show();

    return widget;
}
