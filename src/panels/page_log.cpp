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

#include "panels/page_log.h"
#include "base/qtapplog.h"
#include "base/configuration.h"


page_log::page_log(ControlPanel * cpanel)  : panel_page(cpanel, "Log")
{
    QHBoxLayout * hbox = new QHBoxLayout();

    follow = new QCheckBox("Follow tail");
    follow->setChecked(true);
    hbox->addWidget(follow);

    QPushButton * btnCopyLog            = new QPushButton("Save Log");
    QCheckBox   * cbLogToStderr         = new QCheckBox("Log To stderr");
    QCheckBox   * cbLogToDisk           = new QCheckBox("Log To Disk");
    QCheckBox   * cbLogToPanel          = new QCheckBox("Log To Panel");
    QCheckBox   * cbLogWarningsOnly     = new QCheckBox("Warnings Only");
    QCheckBox   * cbLogNumberLines      = new QCheckBox("Number Lines");
                  cbLogElapsedTime      = new QCheckBox("Elapsed Time");
                  cbLogIntervalTime     = new QCheckBox("Interval Time");

    hbox->addWidget(btnCopyLog);
    hbox->addSpacing(43);
    hbox->addWidget(cbLogToDisk);
    hbox->addWidget(cbLogToStderr);
    hbox->addWidget(cbLogToPanel);
    hbox->addStretch();
    hbox->addWidget(cbLogWarningsOnly);
    hbox->addWidget(cbLogNumberLines);
    hbox->addWidget(cbLogIntervalTime);
    hbox->addWidget(cbLogElapsedTime);
    hbox->addStretch();

    vbox->addLayout(hbox);

    cbLogToStderr->setChecked(config->logToStderr);
    cbLogToDisk->setChecked(config->logToDisk);
    cbLogToPanel->setChecked(config->logToPanel);
    cbLogNumberLines->setChecked(config->logNumberLines);
    cbLogWarningsOnly->setChecked(config->logWarningsOnly);

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
    connect(cbLogWarningsOnly,&QCheckBox::clicked,    this,   &page_log::slot_warningsOnly);
    connect(cbLogNumberLines, &QCheckBox::clicked,    this,   &page_log::slot_numberLines);
    connect(cbLogElapsedTime, &QCheckBox::clicked,    this,   &page_log::slot_elapsedTime);
    connect(cbLogIntervalTime,&QCheckBox::clicked,    this,   &page_log::slot_intervalTime);
    connect(btnCopyLog,       &QPushButton::clicked,  this,   &page_log::slot_copyLog);

    ed = qtAppLog::getTextEditor();     // linkage to qtAppLog
    ed->setMinimumWidth(800);
    ed->setMinimumHeight(750);
    ed->setLineWrapMode(QTextEdit::NoWrap);
    ed->setReadOnly(true);

    const QFont font = ed->font();
    qInfo().noquote() << "log :" << font.toString();

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
    refreshPage();
}

void page_log::refreshPage()
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
    QString name = config->currentlyLoadedXML;
    Q_ASSERT(!name.contains(".xml"));

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

void page_log::slot_warningsOnly(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logWarningsOnly(enable);
    config->logWarningsOnly = enable;
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

AQScrollBar::AQScrollBar(page_log * plog)
{
    connect(this, &AQScrollBar::actionTriggered, plog, &page_log::slot_actionTriggered);
}
