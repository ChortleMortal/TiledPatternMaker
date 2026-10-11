#pragma once

class LogWidget;

#include <QScrollBar>
#include <QDialog>
#include <QCheckBox>
#include "gui/panels/panel_page.h"

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

    void reinitLog();

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
    LogWidget   * logWidget;
    TextEditPtr   logText;
    TextEditPtr   savedText;

    QCheckBox   * follow;
    QCheckBox   * reverseSearch;
    QLineEdit   * search;
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
