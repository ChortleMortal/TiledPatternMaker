#include <QPushButton>

#include "gui/widgets/dlg_trim.h"
#include "gui/widgets/layout_sliderset.h"

DlgTrim::DlgTrim(QWidget * parent) :  QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    trimmerX = new DoubleSpinSet("Trim X",0,-100,100);
    trimmerY = new DoubleSpinSet("TrimY ",0,-100,100);
    QPushButton * applyBtn = new QPushButton("Apply");
    QPushButton * doneBtn  = new QPushButton("Quit");

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(trimmerX);
    vbox->addLayout(trimmerY);
    vbox->addWidget(applyBtn);
    vbox->addWidget(doneBtn);

    setLayout(vbox);

    setFixedWidth(300);

    connect(doneBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(applyBtn,&QPushButton::clicked, this, &DlgTrim::slot_apply);
}

void DlgTrim::slot_ok()
{
    accept();
}

void DlgTrim::slot_apply()
{
    emit sig_apply(trimmerX->value(), trimmerY->value());
}
