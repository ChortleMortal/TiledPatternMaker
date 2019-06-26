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
#include <string>

using std::string;

class qtAppLog
{
public:
    static qtAppLog  * getInstance();
    static void        releaseInstance();
    static void		   crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void logDebugMessages(bool enable);
    void copyLog(QString name);

protected:
    qtAppLog();
    ~qtAppLog();

    void rotate();
    void log(QtMsgType type, QString & msg);

private:
    static qtAppLog * mpThis;
    static QMutex	* pLogLock;

    QString path;
    QString oldLogName;
    QString currentLogName;

    QElapsedTimer elapseTimer;

    QFile	mCurrentFile;
    bool	mbLogDebugMessages;
    bool	active;
};

#endif // QTAPPLOG_H
