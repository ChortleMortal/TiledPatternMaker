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

#include "qtapplog.h"
#include <QtCore>
#include <QMessageBox>

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#endif

#define MAX_LOG_SIZE (50 * 1024 * 1024)
#define MAX_LOG_EXAMINE_COUNT	1000
#undef  MEASURE_ELAPSED

qtAppLog * qtAppLog::mpThis   = nullptr;
QMutex   * qtAppLog::pLogLock = nullptr;

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
    mbLogDebugMessages = true;
    active = false;

#ifdef WIN32
    path = "C:/logs/";
    QDir adir(path);
    if (!adir.exists())
    {
        if (!adir.mkpath(path))
        {
            QMessageBox box;
            box.setText(QString("Failed to create logfile directory: %1").arg(path));
            box.exec();
            return;
        }
    }
    qDebug().noquote() << "log  :"  << adir.canonicalPath();

    if (QLibraryInfo::isDebugBuild())
    {
        currentLogName = "C:/logs/patternLogD.txt";
        oldLogName     = "C:/logs/patternLogD-old.txt";
    }
    else
    {
        currentLogName = "C:/logs/patternLogR.txt";
        oldLogName     = "C:/logs/patternLogRb-old.txt";

    }
#else
    path = "./logs/";
    QDir adir(path);
    if (!adir.exists())
    {
        QDir adir2(".");
        if (!adir2.mkdir("logs"))
        {
            QMessageBox box;
            box.setText(QString("Failed to create logfile directory: %1").arg(path));
            box.exec();
            return;
        }
    }
    qDebug().noquote() << "log  :"  << adir.canonicalPath();

    currentLogName = "./logs/patternLog.txt";
    oldLogName     = "./logs/patternLog-old.txt";
#endif

    mCurrentFile.setFileName(currentLogName);

    active = mCurrentFile.open(QIODevice::ReadWrite | QIODevice::Truncate |  QIODevice::Text);

    if (!active)
    {
        QMessageBox box;
        box.setText(QString("Failed to open logfile: %1").arg(currentLogName));
        box.exec();
        return;
    }

#ifdef MEASURE_ELAPSED
    elapseTimer.start();
#endif
    //rotate();
}

qtAppLog::~qtAppLog()
{
    mCurrentFile.close();
    if (pLogLock)
    {
        delete pLogLock;
    }
}

void qtAppLog::rotate()
{
    qint64 size = mCurrentFile.size();
    if (size > MAX_LOG_SIZE)
    {
        // max size exceeded
        mCurrentFile.close();

        if (QFile::exists(oldLogName))
        {
            // delete old log backup
            QFile::remove(oldLogName);
        }
        // rename current to backup
        mCurrentFile.rename(oldLogName);

        // open new log file
        mCurrentFile.setFileName(currentLogName);
        mCurrentFile.open(QIODevice::Append | QIODevice::Text);
    }
}

void qtAppLog::log(QtMsgType type, QString & msg)
{
#if 0
static int count = 0;
#endif
    if (!active)
    {
        return;
    }

    if (type == QtDebugMsg && !mbLogDebugMessages)
    {
        return;
    }

    QTextStream str(&mCurrentFile);
#ifdef MEASURE_ELAPSED
    //QDateTime tm = QDateTime::currentDateTime();
    //QString  stm = tm.toString("MM/dd/yyyy hh:mm:ss:zzz");
    //str << stm <<  "  " << msg;
    qint64 delta = elapseTimer.restart();
    QString Delta = QString("%1  ").arg(delta, 6, 10, QChar(' '));
    str << Delta << msg;
#else
    str << msg;
#endif
#if 0
    if (++count >= MAX_LOG_EXAMINE_COUNT)
    {
        count = 0;
        rotate();
    }
#endif
}

void qtAppLog::crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    pLogLock->lock();

#if 0
    Q_UNUSED(context);
    QString msg3 = QString("%1\n").arg(msg);
#else
    QString msg2;
    switch (type)
    {
    case QtDebugMsg:
        msg2 = QString("Debug: %1").arg(msg);
        break;
    case QtInfoMsg:
        msg2 = QString("Info: %1").arg(msg);
        break;
    case QtWarningMsg:
        msg2 = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        msg2 = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        msg2 = QString("Fatal: %1").arg(msg);
        break;
    }
#if 0
    QString msg3 = msg2 + QString(" (%1:%2, %3)\n").arg(context.file).arg(context.line).arg(context.function);
#else
    Q_UNUSED(context);
    QString msg3 = msg2 + QString("\n");
#endif
#endif
    QByteArray localMsg = msg3.toLocal8Bit();
#ifdef WIN32
    OutputDebugStringA(localMsg.constData());
#else
    fprintf(stderr,"%s",localMsg.constData());
#endif

    if (mpThis != nullptr)
    {
        mpThis->log(type,msg3);
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

void qtAppLog::logDebugMessages(bool enable)
{
    mbLogDebugMessages = enable;
}


void qtAppLog::copyLog(QString name)
{
    QString to = path + name;

    if (QFile::exists(to))
    {
        QFile::remove(to);
    }

    QFile::copy(currentLogName,to);
}
