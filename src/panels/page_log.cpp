#include <QCheckBox>
#include <QTextEdit>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QFontDatabase>
#include <QApplication>

#include "panels/page_log.h"
#include "panels/controlpanel.h"
#include "widgets/dlg_textedit.h"
#include "misc/qtapplog.h"
#include "settings/configuration.h"
#include "misc/qtapplog.h"
#include "misc/sys.h"

page_log::page_log(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_LOG, "Log")
{
    viewingLog = true;

    savedText = new QTextEdit();
    savedText->setMinimumWidth(750);
    savedText->setMinimumHeight(690);
    savedText->setLineWrapMode(QTextEdit::NoWrap);
    savedText->setReadOnly(true);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    savedText->setFont(fixedFont);
    AQScrollBar * abar = new AQScrollBar(this);
    savedText->setVerticalScrollBar(abar);
    savedText->setStyleSheet("selection-color: rgb(170, 255, 0);  selection-background-color: rgb(255, 0, 0);");

    QHBoxLayout * hbox = new QHBoxLayout();

    follow = new QCheckBox("Follow tail");
    follow->setChecked(true);
    hbox->addWidget(follow);

    QPushButton * btnSaveLog    = new QPushButton("Save Log");
                  btnViewLog    = new QPushButton("View Saved");
    QPushButton * btnOptions    = new QPushButton("Options");
    QPushButton * sizePlus      = new QPushButton("+");
    QPushButton * sizeMinus     = new QPushButton("-");
    QPushButton * btnClear      = new QPushButton("Clear");
                  search        = new QLineEdit();
                  reverseSearch = new QCheckBox("Reverse:");
    QLabel      * sLabel        = new QLabel("Search:");

    sizePlus->setMaximumWidth(25);
    sizeMinus->setMaximumWidth(25);

    hbox->addWidget(btnSaveLog);
    hbox->addWidget(btnViewLog);
    hbox->addStretch();
    hbox->addWidget(sLabel);
    hbox->addWidget(search);
    hbox->addWidget(reverseSearch);
    hbox->addStretch();
    hbox->addWidget(btnClear);
    hbox->addWidget(btnOptions);
    hbox->addWidget(sizeMinus);
    hbox->addWidget(sizePlus);
    vbox->addLayout(hbox);

    connect(btnSaveLog,       &QPushButton::clicked,  this,   &page_log::slot_copyLog);
    connect(btnViewLog,       &QPushButton::clicked,  this,   &page_log::slot_viewLog);
    connect(btnOptions,       &QPushButton::clicked,  this,   &page_log::slot_options);
    connect(sizePlus,         &QPushButton::clicked,  this,   &page_log::slot_sizePlus);
    connect(sizeMinus,        &QPushButton::clicked,  this,   &page_log::slot_sizeMinus);
    connect(btnClear,         &QPushButton::clicked,  this,  [] { qtAppLog::getInstance()->clear(); });
    connect(search,           &QLineEdit::returnPressed, this,&page_log::slot_search);

    logText = qtAppLog::getTextEditor();     // linkage to qtAppLog
    logText->setMinimumWidth(750);
    logText->setMinimumHeight(690);
    logText->setLineWrapMode(QTextEdit::NoWrap);
    logText->setReadOnly(true);
    logText->setStyleSheet("selection-color: rgb(170, 255, 0);  selection-background-color: rgb(255, 0, 0);");

    const QFont font = logText->font();
    qInfo().noquote() << "Log font:" << font.toString();

    tew = new TextEditorWidget();
    vbox->addWidget(tew);
    vbox->addStretch();

    tew->set(logText);

    AQScrollBar * sbar = new AQScrollBar(this);
    logText->setVerticalScrollBar(sbar);

    QTextCursor cursor = logText->textCursor();
    cursor.movePosition(QTextCursor::End);
    logText->setTextCursor(cursor);
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
    QTextCursor cursor = logText->textCursor();
    cursor.movePosition(QTextCursor::End);
    logText->setTextCursor(cursor);
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
    QString mosaic = config->lastLoadedMosaic;
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
    if (viewingLog)
        viewSaved();
    else
        viewLog();
}

void page_log::viewSaved()
{
    qDebug() << "view saved - start";

    QString path      = qtAppLog::getInstance()->logDir();
    QString nameList  = "Log Files (*.txt)";

    QString filename = QFileDialog::getOpenFileName(this, "Load log", path, nameList);

    if (filename.isEmpty())
        return;

    QFile data(filename);
    if (!data.open(QFile::ReadOnly))
    {
        qDebug() << "error opening" << filename;
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    viewingLog = false;

    // populate
    savedText->clear();
    QTextCursor  c = savedText->textCursor();
    c.setPosition(0);
    savedText->setTextCursor(c);

    QTextStream str(&data);
    while (!str.atEnd())
    {
        QString line = str.readLine();
        if (Sys::isDarkTheme)
        {
            if (line.contains("Debug"))
                savedText->setTextColor(Qt::white);
            else  if (line.contains("Info"))
                savedText->setTextColor(Qt::green);
            else  if (line.contains("Warning"))
                savedText->setTextColor(Qt::red);
            else
                savedText->setTextColor(Qt::white);
        }
        else
        {
            if (line.contains("Debug"))
                savedText->setTextColor(Qt::black);
            else  if (line.contains("Info"))
                savedText->setTextColor(Qt::darkGreen);
            else  if (line.contains("Warning"))
                savedText->setTextColor(Qt::darkRed);
            else
                savedText->setTextColor(Qt::black);
        }

        savedText->append(line);
    }
    data.close();

    c = savedText->textCursor();
    c.setPosition(0);
    savedText->setTextCursor(c);

    // use it
    tew->set(savedText);
    repaint();

    QApplication::restoreOverrideCursor();

    btnViewLog->setText("View Log");

    qDebug() << "view saved - end";
}

void page_log::viewLog()
{
    qDebug() << "view log - start";
    viewingLog = true;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tew->set(logText);

    QTextCursor  c = logText->textCursor();
    c.setPosition(0);
    logText->setTextCursor(c);

    repaint();

    QApplication::restoreOverrideCursor();

    btnViewLog->setText("View Saved");

    qDebug() << "view the log - end";
}

void page_log::slot_sizePlus()
{
    auto ed   = tew->get();
    auto font = ed->font();
    int size  = font.pointSize();
    font.setPointSize(++size);
    ed->setFont(font);
}

void page_log::slot_sizeMinus()
{
    auto ed   = tew->get();
    auto font = ed->font();
    int size  = font.pointSize();
    font.setPointSize(--size);
    ed->setFont(font);
}

void page_log::slot_options()
{
    LogOptionsDlg dlg(this);
    dlg.exec();
}

void page_log::slot_search()
{
    QString sstr = search->text();
    if (sstr.isEmpty())
    {
        return;
    }

    auto ted = tew->get();
    if (!reverseSearch->isChecked())
    {
        ted->find(sstr);
    }
    else
    {
        ted->find(sstr,QTextDocument::FindBackward);
    }
}

/////////////////////////////////////////////////
/// \brief AQScrollBar::AQScrollBar
/// \param plog
///
//////////////////////////////////////////////////

AQScrollBar::AQScrollBar(page_log * plog)
{
    connect(this, &AQScrollBar::actionTriggered, plog, &page_log::slot_actionTriggered);
}

///////////////////////////////////////////////////
/// \brief LogOptionsDlg::LogOptionsDlg
/// \param parent
///
///////////////////////////////////////////////////

LogOptionsDlg::LogOptionsDlg(QWidget * parent) : QDialog(parent)
{
    config = Configuration::getInstance();

    setWindowTitle("Log Options");

    QCheckBox   * cbLogToStderr         = new QCheckBox("Log To stderr");
    QCheckBox   * cbLogToDisk           = new QCheckBox("Log To Disk");
    QCheckBox   * cbLogToPanel          = new QCheckBox("Log To Panel");
    QCheckBox   * cbLogDebug            = new QCheckBox("Log Debug");
    QCheckBox   * cbLogNumberLines      = new QCheckBox("Number Lines");
                  cbLogElapsedTime      = new QCheckBox("Elapsed Time");
                  cbLogIntervalTime     = new QCheckBox("Interval Time");
    QPushButton * pbOK                  = new QPushButton("OK");

    QHBoxLayout * hbox1 = new QHBoxLayout();
    hbox1->addWidget(cbLogToDisk);
    hbox1->addWidget(cbLogToStderr);
    hbox1->addWidget(cbLogToPanel);
    hbox1->addWidget(cbLogDebug);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(cbLogNumberLines);
    hbox2->addWidget(cbLogIntervalTime);
    hbox2->addWidget(cbLogElapsedTime);
    hbox2->addStretch();

    QHBoxLayout * hbox3 = new QHBoxLayout();
    hbox3->addStretch();
    hbox3->addWidget(pbOK);

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addLayout(hbox3);

    setLayout(vbox);

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

    connect(cbLogToStderr,    &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_logToStdErr);
    connect(cbLogToDisk,      &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_logToDisk);
    connect(cbLogToPanel,     &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_logToPanel);
    connect(cbLogDebug,       &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_logDebug);
    connect(cbLogNumberLines, &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_numberLines);
    connect(cbLogElapsedTime, &QCheckBox::clicked,    this,   &LogOptionsDlg::slot_elapsedTime);
    connect(cbLogIntervalTime,&QCheckBox::clicked,    this,   &LogOptionsDlg::slot_intervalTime);
    connect(pbOK,             &QPushButton::clicked,  this,   [this] { accept(); });
}

void LogOptionsDlg::slot_logToStdErr(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logToStdErr(enable);
    config->logToStderr = enable;
}

void LogOptionsDlg::slot_logToDisk(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logToDisk(enable);
    config->logToDisk = enable;
}

void LogOptionsDlg::slot_logToPanel(bool enable)
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

void LogOptionsDlg::slot_logDebug(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logDebug(enable);
    config->logDebug = enable;
}

void LogOptionsDlg::slot_numberLines(bool enable)
{
    qtAppLog * log = qtAppLog::getInstance();
    log->logLines(enable);
    config->logNumberLines = enable;
}

void LogOptionsDlg::slot_elapsedTime(bool enable)
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

void LogOptionsDlg::slot_intervalTime(bool enable)
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
