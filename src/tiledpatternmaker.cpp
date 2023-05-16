/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright (c) 2016-2023 David A. Casper  email: david.casper@gmail.com
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

#include "makers/crop_maker/crop_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/cycler.h"

#include "panels/panel.h"
#include "panels/splitscreen.h"
#include "settings/configuration.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"
#include "widgets/image_layer.h"
#include "widgets/memory_combo.h"
#include "widgets/transparentwidget.h"

#ifdef QT_DEBUG
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#else
#include <signal.h>
#endif
#endif

using std::make_shared;

#undef TEST_MEMORY_LEAKS
#ifdef TEST_MEMORY_LEAKS
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader.h"
#include "misc/fileservices.h"
#endif


TiledPatternMaker::TiledPatternMaker(int instance) : QObject()
{
    this->instance = instance;
    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::startEverything, Qt::QueuedConnection);

    _showA = false;
}

void TiledPatternMaker::startEverything()
{
    // instantiate everything
    config            = Configuration::getInstance();
    bool oldPanelLock = config->lockView;
    config->lockView  = true;    // disables view switching during init

    config->appInstance = instance;

    if (config->darkTheme)
    {
        setDarkTheme(true);
    }

#ifdef TEST_MEMORY_LEAKS
    qDebug() << "testing memory management";
    testMemoryManagement();
    qDebug() << "memory management test complete";
#endif

    view            = ViewControl::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    mapEditor       = MapEditor::getInstance();
    cropMaker       = CropMaker::getInstance();

    view->init();

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

    connect(cycler,    &Cycler::sig_finished,       this,  &TiledPatternMaker::slot_cyclerFinished);
    connect(cycler,    &Cycler::sig_compare,        this,  &TiledPatternMaker::slot_compareBMPs);
    connect(cycler,    &Cycler::sig_show_png,       this,  &TiledPatternMaker::slot_show_png);

    // pop-up control panel
    controlPanel = ControlPanel::getInstance();
    controlPanel->init(this);
    controlPanel->show();
    controlPanel->setWindowState((controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    controlPanel->raise();
    controlPanel->activateWindow();

    // init makers
    mosaicMaker->init();
    prototypeMaker->init();
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
            mosaicMaker->slot_loadMosaic(config->lastLoadedXML,false);
        }
    }
    else if (config->autoLoadTiling)
    {
        if (!config->lastLoadedTileName.isEmpty())
        {
            tilingMaker->slot_loadTiling(config->lastLoadedTileName,TILM_LOAD_SINGLE);
        }
    }
    else if (config->autoLoadDesigns)
    {
        auto designMaker = DesignMaker::getInstance();
        designMaker->slot_loadDesign(config->lastLoadedDesignId);
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

void TiledPatternMaker::slot_render()
{
    TilingPtr dummy;
    prototypeMaker->sm_takeUp(dummy, PROM_RENDER);

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

        s.setValue((QString("viewPos/%1").arg(config->appInstance)),view->pos());
        s.setValue(QString("panelPos/%1").arg(config->appInstance), controlPanel->pos());

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
        controlPanel->move(s.value(QString("panelPos/%1").arg(config->appInstance)).toPoint());
        controlPanel->adjustSize();

        view->show();
        view->move(s.value((QString("viewPos/%1").arg(config->appInstance))).toPoint());

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

void TiledPatternMaker::slot_compareBMMPandLoaded(QString leftName, bool autoMode)
{
    emit sig_closeAllImageViewers();

    nameLeft  = leftName;
    emit sig_image0(nameLeft);      // sets page_debug status
    //emit sig_image1(nameRight);   // sets page_debug status

    pathLeft  = MemoryCombo::getTextFor("leftDir") + "/" + nameLeft  + ".bmp";
    pathRight.clear();

    QImage img_left(pathLeft);

    QPixmap pixmap    = view->grab();
    QImage  img_right = pixmap.toImage();

    compareImages(img_left,img_right,leftName, "Loaded Mosaic",autoMode);
}

void TiledPatternMaker::slot_compareBMPs(QString leftName, QString rightName, bool autoMode)
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

    pathLeft  = MemoryCombo::getTextFor("leftDir")  + "/" + nameLeft  + ".bmp";
    pathRight = MemoryCombo::getTextFor("rightDir") + "/" + nameRight + ".bmp";

    QImage img_left(pathLeft);
    QImage img_right(pathRight);

    compareImages(img_left,img_right,pathLeft,pathRight,autoMode);
}

void TiledPatternMaker::slot_compareBMPsPath(QString file1, QString file2)
{
    qDebug().noquote() << file1;
    qDebug().noquote() << file2;

    if (file1.isEmpty() || file2.isEmpty())
    {
        QString str = "No matching image found ";
        QPixmap  pm = makeTextPixmap(str,nameLeft);
        popupPixmap(pm,str);
        return;
    }

    pathLeft = file1;
    pathRight = file2;

    QImage img_left(file1);
    QImage img_right(file2);

    compareImages(img_left,img_right,file1,file2,false);
}

void TiledPatternMaker::compareImages(QImage & img_left, QImage & img_right, QString title_left, QString title_right, bool autoMode)
{
    _imageA = img_left;
    _imageB = img_right;
    _titleA = title_left;
    _titleB = title_right;
    compareImages(autoMode);
}

void TiledPatternMaker::compareImages(bool autoMode)
{
    if (_imageA.isNull())
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

    if (_imageA == _imageB)
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

    if (_imageB.isNull())
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

    //
    // files are different
    //

    if ( (cycler->getMode() == CYCLE_COMPARE_ALL_BMPS || cycler->getMode() == CYCLE_COMPARE_WORKLIST_BMPS) && config->generate_workList)
    {
        config->worklist.add(nameLeft);
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
        if (_imageA.size() != _imageB.size())
        {
            QString str1 = QString("%1 = %2 x %3").arg(pathLeft).arg(_imageA.width()).arg(_imageA.height());
            QString str2 = QString("%1 = %2 x %3").arg(pathRight).arg(_imageB.width()).arg(_imageB.height());
            QPixmap  pm = makeTextPixmap("Images different sizes:",str1,str2);
            QString str = "Images are different sizes";
            emit sig_compareResult(str);    // sets page_debug status
            popupPixmap(pm, QString("Image Differences (%1) (%2").arg(pathLeft).arg(pathRight));
        }
        else
        {
            int w = qMin(_imageA.width(),_imageB.width());
            int h = qMin(_imageA.height(),_imageB.height());
            QImage result(QSize(w,h),_imageA.format());

            for(int i=0; i<h; i++)
            {
                QRgb *rgbLeft   = reinterpret_cast<QRgb*>(_imageA.scanLine(i));
                QRgb *rgbRigth  = reinterpret_cast<QRgb*>(_imageB.scanLine(i));
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
    QPixmap pixmap(1600,500);
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
        // compare
        emit sig_closeAllImageViewers();
        compareImages(false);
    }
    else if (key == 'D')
    {
        // delete (from current worklist)
        emit sig_closeAllImageViewers();
        emit sig_deleteCurrentInWorklist(false);  // this sends a sig_ready()
    }
    else if (key == 'Q')
    {
        // quit
        emit sig_closeAllImageViewers();
        emit sig_cyclerQuit();
    }
    else if (key == 'P')
    {
        // ping-pong
        emit sig_closeAllImageViewers();
        ping_pong_images(config->compare_transparent,true);
    }
    else if (key == 'S')
    {
        // side-by-side
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
        // log
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

void TiledPatternMaker::ping_pong_images(bool transparent, bool popup)
{
    _showA = !_showA;

    QImage & img  =  (_showA) ? _imageA : _imageB;
    QString title =  (_showA) ? _titleA : _titleB;

    QPixmap pm;
    if (config->filter_transparent)
    {
        pm = createTransparentPixmap(img);
    }
    else
    {
        pm   = QPixmap::fromImage(img);
    }

    if (popup)
    {
        if (!transparent)
        {
            popupPixmap(pm,title);
        }
        else
        {
            popupTransparentPixmap(pm,title);
        }
    }
    else
    {
        ImgLayerPtr ilp = make_shared<ImageLayer>(title);
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

void TiledPatternMaker::appDebugBreak()
{
#ifdef QT_DEBUG
    qWarning() << "appDebugBreak";
#ifdef Q_OS_WINDOWS
        DebugBreak();
#else
        raise(SIGTRAP);
#endif
#endif
}

ImageWidget *  TiledPatternMaker::popupPixmap(QPixmap & pixmap,QString title)
{
    ImageWidget * widget = new ImageWidget();
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->setWindowTitle(title);

    QSettings s;
    QPoint pos = s.value("imageWidgetPos").toPoint();
    if (!pos.isNull())
    {
        widget->move(pos);
    }

    widget->show();

    return widget;
}

TransparentWidget * TiledPatternMaker::popupTransparentPixmap(QPixmap & pixmap,QString title)
{
    TransparentWidget * widget = new TransparentWidget(title);
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);

    QSettings s;
    QPoint pos = s.value("imageWidgetPos").toPoint();
    if (!pos.isNull())
    {
        widget->move(pos);
    }

    widget->show();

    return widget;
}

#ifdef TEST_MEMORY_LEAKS

void TiledPatternMaker::testMemoryManagement()
{
    view = ViewControl::getInstance();

    view->dumpRefs();

    // test tiling
    TilingManager  * tm = new TilingManager;
    TilingPtr tp = tm->loadTiling("12-6-Trigon",TILM_LOAD_FROM_MOSAIC);
    view->dumpRefs();

    delete tm;
    view->dumpRefs();

    tp.reset();
    view->dumpRefs();

    // test motif
    QString name = "12-6-trigon.v4";
    QString file = FileServices::getMosaicXMLFile(name);
    QFile afile(file);
    MosaicReader * loader = new MosaicReader;
    auto mosaic = loader->readXML(file);
    view->dumpRefs();

    delete loader;
    view->dumpRefs();

    mosaic.reset();
    view->dumpRefs();

    emit view->sig_identifyYourself();
}

#endif
