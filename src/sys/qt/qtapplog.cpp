#include <QMessageBox>
#include <QTextCursor>
#include <QFontDatabase>
#include <QApplication>
#include <QDir>
#include <QTextStream>

#if defined(Q_OS_WINDOWS)
#include <Windows.h>
#endif

#include "sys/qt/qtapplog.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"

qtAppLog  *      qtAppLog::mpThis          = nullptr;
QMutex    *      qtAppLog::pLogLock        = nullptr;
QtMessageHandler qtAppLog::originalHandler = nullptr;

TextEditPtr qtAppLog::ted;

QString     qtAppLog::currentLogName;
QString     qtAppLog::baseLogName;
QVector<sTrapMsg> qtAppLog::_trappedMsgs;

bool qtAppLog::_logToStderr = true;
bool qtAppLog::_logToDisk   = true;
bool qtAppLog::_logToAppDir = false;
bool qtAppLog::_logToPanel  = true;
bool qtAppLog::_logLines    = false;
bool qtAppLog::_logDebug    = false;
bool qtAppLog::_active      = false;
bool qtAppLog::_active2     = false;
bool qtAppLog::_suspended   = false;
bool qtAppLog::_trapping    = false;
int  qtAppLog::_line        = 1;

std::string qtAppLog::str1 = "\033[32m";
std::string qtAppLog::str2 = "\033[0m";
std::string qtAppLog::str3 = "\033[35;1m";
std::string qtAppLog::str4 = "\033[37m";

eLogTimer qtAppLog::_logTimerSetting = LOGT_NONE;

QElapsedTimer qtAppLog::elapseTimer;

qtAppLog * qtAppLog::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new qtAppLog();
        pLogLock = new QMutex();
    }
    return mpThis;
}

void qtAppLog::releaseInstance()
{
    ted.reset();
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

qtAppLog::qtAppLog()
{
    ted = std::make_shared<QTextEdit>();

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ted->setFont(fixedFont);

    ted->setMinimumHeight(690);
    ted->setLineWrapMode(QTextEdit::NoWrap);
    ted->setReadOnly(true);
    ted->setStyleSheet("selection-color: rgb(170, 255, 0);  selection-background-color: rgb(255, 0, 0);");

    originalHandler = nullptr;
}

qtAppLog::~qtAppLog()
{
    mLogFile.close();
    mLogFile2.close();
    if (originalHandler)
    {
        qInstallMessageHandler(originalHandler);
    }
    if (pLogLock)
    {
        delete pLogLock;
    }
}

void qtAppLog:: init()
{
    originalHandler = qInstallMessageHandler(&crashMessageOutput);

#if defined(Q_OS_WINDOWS)
    QString root = qApp->applicationDirPath();
    QStringList rootpath = root.split("/");
    if (!_logToAppDir)
    {
        _logDir  = rootpath[0] + "/logs/";
    }
    else
    {
        _logDir = Sys::config->getMediaRoot();
        _logDir.replace("/media", "/logs/");

    }
    QDir adir(_logDir);
    if (!adir.exists())
    {
        if (!adir.mkpath(_logDir))
        {
            QMessageBox box;
            box.setText(QString("Failed to create logfile directory: %1").arg(_logDir));
            box.exec();
            return;
        }
    }
#else
    _logDir = QDir::homePath() + "/logs/";
    QDir adir(_logDir);
    if (!adir.exists())
    {
        if (!adir.mkpath(_logDir))
        {
            QMessageBox box;
            box.setText(QString("Failed to create logfile directory: %1").arg(_logDir));
            box.exec();
            return;
        }
    }
#endif

#ifdef QT_DEBUG
    currentLogName = _logDir + baseLogName + "D.txt";
#if defined(Q_OS_WINDOWS)
    if (Sys::appInstance == 1)
    {
        currentLogName = _logDir + baseLogName + "D1.txt";
    }
#endif
#else
    currentLogName = _logDir + baseLogName + "R.txt";
#if defined(Q_OS_WINDOWS)
    if (Sys::appInstance == 1)
    {
        currentLogName = _logDir + baseLogName + "R1.txt";
    }
#endif
#endif

    // open the main log file
    mLogFile.setFileName(currentLogName);
    _active = mLogFile.open(QIODevice::ReadWrite | QIODevice::Truncate |  QIODevice::Text);
    if (!_active)
    {
        QMessageBox box;
        box.setText(QString("Failed to open logfile: %1").arg(currentLogName));
        box.exec();
        return;
    }

    // open secondary log file
    if (Sys::config->enableLog2)
    {
        mLogFile2.setFileName(_logDir + "tpmlog2.txt");
        _active2 = mLogFile2.open(QIODevice::ReadWrite | QIODevice::Truncate |  QIODevice::Text);
        if (!_active2)
        {
            QMessageBox box;
            box.setText(QString("Failed to open logfile2: %1").arg(_logDir + "tpmlog2.txt"));
            box.exec();
            return;
        }
    }
}

void qtAppLog::clear()
{
    if (_active)
    {
        mLogFile.reset();
    }
    ted->clear();
    _line = 1;
    qDebug() << "Log cleared";
    mLogFile.resize(mLogFile.pos());
}

void qtAppLog::logTimer(eLogTimer val)
{
    _logTimerSetting = val;

    switch (val)
    {
    case LOGT_NONE:
        break;
    case LOGT_INTERVAL:
    case LOGT_ELAPSED:
        elapseTimer.restart();
    }
}

void qtAppLog::logToFile(QString & msg)
{
    QTextStream str(&mLogFile);
    str << msg;
}

void qtAppLog::logToFile2(QString & msg)
{
    QTextStream str(&mLogFile2);
    str << msg;
}

void qtAppLog::crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (_suspended)
    {
        return;
    }
    
    if (_trapping && !Sys::imgGeneratorInUse)
    {
        sTrapMsg stm;
        stm.type = type;
        stm.msg  = msg;
        _trappedMsgs.push_back(stm);
        return;
    }

    QMutexLocker locker(pLogLock);

    QString number;
    if (_logLines)
    {
        number = QString("%1  ").arg(_line++, 5, 10, QChar('0'));
    }

    QString sDelta;
    switch (_logTimerSetting)
    {
    case LOGT_INTERVAL:
    {
        qint64 delta = elapseTimer.restart();
        double qdelta = delta /1000.0;
        sDelta = QString("%1  ").arg(qdelta, 8, 'f', 3, QChar(' '));
    }
        break;
    case LOGT_ELAPSED:
    {
        qint64 delta = elapseTimer.elapsed();
        double qdelta = delta /1000.0;
        sDelta = QString("%1  ").arg(qdelta, 8, 'f', 3, QChar(' '));
    }
        break;
    case LOGT_NONE:
        break;
    }

    QString msg2;
    switch (type)
    {
    case QtDebugMsg:
        if (!_logDebug)
        {
            return;
        }
        msg2 = QString("%3%2Debug   : %1").arg(msg,sDelta,number);
        break;

    case QtInfoMsg:
        msg2 = QString("%3%2Info    : %1").arg(msg,sDelta,number);
        break;

    case QtWarningMsg:
        msg2 = QString("%3%2Warning : %1").arg(msg,sDelta,number);
        break;

    case QtCriticalMsg:
        msg2 = QString("%3%2Critical: %1").arg(msg,sDelta,number);
        break;

    case QtFatalMsg:
        msg2 = QString("%3%2Fatal   : %1").arg(msg,sDelta,number);
        break;
    }

#if 0
    msg2 += QString(" (%1:%2, %3)").arg(context.file).arg(context.line).arg(context.function);
#else
    Q_UNUSED(context)
#endif

    if (_logToStderr)
    {
        std::string str;
        switch (type)
        {
        case QtDebugMsg:
#if defined(Q_OS_WINDOWS)
            str = msg2.toStdString();
#else
            str = str4 + msg2.toStdString() + str2;
#endif
            break;
        case QtInfoMsg:
            str = str1 + msg2.toStdString() + str2;
            break;
        case QtWarningMsg:
        case QtCriticalMsg:
        case QtFatalMsg:
            str = str3 + msg2.toStdString() + str2;
            break;
        }
#if defined(Q_OS_WINDOWS)
        str += '\n';
        OutputDebugStringA(str.c_str());
#else
        fprintf(stderr,"%s\n",str.c_str());
#endif
    }

    msg2 += "\n";
    if (_logToPanel && !_trapping)
    {
        switch (type)
        {
        case QtDebugMsg:
            if (Sys::isDarkTheme)
                ted->setTextColor(Qt::white);
            else
                ted->setTextColor(Qt::black);
            break;
        case QtInfoMsg:
            if (Sys::isDarkTheme)
                ted->setTextColor(Qt::green);
            else
                ted->setTextColor(Qt::darkGreen);
            break;
        case QtWarningMsg:
            if (Sys::isDarkTheme)
                ted->setTextColor(Qt::red);
            else
                ted->setTextColor(Qt::darkRed);
            break;
        case QtCriticalMsg:
            if (Sys::isDarkTheme)
                ted->setTextColor(Qt::red);
            else
                ted->setTextColor(Qt::darkRed);
            break;
        case QtFatalMsg:
            if (Sys::isDarkTheme)
                ted->setTextColor(Qt::red);
            else
                ted->setTextColor(Qt::darkRed);
            break;
        }
        ted->insertPlainText(msg2);
    }

    if (_active && _logToDisk)
    {
        mpThis->logToFile(msg2);
    }

    if (_active2 && _logToDisk && msg2.contains("LOG2"))
    {
        mpThis->logToFile2(msg2);
    }

#if defined(Q_OS_WINDOWS) && defined(QT_DEBUG)
    if (type == QtCriticalMsg || type == QtFatalMsg)
    {
        DebugBreak();
    }
#endif
}

void qtAppLog::saveLog(QString to)
{
    if (QFile::exists(to))
    {
        QFile::remove(to);
    }

    qDebug() << "Saving log to:" << to;

    QFile::copy(currentLogName,to);
}

void qtAppLog::suspend(bool enable)
{
    _suspended = enable;
}

void qtAppLog::trap(bool enable)
{
    static int count = 0;
    if (enable)
    {
        count++;
        _trapping = true;
    }
    else if (count)
    {
        count--;
        _trapping = (count > 0);
        if (!_trapping)
        {
            endTrap();
        }
    }
}

void qtAppLog::endTrap()
{
    QMessageLogContext context;
    for (auto & stm : std::as_const(_trappedMsgs))
    {
        crashMessageOutput(stm.type,context,stm.msg);
    }
    _trappedMsgs.clear();
}
