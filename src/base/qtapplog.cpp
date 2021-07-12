/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/qtapplog.h"
#include <QtCore>
#include <QMessageBox>
#include <QTextCursor>
#include <QFontDatabase>

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#endif

qtAppLog  * qtAppLog::mpThis   = nullptr;
QMutex    * qtAppLog::pLogLock = nullptr;
QTextEdit * qtAppLog::ted      = nullptr;

QString     qtAppLog::currentLogName;
QStringList qtAppLog::_trapString;

bool qtAppLog::_logToStderr = true;
bool qtAppLog::_logToDisk   = true;
bool qtAppLog::_logToPanel  = true;
bool qtAppLog::_logLines    = true;
bool qtAppLog::_logWarningsOnly = false;
bool qtAppLog::_active      = false;
bool qtAppLog::_suspend     = false;
bool qtAppLog::_trap        = false;

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

#ifdef WIN32
    QString root = qApp->applicationDirPath();
    QStringList rootpath = root.split("/");
    _logDir  = rootpath[0] + "/logs/";

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
    _logDir = "./logs/";
    QDir adir(_logDir);
    if (!adir.exists())
    {
        QDir adir2(".");
        if (!adir2.mkdir("logs"))
        {
            QMessageBox box;
            box.setText(QString("Failed to create logfile directory: %1").arg(_logDir));
            box.exec();
            return;
        }
    }
    qDebug().noquote() << "log  :"  << adir.canonicalPath();
#endif

#ifdef QT_DEBUG
    currentLogName = _logDir + "patternLogD.txt";
#else
    currentLogName = _logDir + "patternLogR.txt";
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

    if (_suspend)
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
        if (_logWarningsOnly)
        {
            pLogLock->unlock();
            return;
        }
        ted->setTextColor(Qt::black);
        msg2 = QString("%3%2Debug   : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtInfoMsg:
        if (_logWarningsOnly)
        {
            pLogLock->unlock();
            return;
        }
        ted->setTextColor(Qt::darkGreen);
        msg2 = QString("%3%2Info    : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtWarningMsg:
        ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Warning : %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtCriticalMsg:
        ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Critical: %1").arg(msg).arg(sDelta).arg(number);
        break;
    case QtFatalMsg:
        ted->setTextColor(Qt::darkRed);
        msg2 = QString("%3%2Fatal   : %1").arg(msg).arg(sDelta).arg(number);
        break;
    }

    if (_trap)
    {
        _trapString << msg2;
    }

#if 0
    msg2 += QString(" (%1:%2, %3)\n").arg(context.file).arg(context.line).arg(context.function);
#else
    Q_UNUSED(context)
#endif

    if (_logToStderr)
    {
        std::string s1;
        std::string s2;
        std::string str;
        switch (type)
        {
        case QtDebugMsg:
            str = msg2.toStdString();
            break;
        case QtInfoMsg:
            s1 = "\033[32m";
            s2 = "\033[0m";
            str = s1 + msg2.toStdString() + s2;
            break;
        case QtWarningMsg:
            s1 = "\033[35;1m";
            s2 = "\033[0m";
            str = s1 + msg2.toStdString() + s2;
            break;
        case QtCriticalMsg:
            str = msg2.toStdString();
            break;
        case QtFatalMsg:
            str = msg2.toStdString();
            break;
        }
        str += '\n';
#ifdef WIN32
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

#if defined(WIN32) && defined(QT_DEBUG)
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

void qtAppLog::forceTrapOutput()
{
    bool oldTrap = _trap;
    if (oldTrap)
        _trap = false;
    for (auto str : _trapString)
        qWarning() << str;
    _trap = oldTrap;
}

