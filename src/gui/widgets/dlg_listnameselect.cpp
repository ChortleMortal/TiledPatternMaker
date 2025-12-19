#include <QLabel>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>

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
        QString txt = file.getVersionedName().get() + "   ::   " + file.getPathDir();
        QListWidgetItem * item = new QListWidgetItem(txt);
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
    QString txt = item->text();
    QStringList qsl = txt.split("::");
    QString name = qsl[0].trimmed();
    newEdit->setText(name);
}
