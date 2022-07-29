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

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDate>
#include <QFileInfo>

#include "tiledpatternmaker.h"
#include "misc/qtapplog.h"
#include "misc/version.h"
#include "settings/configuration.h"

TiledPatternMaker * theApp = nullptr;

#if defined(Q_OS_WINDOWS)
#include <windows.h>
#include <psapi.h>

HANDLE hHandle = 0;
#endif

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

int main(int argc, char *argv[])
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
    }
#endif

    auto config = Configuration::getInstance();

    auto log = qtAppLog::getInstance();
    qInstallMessageHandler(&qtAppLog::crashMessageOutput);

    if (config->insightMode)
    {
        log->logToStdErr(config->logToStderr);
        log->logToDisk(config->logToDisk);
        log->logToPanel(config->logToPanel);
        log->logLines(config->logNumberLines);
        log->logDebug(config->logDebug);
        log->logTimer(config->logTime);     // last
    }
    else
    {
        log->logToStdErr(true);
        log->logToDisk(true);
        log->logToPanel(false);
        log->logLines(false);
        log->logDebug(false);
        log->logTimer(LOGT_NONE);   // last
    }
    if (config->darkTheme)
    {
        log->logDarkMode(true);
    }

    qInfo().noquote() << QCoreApplication::applicationName()  << "Version:" << tpmVersion;
    QDate cd = QDate::currentDate();
    QTime ct = QTime::currentTime();
    qInfo().noquote() << "Started:" << cd.toString() << ct.toString() << "on" << QSysInfo::machineHostName();
#ifdef QT_DEBUG
    qInfo().noquote() << "Debug build";
#else
    qInfo().noquote() << "Release build";
#endif
    if (config->insightMode)
        qInfo() << "Insight mode";
    else
        qInfo() << "Designer mode";
    qInfo().noquote() << "Qt version      :" << QT_VERSION_STR;

    QFileInfo fi(qtAppLog::currentLogName);
    qInfo().noquote() << "Log file        :"  << fi.canonicalFilePath();

    stackInfo();

#ifdef __linux__
    qDebug().noquote() << "Font:" << QApplication::font().toString();
    QFont afont = QApplication::font();
    afont.setPointSize(8);
    QApplication::setFont(afont);
#endif
    qInfo().noquote() << "App path        :"  << qApp->applicationDirPath();
    qInfo().noquote() << "Local Media root:"  << config->getMediaRootLocal();
    qInfo().noquote() << "App Media root  :"  << config->getMediaRootAppdata();
    qInfo().noquote() << "Media root      :"  << config->getMediaRoot();
    qInfo().noquote() << "Design Dir      :"  << config->rootDesignDir;
    qInfo().noquote() << "Font            :"  << QApplication::font().toString();

    // parse input
    bool noAutoLoad = false;
    bool mapVerify  = false;
    bool forceLoad  = false;
    bool primaryDisplay = false;

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

    // holding down shift key on start stop autoload
    Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
    if (mods & Qt::ShiftModifier)
    {
        noAutoLoad = true;
    }

    // preconfigure
    if (noAutoLoad || mapVerify || forceLoad || primaryDisplay)
    {
        if (noAutoLoad)
        {
            config->autoLoadStyles  = false;
            config->autoLoadTiling  = false;
            config->autoLoadDesigns = false;
            config->enableDetachedPages = false;
        }
        if (mapVerify)
        {
            config->verifyMaps      = true;
        }
        if (forceLoad)
        {
            config->autoLoadStyles  = true;
            config->lastLoadedXML   = QString(loadstr.c_str());
        }
        if (primaryDisplay)
        {
            config->primaryDisplay = true;
        }
    }

    // instantiate and run
    TiledPatternMaker * maker = new TiledPatternMaker();
    theApp = maker;
    emit theApp->sig_start();

    int rv =  app.exec();

    qInfo() << "Exiting...";

    delete maker;

    qInstallMessageHandler(nullptr);				// restores
    qtAppLog::releaseInstance();

    closeHandle();

    return rv;
}
