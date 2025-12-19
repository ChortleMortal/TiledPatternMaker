#pragma once
#ifndef QTAPPLOG_H
#define QTAPPLOG_H

#include <QMutex>
#include <QTextDocument>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QFile>
#include "sys/enums/elogmode.h"

typedef std::shared_ptr<class QTextEdit> TextEditPtr;

struct sTrapMsg
{
    QtMsgType type;
    QString   msg;
};

class qtAppLog
{
public:
    static qtAppLog  * getInstance();
    static void        releaseInstance();

    static TextEditPtr getTextEditor() { return ted; }

    static void crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void init();
    void clear();
    void suspend(bool enable);  // suspend takes precedence over trap
    void trap(bool enable);

    void logToStdErr(bool enable)   { _logToStderr = enable; }
    void logToDisk(bool enable)     { _logToDisk   = enable; }
    void logToAppDir(bool enable)   { _logToAppDir = enable; }
    void logToPanel(bool enable)    { _logToPanel  = enable; }
    void logLines(bool enable)      { _logLines    = enable; }
    void logDebug(bool enable)      { _logDebug    = enable; }
    void logTimer(eLogTimer val);

    bool getLogDebug()              { return _logDebug; }

    void saveLog(QString to);

    QString logDir()                { return this->_logDir; }
    static QString currentLogName;
    static QString baseLogName;
    static const QVector<sTrapMsg> & getTrap() { return _trappedMsgs; }

protected:
    qtAppLog();
    virtual ~qtAppLog();

    void logToFile(QString & msg);
    void logToFile2(QString & msg);
    void endTrap();

private:
    static qtAppLog  * mpThis;
    static QMutex	 * pLogLock;
    static TextEditPtr ted;

    static bool	_logToStderr;
    static bool _logToDisk;
    static bool _logToAppDir;
    static bool _logToPanel;
    static bool _logLines;
    static bool _logDebug;

    static bool	_active;
    static bool	_active2;
    static bool _suspended;
    static bool _trapping;

    static int _line;

    static std::string str1;
    static std::string str2;
    static std::string str3;
    static std::string str4;

    static QVector<sTrapMsg> _trappedMsgs;
    static eLogTimer         _logTimerSetting;
    static QElapsedTimer     elapseTimer;

    static QtMessageHandler  originalHandler;

    QFile	mLogFile;
    QFile	mLogFile2;
    QString _logDir;

};

#endif
