#include "dlg_textedit.h"
#include <QtWidgets>
#include "base/configuration.h"

DlgTextEdit::DlgTextEdit(bool show)
{
    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget(&txt);
    setLayout(vbox);

    setMinimumSize(900,600);

    if (show)
        setAttribute(Qt::WA_DeleteOnClose);

    txt.setAcceptRichText(false);
    txt.setLineWrapMode(QTextEdit::NoWrap);
    txt.setReadOnly(true);

    //const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    //txt.setCurrentFont(fixedFont);

    Configuration * config = Configuration::getInstance();
    QString name;
    if (!config->currentlyLoadingXML.isEmpty())
        name = config->currentlyLoadingXML;
    else
        name = config->currentlyLoadedXML;
    setWindowTitle(name);
    qWarning() << "Problem with " << name;
}
