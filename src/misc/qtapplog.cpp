#include "misc/qtapplog.h"
#include <QMessageBox>
#include <QTextCursor>
#include <QFontDatabase>
#include <QApplication>
#include <QDir>
#include <QTextStream>

#if defined(Q_OS_WINDOWS)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "settings/configuration.h"
#endif


qtAppLog  * qtAppLog::mpThis   = nullptr;
QMutex    * qtAppLog::pLogLock = nullptr;
QTextEdit * qtAppLog::ted      = nullptr;

QString     qtAppLog::currentLogName;
QString     qtAppLog::baseLogName;
QStringList qtAppLog::_trapStringList;

bool qtAppLog::_logToStderr = true;
bool qtAppLog::_logToDisk   = true;
bool qtAppLog::_logToAppDir = false;
bool qtAppLog::_logToPanel  = true;
bool qtAppLog::_logLines    = false;
bool qtAppLog::_logDebug    = false;
bool qtAppLog::_logDarkMode = false;
bool qtAppLog::_active      = false;
bool qtAppLog::_suspended   = false;
bool qtAppLog::_trapping    = false;


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
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

qtAppLog::qtAppLog()
{
    ted = new QTextEdit();
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ted->setFont(fixedFont);
}

void qtAppLog:: init()
{
#if defined(Q_OS_WINDOWS)
    QString root = qApp->applicationDirPath();
    QStringList rootpath = root.split("/");
    if (!_logToAppDir)
    {
        _logDir  = rootpath[0] + "/logs/";
    }
    else
    {
        _logDir = Configuration::getInstance()->getMediaRoot();
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
#else
    currentLogName = _logDir + baseLogName + "R.txt";
#endif

    mCurrentFile.setFileName(currentLogName);

    _active = mCurrentFile.open(QIODevice::ReadWrite | QIODevice::Truncate |  QIODevice::Text);

    if (!_active)
    {
        QMessageBox box;
        box.setText(QString("Failed to open logfile: %1").arg(currentLogName));
        box.exec();
        return;
    }
}

qtAppLog::~qtAppLog()
{
    mCurrentFile.close();
    if (pLogLock)
    {
        delete pLogLock;
    }
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

void qtAppLog::log(QString & msg)
{
    QTextStream str(&mCurrentFile);
    str << msg;
}

void qtAppLog::crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static int line = 1;

    if (_suspended)
    {
        return;
    }

    pLogLock->lock();

    QString number;
    if (_logLines)
    {
        number = QString("%1  ").arg(line, 5, 10, QChar('0'));
        line++;
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
            pLogLock->unlock();
            return;
        }
        if (_logDarkMode)
            ted->setTextColor(Qt::white);
        else
            ted->setTextColor(Qt::black);

        msg2 = QString("%3%2Debug   : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtInfoMsg:
        if (_logDarkMode)
            ted->setTextColor(Qt::green);
        else
            ted->setTextColor(Qt::darkGreen);
        msg2 = QString("%3%2Info    : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtWarningMsg:
        if (_logDarkMode)
            ted->setTextColor(Qt::red);
        else
            ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Warning : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtCriticalMsg:
        if (_logDarkMode)
            ted->setTextColor(Qt::red);
        else
            ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Critical: %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtFatalMsg:
        if (_logDarkMode)
            ted->setTextColor(Qt::red);
        else
            ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Fatal   : %1").arg(msg).arg(sDelta).arg(number);
        break;
    }

    if (_trapping)
    {
        _trapStringList << msg2;
    }

#if 0
    msg2 += QString(" (%1:%2, %3)\n").arg(context.file).arg(context.line).arg(context.function);
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
    if (_logToPanel)
    {
        ted->insertPlainText(msg2);
    }

    if (_active && _logToDisk)
    {
        mpThis->log(msg2);
    }

    pLogLock->unlock();

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

    //QFile::copy(currentLogName,to);

    QString log  = ted->toPlainText();
    QFile file(to);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
             return;

    QTextStream out(&file);
    out << log << "\n";
    file.close();
}

void qtAppLog::suspend(bool enable)
{
    _suspended = enable;
}

void qtAppLog::trap(bool enable)
{
    static int count = 0;
    if (enable)
        count++;
    else if (count)
        count--;
    _trapping = (count > 0);
}

void qtAppLog::trapClear()
{
    _trapStringList.clear();
}