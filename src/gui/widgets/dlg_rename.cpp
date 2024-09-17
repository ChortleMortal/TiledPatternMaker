#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QLineEdit>

#include "gui/widgets/dlg_rename.h"

DlgRename::DlgRename(QWidget * parent) : QDialog(parent)
{
    QGridLayout * grid = new QGridLayout();
    QLabel * oldLabel = new QLabel("Old Name");
    QLabel * newLabel = new QLabel("New Name");

    oldEdit = new QLineEdit();
    oldEdit->setReadOnly(true);
    newEdit = new QLineEdit();

    oldEdit->setMinimumWidth(301);
    newEdit->setMinimumWidth(301);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * renameBtn = new QPushButton("Rename");

    grid->addWidget(oldLabel,0,0);
    grid->addWidget(oldEdit,0,1);
    grid->addWidget(newLabel,1,0);
    grid->addWidget(newEdit,1,1);
    grid->addWidget(cancelBtn,2,0);
    grid->addWidget(renameBtn,2,1);
    setLayout(grid);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(renameBtn, &QPushButton::clicked, this, &QDialog::accept);

    renameBtn->setDefault(true);

    newEdit->setFocus();
}


void DlgRename::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}
