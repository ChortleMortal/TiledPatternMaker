#include <QPushButton>
#include <QHBoxLayout>

#include "gui/model_editors/style_edit/border_editor.h"

BorderEditor::BorderEditor(StylePtr style, eStyleType user) : StyleEditor(style, user)
{
    QPushButton * pbEdit = new QPushButton("Edit in Border Maker");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(pbEdit);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();
    setLayout(vbox);

    connect(pbEdit, &QPushButton::clicked, this, &BorderEditor::sig_editBorder);
}
