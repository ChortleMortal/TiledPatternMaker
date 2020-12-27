#include "dlg_textedit.h"
#include <QtWidgets>
#include "makers/decoration_maker/decoration_maker.h"
#include "base/mosaic.h"

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

    DecorationMaker * dm = DecorationMaker::getInstance();
    MosaicPtr mos = dm->getMosaic();
    if (mos)
        setWindowTitle(mos->getName());
}
