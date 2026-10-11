/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright (c) 2016-2025 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 */

#include <QApplication>
#include <QTimer>
#include "sys/tiledpatternmaker.h"
#include "gui/map_editor/map_editor.h"
#include "gui/panels/panel_page_controller.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/split_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
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
#include "sys/sys/fileservices.h"
#endif

int const TiledPatternMaker::EXIT_CODE_REBOOT = -123456789;

TiledPatternMaker::TiledPatternMaker() : QObject()
{
    sys = nullptr;

    connect(this,&TiledPatternMaker::sig_start, this, &TiledPatternMaker::slot_start, Qt::QueuedConnection);
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
    qRegisterMetaType<std::weak_ptr<Style>>("std::weak_ptr<Style>");

    // instantiate main classes
    sys = new Sys;

#ifdef TEST_MEMORY_LEAKS
    qDebug() << "testing memory management";
    testMemoryManagement();
    qDebug() << "memory management test complete";
#endif

    if (Sys::splitter)
    {
        int width = Sys::controlPanel->width();
        Sys::splitter->setLHSWidth(width);
    }
    else
    {
        Sys::systemViewController->showView();
        Sys::systemViewController->raiseView();

        Qt::WindowStates state = Sys::controlPanel->windowState();
        if (state & Qt::WindowMinimized)
        {
            state &= ~Qt::WindowMinimized;
            Sys::controlPanel->setWindowState(state);
        }
        Sys::controlPanel->show();

        QTimer::singleShot(0, []() { Sys::controlPanel->raise(); Sys::controlPanel->activateWindow();} );
    }

    // move template files if necessary
    FileServices::relocateTemplateFiles();

    connect(this, &TiledPatternMaker::sig_floatPages,      Sys::controlPanel->getPageController(), &PanelPageController::slot_floatPages,Qt::QueuedConnection);
    connect(this, &TiledPatternMaker::sig_subAttachPage,   Sys::controlPanel->getPageController(), &PanelPageController::slot_subAttachPage,Qt::QueuedConnection);
    connect(this, &TiledPatternMaker::sig_reconstructView, Sys::systemViewController,              &ViewController::slot_reconstructView);
    connect(this, &TiledPatternMaker::sig_primaryDisplay,  this,                                   &TiledPatternMaker::slot_bringToPrimaryScreen, Qt::QueuedConnection);

    connect(Sys::systemViewController->getView(),       &SystemView::sig_close,     this, &TiledPatternMaker::slot_stop);
    connect(Sys::controlPanel,  &ControlPanel::sig_close,   this, &TiledPatternMaker::slot_stop);
    connect(Sys::mapEditor,     &MapEditor::sig_close,      this, &TiledPatternMaker::slot_stop);
    connect(Sys::tilingMaker,   &TilingMaker::sig_close,    this, &TiledPatternMaker::slot_stop);

    if (Sys::splitter && Sys::config->bigScreen)
    {
        emit sig_subAttachPage();
    }
    else
    {
        emit sig_floatPages();
    }

    // get started - kick off
    if (Sys::config->autoLoadLast)
    {
        // the loaders' "nothing shown yet" fallback (enabledGangCount() == 0) must not
        // preempt the session's restored layer selection below in reEnableViewers()
        Sys::startupAutoloading = true;

        switch(Sys::config->lastLoadedType)
        {
        case LT_MOSAIC:
        {
            VersionedFile vfile = FileServices::getFile(Sys::config->lastLoadedMosaic,FILE_MOSAIC);
            if (!vfile.isEmpty())
            {
                Sys::mosaicMaker->load(vfile);
            }
        }   break;

        case LT_TILING:
        {
            VersionedFile vfile = FileServices::getFile(Sys::config->lastLoadedTiling,FILE_TILING);
            if (!vfile.isEmpty())
            {
                Sys::tilingMaker->loadTiling(vfile,TilingMaker::Event::LoadSingle);
            }
        }   break;

        case LT_LEGACY:
        {
            eDesign design = (eDesign)designs.key(Sys::config->lastLoadedLegacy.get());
            if (design != NO_DESIGN)
            {
                Sys::designMaker->slot_loadDesign(design);
            }
        }   break;

        case LT_UNDEFINED:
            break;
        }

        Sys::startupAutoloading = false;
    }

    // re-eanble lock status after auto-loads are complete
    Sys::config->lockView = oldPanelLock;    // restore
    Sys::controlPanel->reEnableViewers();

    emit sig_reconstructView();

    if (Sys::primaryDisplay)
    {
        Sys::primaryDisplay = false;
        emit sig_primaryDisplay();
    }

    Sys::systemViewController->splash_remove();
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
    qDebug() << apalette.color(QPalette::WindowText).name();
    qDebug() << apalette.color(QPalette::Button).name();
    qDebug() << apalette.color(QPalette::Light).name();
    qDebug() << apalette.color(QPalette::Midlight).name();
    qDebug() << apalette.color(QPalette::Dark).name();
    qDebug() << apalette.color(QPalette::Mid).name();
    qDebug() << apalette.color(QPalette::Text).name();
    qDebug() << apalette.color(QPalette::BrightText).name();
    qDebug() << apalette.color(QPalette::ButtonText).name();
    qDebug() << apalette.color(QPalette::Base).name();
    qDebug() << apalette.color(QPalette::Window).name();
    qDebug() << apalette.color(QPalette::Shadow).name();
    qDebug() << apalette.color(QPalette::Highlight).name();
    qDebug() << apalette.color(QPalette::HighlightedText).name();
    qDebug() << apalette.color(QPalette::Link).name();
    qDebug() << apalette.color(QPalette::LinkVisited).name();
    qDebug() << apalette.color(QPalette::AlternateBase).name();
    qDebug() << apalette.color(QPalette::NoRole).name();
    qDebug() << apalette.color(QPalette::ToolTipBase).name();
    qDebug() << apalette.color(QPalette::ToolTipText).name();
    qDebug() << apalette.color(QPalette::PlaceholderText).name();
    qDebug() << apalette.color(QPalette::Accent).name();
    return;

//Light
const QString lightColors[QPalette::NColorRoles] = {
"#000000",  //  WindowText
"#f0f0f0",  //  Button
"#ffffff",  //  Light
"#e3e3e3",  //  Midlight
"#a0a0a0",  //  Dark
"#a0a0a0",  //  Mid
"#000000",  //  Text
"#ffffff",  //  BrightText
"#000000",  //  ButtonText
"#ffffff",  //  Base
"#f0f0f0",  //  Window
"#696969",  //  Shadow
"#0078d4",  //  Highlight
"#ffffff",  //  HighlightedText
"#003e92",  //  Link
"#001a68",  //  LinkVisitied
"#e9e7e3",  //  AlternateBase
"#000000",  //  NoRole
"#ffffdc",  //  ToolTipBase
"#000000",  //  ToolTipText
"#000000",  //  PlaceholderText
"#0067c0",  //  Accent
};

const QString darkColors[QPalette::NColorRoles] = {
"#ffffff",  //
"#3c3c3c",  //
"#787878",  //
"#5a5a5a",  //
"#1e1e1e",  //
"#282828",  //
"#ffffff",  //
"#99ebff",  //
"#ffffff",  //
"#2d2d2d",  //
"#1e1e1e",  //
"#000000",  //
"#0078d4",  //
"#ffffff",  //
"#99ebff",  //
"#4cc2ff",  //
"#001a68",  //
"#000000",  //
"#3c3c3c",  //
"#d4d4d4",  //
"#ffffff",  //
"#4cc2ff",  //
};
#endif

    QPalette palette;

    if (Sys::isDarkTheme)
    {
        // Base colors
        palette.setColor(QPalette::Window,          QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText,      Qt::white);
        palette.setColor(QPalette::Base,            QColor(25, 25, 25));
        palette.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
        palette.setColor(QPalette::Text,            Qt::white);
        palette.setColor(QPalette::BrightText,      Qt::red);
        palette.setColor(QPalette::Button,          QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText,      Qt::white);
        palette.setColor(QPalette::Link,            QColor(42, 130, 218));
        palette.setColor(QPalette::LinkVisited,     QColor(100, 80, 180));

        // Tooltip
        palette.setColor(QPalette::ToolTipBase,     QColor(42, 42, 42));
        palette.setColor(QPalette::ToolTipText,     Qt::white);

        // Highlight
        palette.setColor(QPalette::Highlight,       Qt::yellow);    //QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::red);       //Qt::white);

        // Shading / borders
        palette.setColor(QPalette::Light,           QColor(70, 70, 70));
        palette.setColor(QPalette::Midlight,        QColor(65, 65, 65));
        palette.setColor(QPalette::Mid,             QColor(40, 40, 40));
        palette.setColor(QPalette::Dark,            QColor(35, 35, 35));
        palette.setColor(QPalette::Shadow,          QColor(20, 20, 20));

        // Disabled state
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      QColor(127, 127, 127));
        palette.setColor(QPalette::Disabled, QPalette::Text,            QColor(127, 127, 127));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      QColor(127, 127, 127));
        palette.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(80, 80, 80));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
        palette.setColor(QPalette::Disabled, QPalette::Base,            QColor(40, 40, 40));
        palette.setColor(QPalette::Disabled, QPalette::Button,          QColor(40, 40, 40));
    }
    else
    {
        // reverse engineered
        // Light theme (Fusion-based)
        palette.setColor(QPalette::Window,          QColor(0xf0f0f0));
        palette.setColor(QPalette::WindowText,      Qt::black);
        palette.setColor(QPalette::Base,            Qt::white);
        palette.setColor(QPalette::AlternateBase,   QColor(0xe9e7e3));
        palette.setColor(QPalette::Text,            Qt::black);
        palette.setColor(QPalette::BrightText,      Qt::white);
        palette.setColor(QPalette::Button,          QColor(0xf0f0f0));
        palette.setColor(QPalette::ButtonText,      Qt::black);

        // Tooltip — keep the classic pale yellow, it's correct for light
        palette.setColor(QPalette::ToolTipBase,     QColor(0xffffdc));
        palette.setColor(QPalette::ToolTipText,     Qt::black);

        // Links
        palette.setColor(QPalette::Link,            QColor(0x0000cc));
        palette.setColor(QPalette::LinkVisited,     QColor(0x551a8b));

        // Highlight — replace yellow/red with something standard
        palette.setColor(QPalette::Highlight,       Qt::yellow);        //QColor(0x0078d7));  // Windows-style blue
        palette.setColor(QPalette::HighlightedText, Qt::red);           //Qt::white);

        // Shading / borders (these give buttons and frames their 3D look)
        palette.setColor(QPalette::Light,           Qt::white);
        palette.setColor(QPalette::Midlight,        QColor(0xe3e3e3));
        palette.setColor(QPalette::Mid,             QColor(0xa0a0a0));
        palette.setColor(QPalette::Dark,            QColor(0xa0a0a0));
        palette.setColor(QPalette::Shadow,          QColor(0x696969));

        // Disabled group — very important for greyed-out widgets
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      QColor(0xa0a0a0));
        palette.setColor(QPalette::Disabled, QPalette::Text,            QColor(0xa0a0a0));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      QColor(0xa0a0a0));
        palette.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(0xc0c0c0));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0xa0a0a0));
        palette.setColor(QPalette::Disabled, QPalette::Base,            QColor(0xf0f0f0));
        palette.setColor(QPalette::Disabled, QPalette::Button,          QColor(0xf0f0f0));
    }

    QApplication::setPalette(palette);
}

void TiledPatternMaker::slot_bringToPrimaryScreen()
{
    Sys::systemViewController->bringToPrimaryScreen();
    Sys::controlPanel->bringToPrimaryScreen();
    emit sig_imageToPrimary();
}

#ifdef TEST_MEMORY_LEAKS
void TiledPatternMaker::testMemoryManagement()
{
    Sys::dumpRefs();

    // test tiling
    VersionedName vn("12-6-Trigon");
    VersionedFile vf = FileServices::getFile(vn, FILE_TILING);
    QString failMsg;
    TilingPtr tp = Sys::tilingMaker->readTiling(vf,failMsg);
    Sys::dumpRefs();

    tp.reset();
    Sys::dumpRefs();

    // test motif
    VersionedName vname("12-6-trigon.v4");
    VersionedFile file = FileServices::getFile(vname,FILE_MOSAIC);
    QFile afile(file.getVersionedName().get());
	MosaicReader* loader = new MosaicReader(Sys::viewController);
    auto mosaic = loader->readXML(file);
    Sys::dumpRefs();

    delete loader;
    Sys::dumpRefs();

    mosaic.reset();
    Sys::dumpRefs();
}
#endif
