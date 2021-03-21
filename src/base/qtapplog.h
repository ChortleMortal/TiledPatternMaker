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

#ifndef QTAPPLOG_H
#define QTAPPLOG_H

#include <QtCore>
#include <QTextDocument>
#include <QTextEdit>

enum eLogTimer
{
    LOGT_NONE,
    LOGT_INTERVAL,
    LOGT_ELAPSED,
};

class qtAppLog
{
public:
    static qtAppLog  * getInstance();
    static void        releaseInstance();

    static QTextEdit * getTextEditor() { return ted; }

    static void crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void logToStdErr(bool enable)   { _logToStderr = enable; }
    void logToDisk(bool enable)     { _logToDisk   = enable; }
    void logToPanel(bool enable)    { _logToPanel  = enable; }
    void logLines(bool enable)      { _logLines    = enable; }
    void logWarningsOnly(bool enb)  { _logWarningsOnly  = enb; }
    void logTimer(eLogTimer val);

    void saveLog(QString to);

    QString logDir() { return this->_logDir; }
    static QString currentLogName;

protected:
    qtAppLog();
    ~qtAppLog();

    void log(QString & msg);

private:
    static qtAppLog  * mpThis;
    static QMutex	 * pLogLock;
    static QTextEdit * ted;

    static bool	_logToStderr;
    static bool _logToDisk;
    static bool _logToPanel;
    static bool _logLines;
    static bool _logWarningsOnly;
    static bool	_active;
    static eLogTimer _logTimerSetting;
    static QElapsedTimer elapseTimer;

    QFile	mCurrentFile;
    QString _logDir;
};

#endif
