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
#include <QTextCursor>

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#endif

#undef  MEASURE_ELAPSED

qtAppLog  * qtAppLog::mpThis   = nullptr;
QMutex    * qtAppLog::pLogLock = nullptr;
QTextEdit * qtAppLog::ted      = nullptr;

QString qtAppLog::currentLogName;

bool qtAppLog::_logToStderr = true;
bool qtAppLog::_logToDisk = true;
bool qtAppLog::_logToPanel  = true;
bool qtAppLog::_active    = false;


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
    QString path = rootpath[0];
    path += "/logs/";

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

    if (QLibraryInfo::isDebugBuild())
    {
        currentLogName = path + "patternLogD.txt";
    }
    else
    {
        currentLogName = path + "patternLogR.txt";
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

    _active = mCurrentFile.open(QIODevice::ReadWrite | QIODevice::Truncate |  QIODevice::Text);

    if (!_active)
    {
        QMessageBox box;
        box.setText(QString("Failed to open logfile: %1").arg(currentLogName));
        box.exec();
        return;
    }

#ifdef MEASURE_ELAPSED
    elapseTimer.start();
#endif

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
}

void qtAppLog::crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static int line = 1;

    if (!_logToStderr && _logToPanel && _logToDisk)
    {
        return;
    }

    pLogLock->lock();

    QString msg2;
    switch (type)
    {
    case QtDebugMsg:
        ted->setTextColor(Qt::black);
        msg2 = QString("Debug   : %1").arg(msg);
        break;
    case QtInfoMsg:
        ted->setTextColor(Qt::black);
        msg2 = QString("Info    : %1").arg(msg);
        break;
    case QtWarningMsg:
        ted->setTextColor(Qt::red);
        msg2 = QString("Warning : %1").arg(msg);
        break;
    case QtCriticalMsg:
        ted->setTextColor(Qt::red);
        msg2 = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        ted->setTextColor(Qt::red);
        msg2 = QString("Fatal   : %1").arg(msg);
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
        QString number = QString("%1  ").arg(line++, 5, 10, QChar('0'));
        ted->insertPlainText(number);
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

void qtAppLog::copyLog(QString name)
{
    QString to = path + name;

    if (QFile::exists(to))
    {
        QFile::remove(to);
    }

    QFile::copy(currentLogName,to);
}

void qtAppLog::saveLog(QString name)
{
    QString to = path + name;

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
