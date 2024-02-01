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
#include <QGridLayout>
#include <QStyle>

#include "tiledpatternmaker.h"
#include "legacy/design_maker.h"

#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/sys.h"
#include "misc/tpmsplash.h"

#include "panels/controlpanel.h"
#include "panels/splitscreen.h"
#include "settings/configuration.h"
#include "tile/tiling_manager.h"
#include "viewers/view_controller.h"
#include "widgets/memory_combo.h"
#include "widgets/transparent_widget.h"


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

int const TiledPatternMaker::EXIT_CODE_REBOOT = -123456789;

TiledPatternMaker::TiledPatternMaker() : QObject()
{
    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::startEverything, Qt::QueuedConnection);

    splitter = nullptr;
    _splash  = nullptr;
}

void TiledPatternMaker::startEverything()
{
    // instantiate everything
    config            = Configuration::getInstance();
    bool oldPanelLock = config->lockView;
    config->lockView  = true;    // disables view switching during init

#ifdef TEST_MEMORY_LEAKS
    qDebug() << "testing memory management";
    testMemoryManagement();
    qDebug() << "memory management test complete";
#endif

    setPaletteColors();


    view                = new View;
    viewController      = new ViewController;
    Sys::view           = view;
    Sys::viewController = viewController;

    mosaicMaker         = MosaicMaker::getInstance();
    prototypeMaker      = PrototypeMaker::getInstance();
    tilingMaker         = TilingMaker::getInstance();
    mapEditor           = MapEditor::getInstance();
    controlPanel        = ControlPanel::getInstance();

    enableSplash(true);

    // Methinks the makers should be started before views and the control panel started last
    // Finally load something

    // init makers
    mosaicMaker->init();
    prototypeMaker->init();
    tilingMaker->init();

    // init view
    view->init(viewController);
    viewController->init(view);

    qRegisterMetaType<MapPtr>("MapPtr");
    qRegisterMetaType<PolyPtr>("PolyPtr");
    qRegisterMetaType<TilePtr>("TilePtr");
    qRegisterMetaType<QList<int>>();
    
    connect(this, &TiledPatternMaker::sig_refreshView,   viewController, &ViewController::slot_reconstructView);
    connect(this, &TiledPatternMaker::sig_primaryDisplay,this,           &TiledPatternMaker::slot_bringToPrimaryScreen, Qt::QueuedConnection);
    connect(view, &View::sig_raiseMenu,                  this,           &TiledPatternMaker::slot_raiseMenu);

    // init control panel
    controlPanel->init(this);

    // split-screen
    if (config->splitScreen)
    {
       splitScreen();
    }

    controlPanel->show();
    controlPanel->setWindowState((controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    controlPanel->raise();
    controlPanel->activateWindow();

    if (config->splitScreen)
    {
        int width = controlPanel->width();
        splitter->setLHSWidth(width);
    }

    view->show();
    view->raise();
    emit sig_refreshView();

    // this is harmless here but necessary for page floating to be after view is refreshed
    QCoreApplication::processEvents();
    controlPanel->floatPages();

    // get started - kick off
    if (config->autoLoadStyles && !config->lastLoadedMosaic.isEmpty())
    {
        if (!config->lastLoadedMosaic.isEmpty())
        {
            mosaicMaker->loadMosaic(config->lastLoadedMosaic);
        }
    }
    else if (config->autoLoadTiling)
    {
        if (!config->lastLoadedTiling.isEmpty())
        {
            tilingMaker->loadTiling(config->lastLoadedTiling,TILM_LOAD_SINGLE);
        }
    }
    else if (config->autoLoadDesigns)
    {
        auto designMaker = DesignMaker::getInstance();
        eDesign design   = (eDesign)designs.key(config->lastLoadedLegacyDes);
        designMaker->slot_loadDesign(design);
    }

    config->lockView = oldPanelLock;    // restore
    emit sig_lockStatus();

    emit sig_refreshView();

    if (Sys::primaryDisplay)
    {
        Sys::primaryDisplay = false;
        emit sig_primaryDisplay();
    }
}

TiledPatternMaker::~TiledPatternMaker()
{
    delete view;
    delete viewController;    // releases viewers

    DesignMaker::releaseInstance();
    mapEditor->releaseInstance();

    mosaicMaker->releaseInstance();
    prototypeMaker->releaseInstance();
    tilingMaker->releaseInstance();

    controlPanel->closePages();
    controlPanel->close();
    controlPanel->releaseInstance();

    config->releaseInstance();      // performed last
}

void TiledPatternMaker::setPaletteColors()
{
#if 0
    // reverse engineer existing palatte
    QPalette apalette = QApplication::palette();
    qDebug() << apalette.color(QPalette::Window).name();
    qDebug() << apalette.color(QPalette::WindowText).name();
    qDebug() << apalette.color(QPalette::Base).name();
    qDebug() << apalette.color(QPalette::AlternateBase).name();
    qDebug() << apalette.color(QPalette::ToolTipBase).name();
    qDebug() << apalette.color(QPalette::ToolTipText).name();
    qDebug() << apalette.color(QPalette::Text).name();
    qDebug() << apalette.color(QPalette::Button).name();
    qDebug() << apalette.color(QPalette::ButtonText).name();
    qDebug() << apalette.color(QPalette::BrightText).name();
    qDebug() << apalette.color(QPalette::Link).name();
    qDebug() << apalette.color(QPalette::Highlight).name();
    qDebug() << apalette.color(QPalette::HighlightedText).name();
    return;
#endif

    QPalette palette;

    if (Sys::isDarkTheme)
    {
        palette.setColor(QPalette::Window,          QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText,      Qt::white);
        palette.setColor(QPalette::Base,            QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase,     Qt::black);
        palette.setColor(QPalette::ToolTipText,     Qt::white);
        palette.setColor(QPalette::Text,            Qt::white);
        palette.setColor(QPalette::Button,          QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText,      Qt::white);
        palette.setColor(QPalette::BrightText,      Qt::red);
        palette.setColor(QPalette::Link,            QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight,       QColor(0x777777));
      //palette.setColor(QPalette::Highlight,       QColor(42, 130, 218));
      //palette.setColor(QPalette::HighlightedText, Qt::black);
    }
    else
    {
        // reverse engineered
        palette.setColor(QPalette::Window,          QColor(0xf0f0f0));
        palette.setColor(QPalette::WindowText,      Qt::black);
        palette.setColor(QPalette::Base,            Qt::white);
        palette.setColor(QPalette::AlternateBase,   QColor(0xe9e7e3));
        palette.setColor(QPalette::ToolTipBase,     QColor(0xffffdc));
        palette.setColor(QPalette::ToolTipText,     Qt::black);
        palette.setColor(QPalette::Text,            Qt::black);
        palette.setColor(QPalette::Button,          QColor(0xf0f0f0));
        palette.setColor(QPalette::ButtonText,      Qt::black);
        palette.setColor(QPalette::BrightText,      Qt::white);
      //palette.setColor(QPalette::Link,            QColor(0x0000ff));
        palette.setColor(QPalette::Link,            QColor(0x258292));
        palette.setColor(QPalette::Highlight,       QColor(0x0078d7));
      //palette.setColor(QPalette::HighlightedText, QColor("0x0078d7"));
    }

    QApplication::setPalette(palette);
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

void TiledPatternMaker::splitScreen()
{
    Q_ASSERT(config->splitScreen);

    // save current positions
    QSettings s;
    s.setValue((QString("viewPos/%1").arg(Sys::appInstance)),view->pos());
    s.setValue(QString("panelPos/%1").arg(Sys::appInstance), controlPanel->pos());

    // split the screen
    Q_ASSERT(!splitter);
    splitter = new SplitScreen();
    splitter->show();
}

void TiledPatternMaker::enableSplash(bool enable)
{
    if (_splash)
    {
        _splash->hide();
        delete _splash;
    }
    _splash = nullptr;

    if (enable)
    {
        _splash = new TPMSplash();
    }
}

void    TiledPatternMaker::splash(QString & txt)  { if (_splash) _splash->display(txt); }
void    TiledPatternMaker::splash(QString && txt) { if (_splash) _splash->display(txt); }
void    TiledPatternMaker::removeSplash()         { if (_splash) _splash->remove(); }

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
}

#endif
