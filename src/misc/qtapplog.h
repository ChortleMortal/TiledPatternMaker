#pragma once
#ifndef QTAPPLOG_H
#define QTAPPLOG_H

#include <QMutex>
#include <QTextDocument>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QFile>
#include "enums/elogmode.h"

class qtAppLog
{
public:
    static qtAppLog  * getInstance();
    static void        releaseInstance();

    static QTextEdit * getTextEditor() { return ted; }

    static void crashMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void init();
    void suspend(bool enable);  // suspend takes precedence over trap
    void trap(bool enable);
    void trapClear();

    void logToStdErr(bool enable)   { _logToStderr = enable; }
    void logToDisk(bool enable)     { _logToDisk   = enable; }
    void logToAppDir(bool enable)   { _logToAppDir = enable; }
    void logToPanel(bool enable)    { _logToPanel  = enable; }
    void logLines(bool enable)      { _logLines    = enable; }
    void logDebug(bool enable)      { _logDebug    = enable; }
    void logDarkMode(bool enable)   { _logDarkMode = enable; }
    void logTimer(eLogTimer val);

    void saveLog(QString to);

    QString logDir() { return this->_logDir; }
    static QString currentLogName;
    static QString baseLogName;
    static const QStringList & getTrap() { return _trapStringList; }

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
    static bool _logToAppDir;
    static bool _logToPanel;
    static bool _logLines;
    static bool _logDebug;
    static bool _logDarkMode;

    static bool	_active;
    static bool _suspended;
    static bool _trapping;


    static std::string str1;
    static std::string str2;
    static std::string str3;
    static std::string str4;

    static QStringList _trapStringList;

    static eLogTimer _logTimerSetting;
    static QElapsedTimer elapseTimer;

    QFile	mCurrentFile;
    QString _logDir;

};

#endif
