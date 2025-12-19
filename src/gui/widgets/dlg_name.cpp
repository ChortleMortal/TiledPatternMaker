#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "gui/widgets/dlg_name.h"

DlgName::DlgName(QWidget * parent) : QDialog(parent)
{
    QLabel * newLabel = new QLabel("Name");

    newEdit = new QLineEdit();
    newEdit->setMinimumWidth(301);

    QPushButton * cancelBtn = new QPushButton("Cancel");

    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(newLabel);
    hbox->addWidget(newEdit);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addStretch();
    hbox2->addWidget(cancelBtn);
    hbox2->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox2);

    setLayout(vbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);
}
