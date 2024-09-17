#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

#include "gui/widgets/dlg_listnameselect.h"

DlgListNameSelect::DlgListNameSelect(VersionFileList &files, int version)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLabel * newLabel = new QLabel("Name");
    newEdit = new QLineEdit();
    newEdit->setMinimumWidth(301);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(newLabel);
    hbox->addWidget(newEdit);

    list = new LoaderListWidget();
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    if (version == 1)
    {
        cancelBtn->setText("Quit");
        okBtn->setText("Open");
    }

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(cancelBtn);
    hbox2->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(list);
    vbox->addLayout(hbox);
    vbox->addLayout(hbox2);
    setLayout(vbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);

    for (VersionedFile & file : files)
    {
        QListWidgetItem * item = new QListWidgetItem(file.getVersionedName().get());
        list->addItem(item);
    }
    list->setMinimumWidth(list->sizeHintForColumn(0) + 10);
    list->setMinimumHeight((list->sizeHintForRow(0) * files.size()) + 10);
    adjustSize();

    connect(list, &QListWidget::currentRowChanged, this, &DlgListNameSelect::slot_currentRow);
}

void DlgListNameSelect::slot_currentRow(int row)
{
    if (row == -1)
    {
        //newEdit->clear();
        return;
    }

    QListWidgetItem * item = list->item(row);
    newEdit->setText(item->text());
}
