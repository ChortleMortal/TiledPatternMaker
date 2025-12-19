#include <QCoreApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include <QTabWidget>

#include "dlg_textedit.h"
#include "sys/qt/qtapplog.h"
#include "sys/version.h"

LogWidget::LogWidget()
{
}

LogWidget::~LogWidget()
{
    unload();
    //ted.reset();
}

void LogWidget::set(TextEditPtr te)
{
    ted = te;

    unload();

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget(te.get());
    setLayout(vbox);
    adjustSize();
}

void LogWidget::unload()
{
    QLayout * l = layout();
    if (l)
    {
        QLayoutItem * item;
        while ( (item = l->itemAt(0)) != nullptr)
        {
            QWidget * w = item->widget();
            if (w)
            {
                w->setParent(nullptr);
            }
        }
        delete l;
    }
}

DlgTextEdit::DlgTextEdit(QWidget *parent) : QDialog(parent)
{
    vbox = new QVBoxLayout(this);
    vbox->addWidget(&txt);
    setLayout(vbox);

    setMinimumSize(900,600);

    txt.setAcceptRichText(false);
    txt.setLineWrapMode(QTextEdit::NoWrap);
    txt.setReadOnly(true);

    //const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    //txt.setCurrentFont(fixedFont);
}

void DlgTextEdit::set(const QVector<sTrapMsg> &msgs)
{
    for (auto & stm : std::as_const(msgs))
    {
        append(stm.msg);
    }
}

DlgMapVerify::DlgMapVerify(QString name, QWidget *parent) : DlgTextEdit(parent)
{
    setWindowTitle(name);
    qWarning() << "Map verify problem with:" << name;
}

DlgAbout::DlgAbout(QWidget * parent) : DlgTextEdit(parent)
{
    setMinimumSize(600,600);
    QString info = QString("%1 Version %2  Copyright (c) David A. Casper 2016-2025").arg(QCoreApplication::applicationName()).arg(tpmVersion);
    QLabel * line1 = new QLabel(info);
    QHBoxLayout * lay1 = new QHBoxLayout();
    lay1->addWidget(line1);

    QString lstring("TiledPatternMaker is free open soure software.\nYou can redistribute it and/or modify it under the terms of the GNU General Public License");
    QLabel * line2 = new QLabel(lstring);
    QHBoxLayout * lay2 = new QHBoxLayout();
    lay2->addWidget(line2);

    vbox->insertLayout(0,lay1);
    vbox->insertLayout(1,lay2);

    QString name(":/LICENSE");
    QFile data(name);
    if (data.open(QFile::ReadOnly))
    {
        QTextStream str(&data);
        while (!str.atEnd())
        {
            QString line = str.readLine();
            append(line);
        }
        data.close();
    }
    else
        qWarning() << "Coud not open" << name << data.errorString();

    QTextCursor  c = txt.textCursor();
    c.setPosition(0);
    txt.setTextCursor(c);
}

DlgStartOptions::DlgStartOptions(QWidget * parent) : DlgTextEdit(parent)
{
    setWindowTitle("TiledPatternMaker - Startup Options");

    QString name(":/txt/start-options.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        txt.setPlainText(in.readAll());
        data.close();
    }
    else
        qWarning() << "Coud not open" << name << data.errorString();
}

DlgSupport::DlgSupport(QWidget * parent) : DlgTextEdit(parent)
{
    setWindowTitle("TiledPatternMaker - Support");

    QString name(":/txt/support.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        txt.setPlainText(in.readAll());
        data.close();
    }
    else
        qWarning() << "Coud not open" << name << data.errorString();
}

DlgKbdOpts::DlgKbdOpts(QWidget * parent) : QDialog(parent)
{

    setWindowTitle("TiledPatternMaker -  Keyboard and Mouse operations");
    setMinimumSize(900,600);

    auto mosaic = new QTextEdit();
    {
        QString name(":/txt/mosaic.txt");
        QFile data(name);
        if (data.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&data);
            mosaic->setHtml(in.readAll());
            data.close();
        }
        else
            qWarning() << "Coud not open" << name << data.errorString();
        mosaic->adjustSize();
    }

    auto tiling = new QTextEdit();
    {
        QString name(":/txt/tilingmaker.txt");
        QFile data(name);
        if (data.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&data);
            tiling->setHtml(in.readAll());
            data.close();
        }
        else
            qWarning() << "Coud not open" << name << data.errorString();
        tiling->adjustSize();
    }

    auto maped = new QTextEdit();
    {
        QString name(":/txt/mapeditor.txt");
        QFile data(name);
        if (data.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&data);
            maped->setHtml(in.readAll());
            data.close();
        }
        else
            qWarning() << "Coud not open" << name << data.errorString();
        maped->adjustSize();
    }

    auto legacy = new QTextEdit();
    {
        QString name(":/txt/legacy.txt");
        QFile data(name);
        if (data.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&data);
            legacy->setHtml(in.readAll());
            data.close();
        }
        else
            qWarning() << "Coud not open" << name << data.errorString();
        legacy->adjustSize();
    }
    auto tabWidget = new QTabWidget;
    tabWidget->addTab(mosaic, "Mosaic Maker");
    tabWidget->addTab(tiling, "Tiling Maker");
    tabWidget->addTab(maped,  "Map Editor");
    tabWidget->addTab(legacy, "Legacy");
    tabWidget->setCurrentIndex(0);

    auto vbox = new QVBoxLayout;
    vbox->addWidget(tabWidget);

    this->setLayout(vbox);
}











