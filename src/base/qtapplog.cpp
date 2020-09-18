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

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#endif

qtAppLog  * qtAppLog::mpThis   = nullptr;
QMutex    * qtAppLog::pLogLock = nullptr;
QTextEdit * qtAppLog::ted      = nullptr;

QString qtAppLog::currentLogName;

bool qtAppLog::_logToStderr = true;
bool qtAppLog::_logToDisk   = true;
bool qtAppLog::_logToPanel  = true;
bool qtAppLog::_logLines    = true;
bool qtAppLog::_logWarningsOnly = false;
bool qtAppLog::_logElapsed  = false;
bool qtAppLog::_active      = false;

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

    if (_logElapsed)
    {
        elapseTimer.start();
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

void qtAppLog::log(QString & msg)
{
    QTextStream str(&mCurrentFile);
        str << msg;
}

void qtAppLog::crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static int line = 1;

    if (!_logToStderr && _logToPanel && _logToDisk)
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
    if (_logElapsed)
    {
        qint64 delta = elapseTimer.restart();
        sDelta = QString("%1  ").arg(delta, 6, 10, QChar(' '));
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

#if 0
    msg2 += QString(" (%1:%2, %3)\n").arg(context.file).arg(context.line).arg(context.function);
#else
    Q_UNUSED(context)
    msg2 += "\n";
#endif

    if (_logToPanel)
    {
        ted->insertPlainText(msg2);
    }

    if (_logToStderr)
    {
        QByteArray localMsg = msg2.toLocal8Bit();
#ifdef WIN32
        OutputDebugStringA(localMsg.constData());
#else
        fprintf(stderr,"%s",localMsg.constData());
#endif
    }

    if (_active && _logToDisk)
    {
        mpThis->log(msg2);
    }

    pLogLock->unlock();

    if (type == QtFatalMsg)
    {
        abort();
    }
#ifdef WIN32
    if (type == QtCriticalMsg)
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
