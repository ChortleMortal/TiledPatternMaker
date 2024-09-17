#include "version_dialog.h"

#include <QHBoxLayout>
#include <QPushButton>

VersionDialog::VersionDialog(QWidget *parent) : QDialog(parent)
{
    text1 = new QLabel;
    text2 = new QLabel;

    QPushButton * saveBtn   = new QPushButton("Save");
    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * bumpBtn   = new QPushButton("Bump");

    combo = new QComboBox;
    combo->addItem("Auto bump");
    combo->addItem("v1");
    combo->addItem("v2");
    combo->addItem("v3");
    combo->addItem("v4");
    combo->addItem("v5");
    combo->addItem("v6");
    combo->addItem("v7");
    combo->addItem("v8");
    combo->addItem("v9");
    combo->addItem("v10");

    QHBoxLayout * hbox = new  QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(saveBtn);
    hbox->addWidget(cancelBtn);
    hbox->addWidget(bumpBtn);
    hbox->addWidget(combo);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addSpacing(9);
    vbox->addWidget(text1);
    vbox->addSpacing(15);
    vbox->addWidget(text2);
    vbox->addSpacing(19);
    vbox->addLayout(hbox);
    vbox->addSpacing(9);

    setLayout(vbox);

    setText2("Do you want to bump version (Bump) or overwrite (Save)?");

    connect(saveBtn,   &QPushButton::clicked, this, [this] () { accept(); });
    connect(cancelBtn, &QPushButton::clicked, this, [this] () { reject(); });
    connect(bumpBtn,   &QPushButton::clicked, this, [this] () { done(2);  });
}
