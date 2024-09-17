#include <QPushButton>

#include "gui/widgets/dlg_rebase.h"
#include "gui/widgets/layout_sliderset.h"

DlgRebase::DlgRebase(QWidget * parent) : QDialog(parent)
{
    QLabel * descrip  = new QLabel("Deletes all versions higher than new version");
    oldVersion = new SpinSet("Old version",0,0,99);
    oldVersion->setReadOnly(true);
    newVersion = new SpinSet("New Version",0,0,99);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * renameBtn = new QPushButton("Rebase");

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget(descrip);
    vbox->addLayout(oldVersion);
    vbox->addLayout(newVersion);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(cancelBtn);
    hbox->addWidget(renameBtn);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(renameBtn, &QPushButton::clicked, this, &QDialog::accept);

    renameBtn->setDefault(true);
    newVersion->setFocus();
}
