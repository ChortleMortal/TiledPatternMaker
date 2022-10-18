/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright (c) 2016-2022 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 */

#include <QColor>
#include <QApplication>
#include <QMessageBox>
#include <QScreen>
#include <QWindow>
#include <QPainter>

#include "tiledpatternmaker.h"
#include "legacy/design_maker.h"
#include "legacy/designs.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/cycler.h"
#include "mosaic/mosaic_manager.h"
#include "panels/panel.h"
#include "panels/splitscreen.h"
#include "settings/configuration.h"
#include "settings/model_settings.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"
#include "widgets/image_layer.h"
#include "widgets/transparentwidget.h"

using std::make_shared;

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
    originalPalette   = QApplication::palette();

    if (config->darkTheme)
    {
        setDarkTheme(true);
    }

    view            = ViewControl::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    motifMaker      = MotifMaker::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    mapEditor       = MapEditor::getInstance();
    cropMaker      = CropMaker::getInstance();

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
    qRegisterMetaType<TilePtr>("TilePtr");

    connect(this, &TiledPatternMaker::sig_refreshView, view, &ViewControl::slot_refreshView);

    connect(view, &View::sig_raiseMenu, this, &TiledPatternMaker::slot_raiseMenu);

    // create cycler
    QThread * thread = new QThread();
    thread->start();

    cycler = Cycler::getInstance();
    cycler->init(thread);

    connect(this,  &TiledPatternMaker::sig_ready,        cycler, &Cycler::slot_ready);
    connect(this,  &TiledPatternMaker::sig_takeNext,     cycler, &Cycler::slot_ready);
    connect(this,  &TiledPatternMaker::sig_cyclerQuit,   cycler, &Cycler::slot_stopCycle);
    connect(this,  &TiledPatternMaker::sig_primaryDisplay,this,  &TiledPatternMaker::slot_bringToPrimaryScreen, Qt::QueuedConnection);

    connect(cycler,    &Cycler::sig_cycleLoadMosaic,this,  &TiledPatternMaker::slot_cycleLoadMosaic);
    connect(cycler,    &Cycler::sig_cycleLoadTiling,this,  &TiledPatternMaker::slot_cyclerLoadTiling);
    connect(cycler,    &Cycler::sig_finished,       this,  &TiledPatternMaker::slot_cyclerFinished);
    connect(cycler,    &Cycler::sig_compare,        this,  &TiledPatternMaker::slot_compareImages);
    connect(cycler,    &Cycler::sig_show_png,       this,  &TiledPatternMaker::slot_show_png);

    // init makers
    mosaicMaker->init();
    motifMaker->init();
    tilingMaker->init();

    // pop-up control panel
    controlPanel = ControlPanel::getInstance();
    controlPanel->init(this);
    controlPanel->show();
    controlPanel->setWindowState((controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
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
            slot_loadMosaic(config->lastLoadedXML,false);
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

    if (config->primaryDisplay)
    {
        config->primaryDisplay = false;
        emit sig_primaryDisplay();
    }
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

void TiledPatternMaker::setDarkTheme(bool enb)
{
    if (enb)
    {
        auto palette = QPalette();
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, Qt::black);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(0x777777));
      //palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
      //palette.setColor(QPalette::HighlightedText, Qt::black);
        QApplication::setPalette(palette);
    }
}

void TiledPatternMaker::slot_loadDesign(eDesign design)
{
    DesignPtr d = config->availableDesigns.value(design);
    if (!d)
    {
        return;
    }

    slot_buildDesign(design);

    if (!config->lockView)
    {
        controlPanel->selectViewer(VIEW_DESIGN);
    }

    emit sig_refreshView();
}

void TiledPatternMaker::slot_buildDesign(eDesign design)
{
    qDebug().noquote() << "TiledPatternMaker::slot_buildDesign" << Design::getDesignName(design);

    DesignMaker * designMaker = DesignMaker::getInstance();

    view->dump(true);
    designMaker->unload();
    view->dump(true);

    DesignPtr d = config->availableDesigns.value(design);

    designMaker->addDesign(d);

    config->lastLoadedDesignId = design;

    // size view to design
    QSize size = d->getDesignInfo().getSize();
    view->frameSettings.initialiseCommon(size,size);
    view->frameSettings.initialise(VIEW_DESIGN,size,size);
    view->frameSettings.setModelAlignment(M_ALIGN_NONE);

    view->removeAllImages();

    emit sig_refreshView();

    QCoreApplication::processEvents();

    d->build();
    d->repeat();

    emit sig_loadedDesign(design);
}

void TiledPatternMaker::slot_loadMosaic(QString name,bool ready)
{
    qDebug().noquote() << "TiledPatternMaker::slot_loadXML() <" << name << ">";

    controlPanel->setLoadState(ControlPanel::LOADING_MOSAIC,name);
    view->dump(true);

    config->currentlyLoadedXML.clear();

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        view->removeAllImages();

        emit sig_refreshView();
        if (ready)
        {
            emit sig_ready();
        }
        emit sig_mosaicLoaded(name);
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TiledPatternMaker::slot_cycleLoadMosaic(QString name)
{
    qDebug().noquote() << "TiledPatternMaker::slot_cycleLoadMosaic() <" << name << ">";

    controlPanel->setLoadState(ControlPanel::LOADING_MOSAIC,name);

    config->currentlyLoadedXML.clear();

    MosaicManager mm;
    bool rv = mm.loadMosaic(name);
    if (rv)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        view->removeAllImages();

        emit sig_refreshView();
        emit sig_ready();
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TiledPatternMaker::slot_saveMosaic(QString filename,bool test)
{
    qDebug() << "TiledPatternMaker::slot_saveMosaic()";

    QString savedFile;
    MosaicManager mm;
    bool rv = mm.saveMosaic(filename,savedFile,false,test);
    if (rv)
    {
        mapEditor->keepStash(savedFile);
        config->lastLoadedXML      = savedFile;
        config->currentlyLoadedXML = savedFile;

        emit sig_mosaicWritten();
        emit sig_mosaicLoaded(savedFile);
    }
}

void TiledPatternMaker::slot_loadTiling(QString name, eSM_Event mode)
{
    controlPanel->setLoadState(ControlPanel::LOADING_TILING,name);

    TilingManager tm;
    if (tm.loadTiling(name,mode))
    {
        config->lastLoadedTileName = name;

        view->removeAllImages();

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
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TiledPatternMaker::slot_cyclerLoadTiling(QString name)
{
    controlPanel->setLoadState(ControlPanel::LOADING_TILING,name);

    TilingManager tm;
    if (tm.loadTiling(name,SM_LOAD_SINGLE))
    {
        config->lastLoadedTileName = name;

        view->removeAllImages();

        emit sig_refreshView();
        emit sig_ready();
    }
    controlPanel->setLoadState(ControlPanel::LOADING_NONE);
}

void TiledPatternMaker::slot_saveTiling(QString name)
{
    TilingPtr tiling = tilingMaker->getSelected();

    TilingManager tm;
    tm.saveTiling(name,tiling); // Tiling Manager haandles OK/FAIL status
}

void TiledPatternMaker::slot_render()
{
    TilingPtr dummy;
    motifMaker->sm_takeUp(dummy, SM_RENDER);
    QVector<PrototypePtr> dummy2;
    mosaicMaker->sm_takeUp(dummy2,SM_RENDER);

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
    QMessageBox box(controlPanel);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();

    emit sig_closeAllImageViewers();
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

void TiledPatternMaker::slot_compareLoaded(QString leftName, bool autoMode)
{
    nameLeft  = leftName;
    //nameRight = "Loaded Mosaic";
    emit sig_image0(nameLeft);      // sets page_debug status
    //emit sig_image1(nameRight);      // sets page_debug status

    pathLeft  = config->compareDir0 + "/" + nameLeft  + ".bmp";
    pathRight.clear();

    QImage img_left(pathLeft);

    QPixmap pixmap    = view->grab();
    QImage  img_right = pixmap.toImage();

    compare(img_left,img_right,autoMode);
}

void TiledPatternMaker::slot_compareImages(QString leftName, QString rightName, bool autoMode)
{
    nameLeft  = leftName;
    nameRight = rightName;
    //qDebug() << "fileLeft " << nameLeft;
    //qDebug() << "fileRight" << nameRight;

    emit sig_image0(nameLeft);      // sets page_debug status
    emit sig_image1(nameRight);     // sets page_debug status

    if (nameRight.isEmpty())
    {
        qWarning() << "different (no match)" << nameLeft;
        QString str = "No matching image found ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,nameLeft);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
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

    compare(img_left,img_right,autoMode);
}

void TiledPatternMaker::compare(QImage & img_left, QImage & img_right, bool autoMode)
{
    if (img_left.isNull())
    {
        qWarning() << "Image not found" << nameLeft;
        QString str = "Image not found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,nameLeft);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    if (img_left == img_right)
    {
        qInfo() << "same     " << pathLeft;
        QString str = "Images are the same";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,pathLeft,pathRight);
        if (!autoMode)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    if (img_right.isNull())
    {
        qWarning() << "different (no match)" << nameLeft;
        QString str = "No matching image found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,nameLeft);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    qWarning() << "different" << pathLeft;

    // files are different
    if ( (cycler->getMode() == CYCLE_COMPARE_ALL_IMAGES || cycler->getMode() == CYCLE_COMPARE_WORKLIST_IMAGES) && config->generate_workList)
    {
        config->addWorkList(nameLeft);
    }

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
            popupPixmap(pm, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
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
                popupTransparentPixmap(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
            }
            else
            {
               popupPixmap(pixmap, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
            }
        }
    }
    else
    {
        // show images
        QPixmap pm(pathLeft);
        ImageWidget * widget = popupPixmap(pm,pathLeft);
        widget->move(widget->pos().x()-200,widget->pos().y());

        QPixmap pm2;
        if (!pathRight.isEmpty())
        {
            pm2 = QPixmap(pathRight);
        }
        else
        {
            pm2 = view->grab();
        }
        widget = popupPixmap(pm2,pathRight);
        widget->move(widget->pos().x()+200,widget->pos().y());
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
    else if (key == 'C')
    {
        emit sig_closeAllImageViewers();
        slot_compareImages(nameLeft,nameRight,false);
    }
    else if (key == 'D')
    {
        emit sig_closeAllImageViewers();
        emit sig_deleteCurrentInWorklist();
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
        slot_view_image(pathLeft,pathRight,config->compare_transparent,true);
    }
    else if (key == 'S')
    {
        QPixmap pm(pathLeft);
        ImageWidget * widget = popupPixmap(pm,pathLeft);
        widget->move(widget->pos().x()-100,widget->pos().y());

        QPixmap pm2;
        if (!pathRight.isEmpty())
        {
            pm2 = QPixmap(pathRight);
        }
        else
        {
            pm2 = view->grab();
        }
        widget = popupPixmap(pm2,pathRight);
        widget->move(widget->pos().x()+100,widget->pos().y());
    }
    else if (key == 'L')
    {
        qWarning() << "FILE LOGGED (needs attention)";
        emit sig_closeAllImageViewers();
        emit sig_takeNext();
    }
}

void TiledPatternMaker::slot_view_image(QString left, QString right, bool transparent, bool popup)
{
    showFirst = !showFirst;
    QString file =  (showFirst) ? left : right;

    QPixmap pm;
    if (popup)
    {
        if (!transparent)
        {
            if (!file.isEmpty())
            {
                pm.load(file);
            }
            else
            {
                pm = view->grab();
            }
            popupPixmap(pm,file);
        }
        else
        {
            Q_ASSERT(transparent);
            if (config->filter_transparent)
            {
                if (!file.isEmpty())
                {
                    pm = createTransparentPixmap(QImage(file));
                }
                else
                {
                    QImage img = view->grab().toImage();
                    pm = createTransparentPixmap(img);
                }
            }
            else
            {
                if (!file.isEmpty())
                {
                    pm.load(file);
                }
                else
                {
                    pm = view->grab();
                }
            }
            popupTransparentPixmap(pm,file);
        }
    }
    else
    {
        Q_ASSERT(!popup);
        if (config->filter_transparent)
        {
            if (!file.isEmpty())
            {
                pm = createTransparentPixmap(QImage(file));
            }
            else
            {
                QImage img = view->grab().toImage();
                pm = createTransparentPixmap(img);
            }
        }
        else
        {
            if (!file.isEmpty())
            {
                pm.load(file);
            }
            else
            {
                pm = view->grab();
            }
        }
        ImgLayerPtr ilp = make_shared<ImageLayer>(file);
        ilp->setPixmap(pm);
        view->addImage(ilp);
        emit sig_refreshView();
    }
}

QPixmap  TiledPatternMaker::createTransparentPixmap(QImage img)
{
    Configuration * config = Configuration::getInstance();
    QColor transparentColor = config->transparentColor;
    int r,g,b,a;
    transparentColor.getRgb(&r,&g,&b,&a);
    qint32 color = (r << 16) + (g << 8) + b;

    // create image from bitmap
    qDebug() << img.width() << img.height() << img.format();

    // add alpha
    QImage img2 = img.convertToFormat(QImage::Format_ARGB32);
    qDebug() << img2.width() << img2.height() << img2.format();

    // make color black transparent
    int w = img2.width();
    int h = img2.height();
    for(int i=0; i<h; i++)
    {
        QRgb *rgb   = reinterpret_cast<QRgb*>(img2.scanLine(i));
        for( int j=0; j<w; j++)
        {
            qint32 pixel = rgb[j];
            //if ((pixel & 0x00ffffff) == 0)
            if ((pixel & 0x00ffffff) == color)
            {
                rgb[j] = 0x00000000;
            }
        }
    }

    // create pixmap
    QPixmap pixmap;
    pixmap.convertFromImage(img2, Qt::NoFormatConversion);
    qDebug() << pixmap.width() << pixmap.height() << pixmap.depth() << pixmap.hasAlpha() << pixmap.hasAlphaChannel();

    return pixmap;
}


ImageWidget *  TiledPatternMaker::popupPixmap(QPixmap & pixmap,QString title)
{
    ImageWidget * widget = new ImageWidget();
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->setWindowTitle(title);
    widget->show();

    return widget;
}

TransparentWidget * TiledPatternMaker::popupTransparentPixmap(QPixmap & pixmap,QString title)
{
    TransparentWidget * widget = new TransparentWidget(title);
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->show();

    return widget;
}
