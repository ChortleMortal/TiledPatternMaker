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

#include "sys/tiledpatternmaker.h"
#include "gui/map_editor/map_editor.h"
#include "gui/panels/panel_page_controller.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/split_screen.h"
#include "gui/top/view_controller.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "sys/enums/efilesystem.h"
#include "sys/fileservices.h"
#include "sys/sys.h"

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
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "sys/sys/versioning.h"
#include "model/tilings/tiling_manager.h"
#include "sys/sys/fileservices.h"
#endif

int const TiledPatternMaker::EXIT_CODE_REBOOT = -123456789;

TiledPatternMaker::TiledPatternMaker() : QObject()
{
    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::slot_start, Qt::QueuedConnection);

    splitter = nullptr;
    sys      = nullptr;
}

void TiledPatternMaker::slot_start()
{
    // instantiate everything
    bool oldPanelLock      = Sys::config->lockView;
    Sys::config->lockView  = true;    // disables view switching during init

    setPaletteColors();

    qRegisterMetaType<MapPtr>("MapPtr");
    qRegisterMetaType<PolyPtr>("PolyPtr");
    qRegisterMetaType<TilePtr>("TilePtr");
    qRegisterMetaType<QList<int>>();
    qRegisterMetaType<VersionedName>("VersionName");

    // instantiate main classes
    sys = new Sys;

#ifdef TEST_MEMORY_LEAKS
    qDebug() << "testing memory management";
    testMemoryManagement();
    qDebug() << "memory management test complete";
#endif

    // split-screen
    if (Sys::config->splitScreen)
    {
        splitScreen();
    }

    Sys::controlPanel->show();
    Sys::controlPanel->setWindowState((Sys::controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    Sys::controlPanel->raise();
    Sys::controlPanel->activateWindow();

    if (Sys::config->splitScreen)
    {
        int width = Sys::controlPanel->width();
        splitter->setLHSWidth(width);
    }

    Sys::view->show();
    Sys::view->raise();

    connect(this, &TiledPatternMaker::sig_floatPages,      Sys::controlPanel->getPageController(), &PanelPageController::slot_floatPages,Qt::QueuedConnection);
    connect(this, &TiledPatternMaker::sig_reconstructView, Sys::viewController, &ViewController::slot_reconstructView);
    connect(this, &TiledPatternMaker::sig_primaryDisplay,  this,                &TiledPatternMaker::slot_bringToPrimaryScreen, Qt::QueuedConnection);

    connect(Sys::view,          &View::sig_close,           this, &TiledPatternMaker::slot_stop);
    connect(Sys::controlPanel,  &ControlPanel::sig_close,   this, &TiledPatternMaker::slot_stop);
    connect(Sys::mapEditor,     &MapEditor::sig_close,      this, &TiledPatternMaker::slot_stop);
    connect(Sys::tilingMaker,   &TilingMaker::sig_close,    this, &TiledPatternMaker::slot_stop);

    emit sig_floatPages();

    // get started - kick off
    if (Sys::config->autoLoadStyles && !Sys::config->lastLoadedMosaic.isEmpty())
    {
        VersionedFile vfile = FileServices::getFile(Sys::config->lastLoadedMosaic,FILE_MOSAIC);
        Sys::mosaicMaker->loadMosaic(vfile);
    }
    else if (Sys::config->autoLoadTiling &&  !Sys::config->lastLoadedTiling.isEmpty())
    {
        VersionedFile vfile = FileServices::getFile(Sys::config->lastLoadedTiling,FILE_TILING);
        Sys::tilingMaker->loadTiling(vfile,TILM_LOAD_SINGLE);
    }
    else if (Sys::config->autoLoadDesigns)
    {
        eDesign design   = (eDesign)designs.key(Sys::config->lastLoadedLegacy.get());
        Sys::designMaker->slot_loadDesign(design);
    }

    Sys::config->lockView = oldPanelLock;    // restore
    emit sig_lockStatus();

    emit sig_reconstructView();

    if (Sys::primaryDisplay)
    {
        Sys::primaryDisplay = false;
        emit sig_primaryDisplay();
    }

    Sys::splash->remove();

    qInfo() << "TPM started";
}

TiledPatternMaker::~TiledPatternMaker()
{
    if (sys)
    {
        delete sys;
        sys = nullptr;
    }
}

void TiledPatternMaker::slot_stop()
{
    static bool stopping = false;

    if (stopping)
        return;
    stopping = true;

    delete sys;
    sys = nullptr;

    qApp->quit();
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
        palette.setColor(QPalette::Highlight,       Qt::yellow);
        palette.setColor(QPalette::HighlightedText, Qt::red);
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
        palette.setColor(QPalette::Highlight,       Qt::yellow);
        palette.setColor(QPalette::HighlightedText, Qt::red);
    }

    QApplication::setPalette(palette);
}

void TiledPatternMaker::slot_render()
{
    TilingPtr dummy;
    Sys::prototypeMaker->sm_takeUp(dummy, PROM_RENDER);

    if (!Sys::config->lockView)
    {
        Sys::controlPanel->delegateView(VIEW_MOSAIC);
    }
    emit sig_reconstructView();
}

void TiledPatternMaker::slot_bringToPrimaryScreen()
{
    QScreen * primary = qApp->primaryScreen();

    QWindow * wh = Sys::view->windowHandle();
    if (wh)
    {
        wh->setScreen(primary);
        Sys::view->move(primary->geometry().center() - Sys::view->rect().center());
        Sys::view->setWindowState((Sys::view->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        Sys::view->raise();  // for MacOS
        Sys::view->activateWindow();  // for windows
        Sys::controlPanel->move(primary->geometry().center() - Sys::view->rect().center());
        Sys::controlPanel->setWindowState((Sys::controlPanel->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        Sys::controlPanel->raise();  // for MacOS
        Sys::controlPanel->activateWindow();  // for windows
    }
}

void TiledPatternMaker::splitScreen()
{
    Q_ASSERT(Sys::config->splitScreen);

    // save current positions
    QSettings s;
    s.setValue((QString("viewPos/%1").arg(Sys::appInstance)),Sys::view->pos());
    s.setValue(QString("panelPos/%1").arg(Sys::appInstance), Sys::controlPanel->pos());

    // split the screen
    Q_ASSERT(!splitter);
    splitter = new SplitScreen();
    splitter->show();
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

#ifdef TEST_MEMORY_LEAKS

void TiledPatternMaker::testMemoryManagement()
{
    Sys::dumpRefs();

    // test tiling
    TilingManager  * tm = new TilingManager;
    VersionedName vn("12-6-Trigon");
    TilingPtr tp = tm->loadTiling(vn,TILM_LOAD_FROM_MOSAIC);
    Sys::dumpRefs();

    delete tm;
    Sys::dumpRefs();

    tp.reset();
    Sys::dumpRefs();

    // test motif
    VersionedName vname("12-6-trigon.v4");
    VersionedFile file = FileServices::getFile(vname,FILE_MOSAIC);
    QFile afile(file.getVersionedName().get());
    MosaicReader * loader = new MosaicReader;
    auto mosaic = loader->readXML(file);
    Sys::dumpRefs();

    delete loader;
    Sys::dumpRefs();

    mosaic.reset();
    Sys::dumpRefs();
}

#endif
