#pragma once
#ifndef PAGE_LOG_H
#define PAGE_LOG_H

class TextEditorWidget;

#include <QScrollBar>
#include <QDialog>
#include "panels/panel_page.h"

typedef std::shared_ptr<class QTextEdit> TextEditPtr;

class page_log : public panel_page
{
    Q_OBJECT

public:
    page_log(ControlPanel * cpanel);
    virtual ~page_log();

    void onEnter() override;
    void onExit() override {}
    void onRefresh() override;

public slots:
    void slot_copyLog();

private slots:
    void    slot_viewLog();
    void    slot_options();
    void    slot_sizePlus();
    void    slot_sizeMinus();
    void    slot_search();

protected:
    void    viewSaved();
    void    viewLog();

private:
    QCheckBox   * follow;
    QCheckBox   * reverseSearch;
    TextEditPtr   logText;
    TextEditPtr   savedText;
    QLineEdit   * search;

    TextEditorWidget * tew;

    QPushButton * btnViewLog;

    bool          viewingLog;
};

class LogOptionsDlg : public QDialog
{
    Q_OBJECT

public:
    LogOptionsDlg(QWidget * parent);

private:
    QCheckBox   * cbLogElapsedTime;
    QCheckBox   * cbLogIntervalTime;

    Configuration * config;

private slots:
    void    slot_logToStdErr(bool enable);
    void    slot_logToDisk(bool enable);
    void    slot_logToPanel(bool enable);
    void    slot_numberLines(bool enable);
    void    slot_logDebug(bool enable);
    void    slot_elapsedTime(bool enable);
    void    slot_intervalTime(bool enable);
};

#endif
