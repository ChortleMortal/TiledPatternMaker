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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QSettings>
#include <QStandardPaths>
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
#include <tlhelp32.h>
#endif

#if defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#define USE_LOG

TiledPatternMaker* theApp = nullptr;

namespace
{
#if defined(Q_OS_WINDOWS)
    void stackInfo()
    {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            qInfo() << "Stack size        :" << (static_cast<double>(pmc.WorkingSetSize) / static_cast<double>(1024 * 1024)) << "MB";
        }
    }
#endif

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

        qInfo().noquote() << "Version           :" << TpmVersion::getVersion();
#ifdef QT_DEBUG
        qInfo().noquote() << "Build             : Debug" << QSysInfo::productType()  << TpmVersion::getTimestamp();
#else
        qInfo().noquote() << "Build             : Release" << QSysInfo::productType()  << TpmVersion::getTimestamp();
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
#if defined(Q_OS_LINUX)
        qInfo().noquote() << "Media Version     :" << LINUX_MEDIA_FILES_VERSION;
#endif
        qInfo().noquote() << "App path          :" << qApp->applicationDirPath();
        qInfo().noquote() << "Media root        :" << Sys::getRootMediaDir();
        qInfo().noquote() << "Images root       :" << Sys::getRootImageDir();
        qInfo().noquote() << "Log root          :" << Sys::getLogDir();
        qInfo().noquote() << "Config store      :" << Sys::settings().fileName();

        qInfo().noquote() << "Design Dir        :" << Sys::rootMosaicDir;
        qInfo().noquote() << "App Font          :" << QApplication::font().toString();
        qInfo().noquote() << "App Style         :" << QApplication::style();
        QFileInfo fi(qtAppLog::currentLogName);
        qInfo().noquote() << "Log file          :" << fi.canonicalFilePath();
        const QFont font = qtAppLog::getTextEditor()->font();
        qInfo().noquote() << "Log font          :" << font.toString();
        qInfo().noquote() << "Platform          :" << QGuiApplication::platformName();
        qInfo().noquote() << "App Instance      :" << Sys::appInstance;
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
            QSettings s = Sys::settings();
            s.clear();
            s.sync();
            qInfo() << "Registry settings cleared.";
        }
    }

#if defined(Q_OS_WINDOWS)
    bool launchedFromQtCreator()
    {
        DWORD pid = GetCurrentProcessId();
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);

        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE)
            return false;

        DWORD parentPid = 0;
        if (Process32First(snap, &pe))
        {
            do
            {
                if (pe.th32ProcessID == pid)
                {
                    parentPid = pe.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(snap, &pe));
        }
        CloseHandle(snap);

        if (!parentPid)
            return false;

        HANDLE parent = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, parentPid);
        if (!parent)
            return false;

        wchar_t name[MAX_PATH];
        GetModuleBaseNameW(parent, nullptr, name, MAX_PATH);
        CloseHandle(parent);

        QString exe = QString::fromWCharArray(name).toLower();
        return exe.contains("qtcreator")
               || exe.contains("qtcreator_process_stub")
               || exe.contains("gdb")
               || exe.contains("lldb")
               || exe.contains("cdb");
    }
#endif

#if defined(Q_OS_LINUX)
    bool launchedFromQtCreator()
    {
        // Qt Creator (or gdb/lldb when run under its debugger) is our direct
        // parent process when launched from the IDE; a normal desktop/shell
        // launch's parent is a shell, terminal, or init/session manager.
        pid_t parentPid = getppid();
        if (parentPid <= 0)
            return false;

        QFile commFile(QString("/proc/%1/comm").arg(parentPid));
        if (!commFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QString name = QString::fromUtf8(commFile.readAll()).trimmed().toLower();
        return name.contains("qtcreator")
               || name.contains("gdb")
               || name.contains("lldb");
    }
#endif

} // anonymous namespace

int main(int argc, char* argv[])
{
#ifdef Q_OS_LINUX
    // Force XCB (X11/XWayland) backend to avoid a ~10s Wayland
    // xdg-activation busy-cursor timeout on GNOME/Mutter.
    // Only set it if the user/environment hasn't already specified
    // a platform explicitly, so power users can still override.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
    {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    Sys::appInstance    = 0;
    bool noAutoLoad     = false;
    bool mapVerify      = false;
    bool forceLoad      = false;
    bool primaryDisplay = false;
    int currentExitCode = 0;

    init_legacy_designs();

    // Cross-platform (Windows/Linux/macOS) second-instance detection, via an
    // advisory lock file in the per-user cache dir. Lives for the whole
    // process (across EXIT_CODE_REBOOT restarts of the QApplication below),
    // so the lock is only released on real process exit.
    QString lockDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(lockDir);
    QLockFile instanceLock(lockDir + "/TiledPatternMaker.lock");
    instanceLock.setStaleLockTime(0); // long-lived resource: rely on liveness check, not a time cutoff
    bool instanceLockAttempted = false;

    do
    {
        // Ensure QApplication is created before any QWidget-based objects (QMessageBox, etc.)
        QApplication app(argc, argv);
        QApplication::setStyle("Fusion");
        QCoreApplication::setApplicationName("TiledPatternMaker");

#if defined(Q_OS_WINDOWS) || defined(Q_OS_LINUX)
        if (launchedFromQtCreator())
        {
            // Note legacy code could be running at 0 or 1, but never 2
            Sys::appInstance = 2;
        }
        else
#endif
        if (!instanceLockAttempted)
        {
            // Only probe/create the lock once; later reboot-loop iterations
            // keep whatever instance id was already established.
            instanceLockAttempted = true;
            if (!instanceLock.tryLock(100))
            {
                if (instanceLock.error() == QLockFile::LockFailedError)
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
                else
                {
                    qWarning() << "Failed to create single-instance lock file:" << instanceLock.error();
                }
            }
        }

        // Restore original startup keyboard-modifier behavior:
        // - Shift + Alt: clear registry/settings
        // - Shift only: disable autoload and force primary display
        // - Shift only: disable autoload and force primary display
        Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
        if (mods & Qt::ShiftModifier)
        {
            if (mods & Qt::AltModifier)
            {
                // shift-alt
                firstBirthday();
            }
            if (mods & Qt::ControlModifier)
            {
                // shift-ctrl
                primaryDisplay = true;
            }
            else
            {
                // shift
                noAutoLoad = true;
            }
        }

        // parse input using QCommandLineParser
        QCommandLineParser parser;
        parser.setApplicationDescription("TiledPatternMaker");
        parser.addHelpOption();
        parser.addVersionOption();
        QCommandLineOption firstBirthdayOpt(QStringList()  << "first-birthday",  "Reset all settings to default.");
        QCommandLineOption noAutoOpt(QStringList()         << "no-autoload",     "Do not autoload last mosaic on startup.");
        QCommandLineOption mapVerifyOpt(QStringList()      << "map-verify",      "Run map verification.");
        QCommandLineOption loadOpt(QStringList()           << "load",            "Force load the specified mosaic.", "file");
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

        // restartApp() disables panel updates before rebooting
        Sys::updatePanel = true;

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

        // now that theme is established
        config->loadBackgroundColors();

#ifdef __linux__
        QFont afont = QApplication::font();
        afont.setPointSize(8);
        QApplication::setFont(afont);
#endif

        // start the log
        auto log = qtAppLog::getInstance();
        Sys::log = log;

#ifdef USE_LOG
        if (config->insightMode)
        {
            log->logToStdErr(config->logToStderr);
            log->logToDisk(config->logToDisk);
            log->logToPanel(config->logToPanel);
            log->logLines(config->logNumberLines);
            log->logDebug(config->logDebug);
            log->logTimer(config->logTime); // last
        }
        else
        {
            log->logToStdErr(true);
            log->logToDisk(true);
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
