#include <QCheckBox>
#include <QTextEdit>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>

#include "panels/page_log.h"
#include "panels/panel.h"
#include "widgets/dlg_textedit.h"
#include "misc/qtapplog.h"
#include "settings/configuration.h"

page_log::page_log(ControlPanel * cpanel)  : panel_page(cpanel, "Log")
{
    QHBoxLayout * hbox = new QHBoxLayout();

    follow = new QCheckBox("Follow tail");
    follow->setChecked(true);
    hbox->addWidget(follow);

    QPushButton * btnSaveLog            = new QPushButton("Save Log");
    QPushButton * btnViewLog            = new QPushButton("View Saved");
    QCheckBox   * cbLogToStderr         = new QCheckBox("Log To stderr");
    QCheckBox   * cbLogToDisk           = new QCheckBox("Log To Disk");
    QCheckBox   * cbLogToPanel          = new QCheckBox("Log To Panel");
    QCheckBox   * cbLogDebug            = new QCheckBox("Log Debug");
    QCheckBox   * cbLogNumberLines      = new QCheckBox("Number Lines");
                  cbLogElapsedTime      = new QCheckBox("Elapsed Time");
                  cbLogIntervalTime     = new QCheckBox("Interval Time");
    QPushButton * sizePlus              = new QPushButton("+");
    QPushButton * sizeMinus             = new QPushButton("-");

    sizePlus->setMaximumWidth(21);
    sizeMinus->setMaximumWidth(21);

    hbox->addWidget(btnSaveLog);
    hbox->addWidget(btnViewLog);
    hbox->addWidget(cbLogToDisk);
    hbox->addWidget(cbLogToStderr);
    hbox->addWidget(cbLogToPanel);
    hbox->addStretch();
    hbox->addWidget(cbLogDebug);
    hbox->addWidget(cbLogNumberLines);
    hbox->addWidget(cbLogIntervalTime);
    hbox->addWidget(cbLogElapsedTime);
    hbox->addWidget(sizePlus);
    hbox->addWidget(sizeMinus);

    vbox->addLayout(hbox);

    cbLogToStderr->setChecked(config->logToStderr);
    cbLogToDisk->setChecked(config->logToDisk);
    cbLogToPanel->setChecked(config->logToPanel);
    cbLogNumberLines->setChecked(config->logNumberLines);
    cbLogDebug->setChecked(config->logDebug);

    switch (config->logTime)
    {
    case LOGT_NONE:
        cbLogElapsedTime->setChecked(false);
        cbLogIntervalTime->setChecked(false);
        break;
    case LOGT_ELAPSED:
        cbLogElapsedTime->setChecked(true);
        cbLogIntervalTime->setChecked(false);
        break;
    case LOGT_INTERVAL:
        cbLogElapsedTime->setChecked(false);
        cbLogIntervalTime->setChecked(true);
        break;
    }

    connect(cbLogToStderr,    &QCheckBox::clicked,    this,   &page_log::slot_logToStdErr);
    connect(cbLogToDisk,      &QCheckBox::clicked,    this,   &page_log::slot_logToDisk);
    connect(cbLogToPanel,     &QCheckBox::clicked,    this,   &page_log::slot_logToPanel);
    connect(cbLogDebug,       &QCheckBox::clicked,    this,   &page_log::slot_logDebug);
    connect(cbLogNumberLines, &QCheckBox::clicked,    this,   &page_log::slot_numberLines);
    connect(cbLogElapsedTime, &QCheckBox::clicked,    this,   &page_log::slot_elapsedTime);
    connect(cbLogIntervalTime,&QCheckBox::clicked,    this,   &page_log::slot_intervalTime);
    connect(btnSaveLog,       &QPushButton::clicked,  this,   &page_log::slot_copyLog);
    connect(btnViewLog,       &QPushButton::clicked,  this,   &page_log::slot_viewLog);
    connect(sizePlus,         &QPushButton::clicked,  this,   &page_log::slot_sizePlus);
    connect(sizeMinus,        &QPushButton::clicked,  this,   &page_log::slot_sizeMinus);

    ed = qtAppLog::getTextEditor();     // linkage to qtAppLog
    ed->setMinimumWidth(800);
    ed->setMinimumHeight(750);
    ed->setLineWrapMode(QTextEdit::NoWrap);
    ed->setReadOnly(true);

    const QFont font = ed->font();
    qInfo().noquote() << "Log font:" << font.toString();

    vbox->addWidget(ed);

    sbar = new AQScrollBar(this);
    ed->setVerticalScrollBar(sbar);

    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);

    vbox->addStretch();
}

void page_log::onEnter()
{
    onRefresh();
}

void page_log::onRefresh()
{
    if (!follow->isChecked())
    {
        return;
    }
    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);
}

void page_log::slot_actionTriggered()
{
    follow->setChecked(false);
}

void page_log::slot_copyLog()
{
    QString cdt    = QDateTime::currentDateTime().toString("yy.MM.dd-hh.mm.ss");
    QString host   = QSysInfo::machineHostName();
    QString branch = panel->gitBranch;
    QString mosaic = config->currentlyLoadedXML;
    Q_ASSERT(!mosaic.contains(".xml"));
    QString name   = cdt + "-" + host;
    if (!branch.isEmpty())
    {
        name += "-" + branch;
    }
    if (!mosaic.isEmpty())
    {
        name += "-" + mosaic;
    }
#ifdef QT_DEBUG
    name += "-DEB";
#else
    name += "-REL";
#endif

    QString path      = qtAppLog::getInstance()->logDir();
    QString nameList  = "TXT (*.txt)";

    QFileDialog dlg(this, "Save log as", path, nameList);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setOptions(QFileDialog::DontConfirmOverwrite);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.selectFile(name);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    QStringList fileList = dlg.selectedFiles();
    if (fileList.isEmpty())
    {
        qDebug() << "No file selected";
        return;
    }
    QString file = fileList.at(0);
    if (!file.contains(".txt"))
    {
        file = file + ".txt";
    }

    qtAppLog * log = qtAppLog::getInstance();
    log->saveLog(file);
}

void page_log::slot_viewLog()
{
    QString path      = qtAppLog::getInstance()->logDir();
    QString nameList  = "Log Files (*.txt)";

    QString filename = QFileDialog::getOpenFileName(this, "Load log", path, nameList);

    if (filename.isEmpty()) return;

    DlgLogView * dlg = new DlgLogView(filename,this);
    dlg->show();
}

void page_log::slot_logToStdErr(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logToStdErr(enable);
    config->logToStderr = enable;
}

void page_log::slot_logToDisk(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logToDisk(enable);
    config->logToDisk = enable;
}

void page_log::slot_logToPanel(bool enable)
{
    if (!enable)
    {
        qDebug() << "Log to Panel: OFF";
    }
    qtAppLog * log = qtAppLog::getInstance();
    log->logToPanel(enable);
    config->logToPanel = enable;
    if (enable)
    {
        qDebug() << "Log to Panel: ON";
    }
}

void page_log::slot_logDebug(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logDebug(enable);
    config->logDebug = enable;
}

void page_log::slot_numberLines(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logLines(enable);
    config->logNumberLines = enable;
}

void page_log::slot_elapsedTime(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    if (enable)
    {
        log->logTimer(LOGT_ELAPSED);
        config->logTime = LOGT_ELAPSED;
        cbLogIntervalTime->blockSignals(true);
        cbLogIntervalTime->setChecked(false);
        cbLogIntervalTime->blockSignals(false);
    }
    else
    {
        log->logTimer(LOGT_NONE);
        config->logTime = LOGT_NONE;
    }
}

void page_log::slot_intervalTime(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    if (enable)
    {
        log->logTimer(LOGT_INTERVAL);
        config->logTime = LOGT_INTERVAL;
        cbLogElapsedTime->blockSignals(true);
        cbLogElapsedTime->setChecked(false);
        cbLogElapsedTime->blockSignals(false);
    }
    else
    {
        log->logTimer(LOGT_NONE);
        config->logTime = LOGT_NONE;
    }
}

void page_log::slot_sizePlus()
{
    QFont font = ed->font();
    int size   = font.pointSize();
    font.setPointSize(++size);
    ed->setFont(font);
}

void page_log::slot_sizeMinus()
{
    QFont font = ed->font();
    int size   = font.pointSize();
    font.setPointSize(--size);
    ed->setFont(font);
}

AQScrollBar::AQScrollBar(page_log * plog)
{
    connect(this, &AQScrollBar::actionTriggered, plog, &page_log::slot_actionTriggered);
}
