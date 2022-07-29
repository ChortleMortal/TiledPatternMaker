#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "widgets/dlg_name.h"

DlgName::DlgName(QWidget * parent) : QDialog(parent)
{
    QGridLayout * grid = new QGridLayout();
    QLabel * newLabel = new QLabel("Name");

    newEdit = new QLineEdit();

    newEdit->setMinimumWidth(301);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    grid->addWidget(newLabel,0,0);
    grid->addWidget(newEdit,0,1);
    grid->addWidget(cancelBtn,1,0);
    grid->addWidget(okBtn,1,1);
    setLayout(grid);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}
