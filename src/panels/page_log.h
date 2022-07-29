#ifndef PAGE_LOG_H
#define PAGE_LOG_H

class QCheckBox;
class QTextEdit;

#include <QScrollBar>
#include "widgets/panel_page.h"

class page_log;

class AQScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    AQScrollBar(page_log *plog);
};

class page_log : public panel_page
{
    Q_OBJECT

public:
    page_log(ControlPanel * cpanel);

    void onEnter() override;
    void onExit() override {}
    void onRefresh() override;

public slots:
    void slot_actionTriggered();

private slots:
    void    slot_copyLog();
    void    slot_viewLog();
    void    slot_logToStdErr(bool enable);
    void    slot_logToDisk(bool enable);
    void    slot_logToPanel(bool enable);
    void    slot_numberLines(bool enable);
    void    slot_logDebug(bool enable);
    void    slot_elapsedTime(bool enable);
    void    slot_intervalTime(bool enable);
    void    slot_sizePlus();
    void    slot_sizeMinus();

protected:

private:
    QCheckBox   * follow;
    QTextEdit   * ed;
    AQScrollBar * sbar;
    QCheckBox   * cbLogElapsedTime;
    QCheckBox   * cbLogIntervalTime;
};

#endif
