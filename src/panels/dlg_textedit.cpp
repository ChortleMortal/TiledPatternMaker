#include "dlg_textedit.h"
#include "settings/configuration.h"
#include "viewers/view.h"

DlgTextEdit::DlgTextEdit(QWidget *parent) : QDialog(parent)
{
    vbox = new QVBoxLayout(this);
    vbox->addWidget(&txt);
    setLayout(vbox);

    setMinimumSize(900,600);

    setAttribute(Qt::WA_DeleteOnClose);

    txt.setAcceptRichText(false);
    txt.setLineWrapMode(QTextEdit::NoWrap);
    txt.setReadOnly(true);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    txt.setCurrentFont(fixedFont);
}

DlgTextEdit::~DlgTextEdit()
{
    //qDebug() << "DlgTextEdit destructor";
}

void DlgTextEdit::set(QStringList & qsl)
{
    for (auto line : qsl)
        append(line);
}

DlgMapVerify::DlgMapVerify(QString name, QWidget *parent) : DlgTextEdit(parent)
{
    QString dlgName;

    LoadUnit & loadUnit = View::getInstance()->getLoadUnit();
    if (loadUnit.loadTimer.isValid())
    {
        dlgName = loadUnit.name;
    }
    else
    {
        Configuration * config = Configuration::getInstance();
        dlgName = config->currentlyLoadedXML;
    }

    setWindowTitle(dlgName);
    qWarning() << "Map:" << name << "verify problem with:" << dlgName;
}

DlgMapVerify::~DlgMapVerify()
{
    //qDebug() << "DlgMapVerify destructor";
}

DlgLogView::DlgLogView(QString name, QWidget *parent) : DlgTextEdit(parent)
{
    QPushButton * saveBtn = new QPushButton("Save");
    connect(saveBtn, &QPushButton::clicked, this, &DlgLogView::slot_save);
    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(saveBtn);
    vbox->addLayout(hbox);

    txt.setReadOnly(false);

    QFile data(name);
    if (!data.open(QFile::ReadOnly))
    {
        qDebug() << "error opening" << name;
        close();
    }

    setWindowTitle(name);

    QTextStream str(&data);
    while (!str.atEnd())
    {
        QString line = str.readLine();

        if (line.contains("Debug"))
            txt.setTextColor(Qt::black);
        else  if (line.contains("Info"))
            txt.setTextColor(Qt::darkGreen);
        else  if (line.contains("Warning"))
            txt.setTextColor(Qt::darkRed);
        else
            txt.setTextColor(Qt::black);

        append(line);
    }
    data.close();

    QTextCursor  c = txt.textCursor();
    c.setPosition(0);
    txt.setTextCursor(c);
}

DlgLogView::~DlgLogView()
{
    //qDebug() << "DlgLogView destructor";
}

void DlgLogView::slot_save()
{
    QString nameList  = "Log Files (*.txt)";
    QString path      = qtAppLog::getInstance()->logDir();

    QString filename2 = QFileDialog::getSaveFileName(this, "Save log", path, nameList);
    if (filename2.isEmpty())
        return;

    qDebug() << "saving" << filename2;

    QFile data(filename2);
    if (!data.open(QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite))
    {
        qDebug() << "error opening" << filename2;
        return;
    }

    QTextStream str(&data);
    str << txt.toPlainText();

    data.close();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("OK");
    box.exec();
}
