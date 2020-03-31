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


page_log::page_log(ControlPanel * cpanel)  : panel_page(cpanel, "Log")
{
    QHBoxLayout * hbox = new QHBoxLayout();

    follow = new QCheckBox("Follow tail");
    follow->setChecked(true);
    hbox->addWidget(follow);

    QPushButton * btnCopyLog = new QPushButton("Save Log");
    QCheckBox   * cbLogToStderr         = new QCheckBox("Log To stderr");
    QCheckBox   * cbLogToDisk           = new QCheckBox("Log To Disk");
    QCheckBox   * cbLogToPanel          = new QCheckBox("Log To Panel");
    QCheckBox   * cbLogNumberLines      = new QCheckBox("Number Lines");

    hbox->addWidget(btnCopyLog);
    hbox->addSpacing(43);
    hbox->addWidget(cbLogToDisk);
    hbox->addWidget(cbLogToStderr);
    hbox->addWidget(cbLogToPanel);
    hbox->addStretch();
    hbox->addWidget(cbLogNumberLines);
    hbox->addStretch();

    vbox->addLayout(hbox);

    cbLogToStderr->setChecked(config->logToStderr);
    cbLogToDisk->setChecked(config->logToDisk);
    cbLogToPanel->setChecked(config->logToPanel);
    cbLogNumberLines->setChecked(config->logNumberLines);

    connect(cbLogToStderr,    &QCheckBox::clicked,    this,   &page_log::slot_logToStdErr);
    connect(cbLogToDisk,      &QCheckBox::clicked,    this,   &page_log::slot_logToDisk);
    connect(cbLogToPanel,     &QCheckBox::clicked,    this,   &page_log::slot_logToPanel);
    connect(cbLogNumberLines, &QCheckBox::clicked,    this,   &page_log::slot_numberLines);
    connect(btnCopyLog,       &QPushButton::clicked,  this,   &page_log::slot_copyLog);

    ed = qtAppLog::getTextEditor();     // linkage to qtAppLog
    ed->setMinimumWidth(800);
    ed->setMinimumHeight(750);
    ed->setLineWrapMode(QTextEdit::NoWrap);
    ed->setReadOnly(true);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ed->setCurrentFont(fixedFont);

    vbox->addWidget(ed);

    sbar = new AQScrollBar(this);
    ed->setVerticalScrollBar(sbar);

    refreshPage();

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

    QString path;
#ifdef WIN32
    path = "C:/logs/";
#else
    path = "./logs/";
#endif

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

void page_log::slot_numberLines(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logLines(enable);
    config->logNumberLines = enable;
}

AQScrollBar::AQScrollBar(page_log * plog)
{
    connect(this, &AQScrollBar::actionTriggered, plog, &page_log::slot_actionTriggered);
}
