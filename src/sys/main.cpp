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

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDate>
#include <QFileInfo>
#include <QSettings>
#include <QStyleHints>
#include <QProcess>
#include <QStyle>
#include <QCommandLineParser>

#include "sys/tiledpatternmaker.h"
#include "sys/enums/edesign.h"
#include "sys/qt/qtapplog.h"
#include "sys/sys.h"
#include "sys/version.h"
#include "model/settings/configuration.h"

#if defined(Q_OS_WINDOWS)
#include <windows.h>
#include <psapi.h>
#endif

#define USE_LOG

TiledPatternMaker* theApp = nullptr;

namespace
{
#if defined(Q_OS_WINDOWS)
    struct WinMutexGuard
    {
        WinMutexGuard() noexcept = default; // ensure default-constructible

        HANDLE handle = nullptr;

        bool create(const wchar_t* name)
        {
            handle = CreateMutexW(NULL, TRUE, name);
            return handle != nullptr;
        }

        bool alreadyExists() const
        {
            return (GetLastError() == ERROR_ALREADY_EXISTS);
        }

        ~WinMutexGuard()
        {
            if (handle)
            {
                ReleaseMutex(handle);
                CloseHandle(handle);
                handle = nullptr;
            }
        }

        WinMutexGuard(const WinMutexGuard&) = delete;
        WinMutexGuard& operator=(const WinMutexGuard&) = delete;
    };
#endif

    void stackInfo()
    {
#if defined(Q_OS_WINDOWS)
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            qInfo() << "Stack size        :" << (static_cast<double>(pmc.WorkingSetSize) / static_cast<double>(1024 * 1024)) << "MB";
        }
#endif
    }

    QString runGitCommand(const QStringList& args, int timeoutMs = 1000)
    {
        QProcess proc;
        proc.start("git", args);

        if (!proc.waitForStarted(timeoutMs))
        {
            qWarning() << "git command failed to start:" << args;
            return QString();
        }

        if (!proc.waitForFinished(timeoutMs))
        {
            qWarning() << "git command timed out:" << args;
            proc.kill();
            proc.waitForFinished();
            return QString();
        }

        if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
        {
            // no hard error here - just log and return empty
            qDebug() << "git returned non-zero exit code:" << proc.exitCode() << "args:" << args;
            return QString();
        }

        const QByteArray out = proc.readAllStandardOutput();
        return QString::fromUtf8(out).trimmed();
    }

    void getGitInfo()
    {
        // git branch
        Sys::gitBranch = runGitCommand({ "branch", "--show-current" });

        // git sha
        Sys::gitSha = runGitCommand({ "rev-parse", "--short", "HEAD" });

        // git root (we store just the last path component, same behavior as original)
        QString root = runGitCommand({ "rev-parse", "--show-toplevel" });
        if (!root.isEmpty())
        {
            QFileInfo fi(root);
            Sys::gitRoot = fi.fileName();
        }
    }

    void logSystemInfo()
    {
        QTime ct = QTime::currentTime();
        QDate cd = QDate::currentDate();
        qInfo().noquote() << QCoreApplication::applicationName() << ":" << "started" << cd.toString() << ct.toString() << "on" << QSysInfo::machineHostName();

        qInfo().noquote() << "Version           :" << tpmVersion;
#ifdef QT_DEBUG
        qInfo().noquote() << "Build             : Debug" << QSysInfo::productType();
#else
        qInfo().noquote() << "Build             : Release" << QSysInfo::productType();
#endif

        if (Sys::config && Sys::config->insightMode)
            qInfo().noquote() << "Mode              : Insight mode";
        else
            qInfo().noquote() << "Mode              : Designer mode";

        qInfo().noquote() << "Qt version        :" << QT_VERSION_STR;

#if defined(Q_OS_WINDOWS)
        qInfo() << "MSC Version       :" << _MSC_VER;
        stackInfo();
#endif

        qInfo().noquote() << "App path          :" << qApp->applicationDirPath();
        if (Sys::config)
        {
            qInfo().noquote() << "Local Media root  :" << Sys::config->getMediaRootLocal();
            qInfo().noquote() << "App Media root    :" << Sys::config->getMediaRootAppdata();
            qInfo().noquote() << "Media root        :" << Sys::config->getMediaRoot();
        }
        qInfo().noquote() << "Design Dir        :" << Sys::rootMosaicDir;
        qInfo().noquote() << "App Font          :" << QApplication::font().toString();
        qInfo().noquote() << "App Style         :" << QApplication::style();
        QFileInfo fi(qtAppLog::currentLogName);
        qInfo().noquote() << "Log file          :" << fi.canonicalFilePath();
        const QFont font = qtAppLog::getTextEditor()->font();
        qInfo().noquote() << "Log font          :" << font.toString();
        qInfo().noquote() << "Platform          :" << QGuiApplication::platformName();

        getGitInfo();
        if (!Sys::gitBranch.isEmpty())
            qInfo().noquote() << "git branch        :" << Sys::gitBranch;
    }

    void firstBirthday()
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
            s.sync();
            qInfo() << "Registry settings cleared.";
        }
    }
} // anonymous namespace

int main(int argc, char* argv[])
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
        // Ensure QApplication is created before any QWidget-based objects (QMessageBox, etc.)
        QApplication app(argc, argv);
        QApplication::setStyle("Fusion");
        QCoreApplication::setOrganizationName("DAC");
        QCoreApplication::setApplicationName("TiledPatternMaker");

#if defined(Q_OS_WINDOWS)
       WinMutexGuard instanceGuard;
        const wchar_t* uniqueName = L"TiledPatternMaker";
        if (!instanceGuard.create(uniqueName))
        {
            // Failed to create mutex; continue but log
            qWarning() << "Failed to create single-instance mutex.";
        }
        else if (instanceGuard.alreadyExists())
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setText("Program already started. Start another?");
            int ret = box.exec();
            if (ret == QMessageBox::No)
            {
                return 1; // Exit program
            }
            Sys::appInstance = 1;
        }
#endif

        // Restore original startup keyboard-modifier behavior:
        // - Shift + Alt: clear registry/settings
        // - Shift only: disable autoload and force primary display
        // - Shift only: disable autoload and force primary display
        Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
        if (mods & Qt::ShiftModifier)
        {
            if (mods & Qt::AltModifier)
            {
                firstBirthday();
            }
            else
            {
                noAutoLoad = true;
                primaryDisplay = true;
            }
        }

        // parse input using QCommandLineParser
        QCommandLineParser parser;
        parser.setApplicationDescription("TiledPatternMaker");
        parser.addHelpOption();
        parser.addVersionOption();
        QCommandLineOption firstBirthdayOpt(QStringList() << "first-birthday", "Reset all settings to default.");
        QCommandLineOption noAutoOpt(QStringList() << "no-autoload", "Do not autoload last mosaic on startup.");
        QCommandLineOption mapVerifyOpt(QStringList() << "map-verify", "Run map verification.");
        QCommandLineOption loadOpt(QStringList() << "load", "Force load the specified mosaic.", "file");
        QCommandLineOption primaryDisplayOpt(QStringList() << "primary-display", "Force primary display output.");
        parser.addOption(firstBirthdayOpt);
        parser.addOption(noAutoOpt);
        parser.addOption(mapVerifyOpt);
        parser.addOption(loadOpt);
        parser.addOption(primaryDisplayOpt);
        parser.process(app);

        if (parser.isSet(firstBirthdayOpt))
        {
            firstBirthday();
            qInfo() << "--first-birthday";
        }

        if (parser.isSet(noAutoOpt))
        {
            noAutoLoad = true;
            qInfo() << "--no-autoload";
        }
        if (parser.isSet(mapVerifyOpt))
        {
            mapVerify = true;
            qInfo() << "--map-verify";
        }
        QString loadstr;
        if (parser.isSet(loadOpt))
        {
            forceLoad = true;
            loadstr = parser.value(loadOpt).toStdString().c_str();
            qInfo().noquote() << "--load" << loadstr;
        }
        if (parser.isSet(primaryDisplayOpt))
        {
            primaryDisplay = true;
            qInfo().noquote() << "--primary-display";
        }

        // load configuration (use RAII)
        auto config = std::make_unique<Configuration>();
        Sys::config = config.get();

#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
        Qt::ColorScheme scheme = QApplication::styleHints()->colorScheme();

        switch (Sys::config->colorTheme)
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
                QApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
                Sys::isDarkTheme = false;
                break;
            }
            break;

        case LITE_THEME:
            Sys::isDarkTheme = false;
            QApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
            break;

        case DARK_THEME:
            Sys::isDarkTheme = true;
            QApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
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

#ifdef __linux__
        QFont afont = QApplication::font();
        afont.setPointSize(8);
        QApplication::setFont(afont);
#endif

        auto log = qtAppLog::getInstance();
        Sys::log = log;

#ifdef USE_LOG
        log->baseLogName = config->baseLogName;

        if (config->insightMode)
        {
            log->logToStdErr(config->logToStderr);
            log->logToDisk(config->logToDisk);
            log->logToAppDir(config->logToAppDir);
            log->logToPanel(config->logToPanel);
            log->logLines(config->logNumberLines);
            log->logDebug(config->logDebug);
            log->logTimer(config->logTime); // last
        }
        else
        {
            log->logToStdErr(true);
            log->logToDisk(true);
            log->logToAppDir(false);
            log->logToPanel(false);
            log->logLines(false);
            log->logDebug(false);
            log->logTimer(LOGT_NONE); // last
        }

        log->init();
#endif

        logSystemInfo();

        // preconfigure according to parsed options
        if (noAutoLoad)
        {
            config->autoLoadLast = false;
        }
        if (mapVerify)
        {
            config->verifyMaps = true;
        }
        if (forceLoad)
        {
            config->autoLoadLast = true;
            config->lastLoadedMosaic.set(QString::fromStdString(loadstr.toStdString()));
        }
        if (primaryDisplay)
        {
            Sys::primaryDisplay = true;
            Sys::enableDetachedPages = false;
        }

        // instantiate and run (use RAII)
        auto maker = std::make_unique<TiledPatternMaker>();
        theApp = maker.get();
        emit theApp->sig_start();

        currentExitCode = app.exec();

        qInfo() << "Exiting...";

        // teardown in reverse order of construction
        maker.reset();
        qInfo() << "TPM exited";

        log->releaseInstance();
        qInfo() << "Log closed.";

        // release application-level config
        Sys::config = nullptr;
        config.reset();

    } while (currentExitCode == TiledPatternMaker::EXIT_CODE_REBOOT);

    return currentExitCode;
}
