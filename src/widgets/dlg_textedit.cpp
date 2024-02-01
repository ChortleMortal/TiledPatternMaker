#include <QFontDatabase>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>

#include "dlg_textedit.h"
#include "settings/configuration.h"
#include "viewers/view_controller.h"
#include "panels/panel_misc.h"
#include "misc/sys.h"
#include "misc/version.h"

TextEditorWidget::TextEditorWidget()
{
    setContentsMargins(0,0,0,0);
}

TextEditorWidget::TextEditorWidget(QTextEdit * te)
{
    setFixedWidth(600);
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(te);
    setLayout(aLayout);
}

void TextEditorWidget::set(QTextEdit * te)
{
    ed = te;

    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(te);

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
    setLayout(aLayout);
    adjustSize();
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

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    txt.setCurrentFont(fixedFont);

    config = Configuration::getInstance();
}

DlgTextEdit::~DlgTextEdit()
{
    //qDebug() << "DlgTextEdit destructor";
}

void DlgTextEdit::set(const QStringList & qsl)
{
    for (auto & line : std::as_const(qsl))
    {
        append(line);
    }
}

DlgMapVerify::DlgMapVerify(QString name, QWidget *parent) : DlgTextEdit(parent)
{
    // FIXME - does this work with tiling maps

    LoadUnit & loadUnit = Sys::view->getLoadUnit();
    QString dlgName = loadUnit.getLoadName();

    setWindowTitle(dlgName);
    qWarning() << "Map:" << name << "verify problem with:" << dlgName;
}

DlgMapVerify::~DlgMapVerify()
{
    //qDebug() << "DlgMapVerify destructor";
}

DlgAbout::DlgAbout(QWidget * parent) : DlgTextEdit(parent)
{
    setMinimumSize(600,600);

    QString info = QString("%1 Version %2  Copyright (c) David A. Casper 2016-2023").arg(QCoreApplication::applicationName()).arg(tpmVersion);
    QLabel * line1 = new QLabel(info);
    QHBoxLayout * lay1 = new QHBoxLayout();
    lay1->addWidget(line1);

    QString lstring("TiledPatternMaker is free open soure software.\nYou can redistribute it and/or modify it under the terms of the GNU General Public License");
    QLabel * line2 = new QLabel(lstring);
    QHBoxLayout * lay2 = new QHBoxLayout();
    lay2->addWidget(line2);
    vbox->insertLayout(0,lay1);
    vbox->insertLayout(1,lay2);

    txt.setReadOnly(true);

    QString name("../LICENSE");
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
    {
        QString name = "./LICENSE";
        QFile data(name);
        QTextStream str(&data);
        if (data.open(QFile::ReadOnly))
        {
            while (!str.atEnd())
            {
                QString line = str.readLine();
                append(line);
            }
        }
        else
        {
            qWarning() << "Coud not open" << name;
            close();
        }
        data.close();
    }

    QTextCursor  c = txt.textCursor();
    c.setPosition(0);
    txt.setTextCursor(c);
}
