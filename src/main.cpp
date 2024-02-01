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

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDate>
#include <QFileInfo>
#include <QSettings>
#include <QStyleHints>

#include "tiledpatternmaker.h"
#include "misc/qtapplog.h"
#include "misc/sys.h"
#include "misc/version.h"
#include "settings/configuration.h"

#if defined(Q_OS_WINDOWS)
#include <windows.h>
#include <psapi.h>

HANDLE hHandle = 0;
#endif

TiledPatternMaker * theApp          = nullptr;
QtMessageHandler originalMsgHandler = nullptr;

void stackInfo()
{
#if defined(Q_OS_WINDOWS)
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    qInfo() << "Stack size      :" << (static_cast<double>(pmc.WorkingSetSize) / static_cast<double>(1024 * 1024)) << "MB";
#endif
}

void closeHandle()
{
#if defined(Q_OS_WINDOWS)
    if (hHandle)
    {
        ReleaseMutex(hHandle); // Explicitly release mutex
        CloseHandle(hHandle); // close handle before terminating
        hHandle = 0;
    }
#endif
}

void installLog()
{
    originalMsgHandler = qInstallMessageHandler(&qtAppLog::crashMessageOutput);
}

void uninstallLog()
{
    if (originalMsgHandler)
    {
        qInstallMessageHandler(originalMsgHandler);
    }
}

int main(int argc, char *argv[])
{
    Sys::appInstance    = 0;
    bool noAutoLoad     = false;
    bool mapVerify      = false;
    bool forceLoad      = false;
    bool primaryDisplay = false;
    int currentExitCode = 0;

    init_legacy_designs();

    do
    {
        QApplication app(argc, argv);
        QApplication::setStyle("Fusion");
        QCoreApplication::setOrganizationName("DAC");
        QCoreApplication::setApplicationName("TiledPatternMaker");

#if defined(Q_OS_WINDOWS)
    const wchar_t * uniqueName = L"TiledPatternMaker";
    hHandle = CreateMutexW(NULL, TRUE, uniqueName);
    if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setText("Program already started. Start another?");
            int ret = box.exec();
            if (ret == QMessageBox::No)
            {
                return(1); // Exit program
            }
            Sys::appInstance = 1;
        }
#endif
        // holding down shift key on start to stop autoload or reset registry
        Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
        if (mods & Qt::ShiftModifier)
        {
            if (mods & Qt::AltModifier)
            {
                QMessageBox box;
                box.setIcon(QMessageBox::Warning);
                box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                box.setText("This will erase all registry settings for this application\n\nProceed?");
                int ret = box.exec();
                if (ret == QMessageBox::Yes)
                {
                    QSettings s;
                    s.clear();
                }
            }
            else
            {
                noAutoLoad = true;
            }
        }

        auto config = Configuration::getInstance();

#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
        Qt::ColorScheme  scheme = QApplication::styleHints()->colorScheme();

        switch (config->colorTheme)
        {
        case AUTO_THEME:
            switch (scheme)
            {
            case Qt::ColorScheme::Dark:
                Sys::isDarkTheme = true;
                break;
            case Qt::ColorScheme::Light:
                Sys::isDarkTheme = false;
                break;
            case Qt::ColorScheme::Unknown:
                Sys::isDarkTheme = false;
                break;
            }
            break;

        case LITE_THEME:
            Sys::isDarkTheme = false;
            break;

        case DARK_THEME:
            Sys::isDarkTheme = true;
            break;
        }

#else
        // older versions of Qt
        switch (config->colorTheme)
        {
        case AUTO_THEME:
        case LITE_THEME:
            Sys::isDarkTheme = false;
            break;
        case DARK_THEME:
            Sys::isDarkTheme = true;
            break;
        }
#endif

        auto log = qtAppLog::getInstance();

        log->baseLogName = config->baseLogName;

        if (config->insightMode)
        {
            log->logToStdErr(config->logToStderr);
            log->logToDisk(config->logToDisk);
            log->logToAppDir(config->logToAppDir);
            log->logToPanel(config->logToPanel);
            log->logLines(config->logNumberLines);
            log->logDebug(config->logDebug);
            log->logTimer(config->logTime);     // last
        }
        else
        {
            log->logToStdErr(true);
            log->logToDisk(true);
            log->logToAppDir(false);
            log->logToPanel(false);
            log->logLines(false);
            log->logDebug(false);
            log->logTimer(LOGT_NONE);   // last
        }

        log->init();

        installLog();

        qInfo().noquote() << QCoreApplication::applicationName()  << "Version:" << tpmVersion;
        QDate cd = QDate::currentDate();
        QTime ct = QTime::currentTime();
        qInfo().noquote() << "Started:" << cd.toString() << ct.toString() << "on" << QSysInfo::machineHostName();
#ifdef QT_DEBUG
        qInfo().noquote() << "Debug build" << QSysInfo::productType();
#else
        qInfo().noquote() << "Release build" << QSysInfo::productType();
#endif

        if (config->insightMode)
            qInfo() << "Insight mode";
        else
            qInfo() << "Designer mode";
        qInfo().noquote() << "Qt version      :" << QT_VERSION_STR;

        QFileInfo fi(qtAppLog::currentLogName);
        qInfo().noquote() << "Log file        :"  << fi.canonicalFilePath();

#if defined(Q_OS_WINDOWS)
        qInfo() << "MSC Version     :" << _MSC_VER;
        stackInfo();
#endif
#ifdef __linux__
        qInfo().noquote() << "Font:" << QApplication::font().toString();
        QFont afont = QApplication::font();
        afont.setPointSize(9);
        QApplication::setFont(afont);
#endif
        qInfo().noquote() << "App path        :"  << qApp->applicationDirPath();
        qInfo().noquote() << "Local Media root:"  << config->getMediaRootLocal();
        qInfo().noquote() << "App Media root  :"  << config->getMediaRootAppdata();
        qInfo().noquote() << "Media root      :"  << config->getMediaRoot();
        qInfo().noquote() << "Design Dir      :"  << Sys::rootMosaicDir;
        qInfo().noquote() << "Font            :"  << QApplication::font().toString();

        // parse input
        std::string loadstr;
        for (int i = 1; i < argc; ++i)
        {
            if (qstrcmp(argv[i], "--no-autoload") == 0)
            {
                noAutoLoad = true;
                qInfo() << "--no-autoload";
            }
            if (qstrcmp(argv[i], "--map-verify") == 0)
            {
                mapVerify = true;
                qInfo() << "--map-verify";
            }
            if (qstrcmp(argv[i], "--load") == 0)
            {
                forceLoad = true;
                i++;
                loadstr = std::string(argv[i]);
                qInfo().noquote() <<  "--load" << loadstr.c_str();
            }
            if (qstrcmp(argv[i], "--primary-display") == 0)
            {
                primaryDisplay = true;
                qInfo().noquote() <<  "--primary-display";
            }
            else
            {
                qWarning() << "Unknown parameter" << argv[i];
            }
        }

        // preconfigure
        if (noAutoLoad || mapVerify || forceLoad || primaryDisplay)
        {
            if (noAutoLoad)
            {
                config->autoLoadStyles   = false;
                config->autoLoadTiling   = false;
                config->autoLoadDesigns  = false;
                Sys::enableDetachedPages = false;
            }
            if (mapVerify)
            {
                config->verifyMaps       = true;
            }
            if (forceLoad)
            {
                config->autoLoadStyles   = true;
                config->lastLoadedMosaic = QString(loadstr.c_str());
            }
            if (primaryDisplay)
            {
                Sys::primaryDisplay      = true;
            }
        }

        // instantiate and run
        TiledPatternMaker * maker = new TiledPatternMaker();
        theApp = maker;
        emit theApp->sig_start();

        currentExitCode =  app.exec();

        qInfo() << "Exiting...";

        delete maker;

        qInstallMessageHandler(nullptr);				// restores
        log->releaseInstance();
        config->releaseInstance();

        closeHandle();

    } while (currentExitCode == TiledPatternMaker::EXIT_CODE_REBOOT);


    return currentExitCode;
}
