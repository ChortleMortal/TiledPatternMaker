#include "dlg_listselect.h"

DlgListSelect::DlgListSelect(QStringList files)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    list = new LoaderListWidget();
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    QPushButton * okBtn = new QPushButton("OK");
    okBtn->setDefault(true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(cancelBtn);
    hbox->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(list);
    vbox->addLayout(hbox);
    setLayout(vbox);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);

    for (auto it = files.begin(); it != files.end(); it++)
    {
        QString file = *it;
        QListWidgetItem * item = new QListWidgetItem(file);
        list->addItem(item);
    }
    list->setMinimumWidth(list->sizeHintForColumn(0) + 10);
    list->setMinimumHeight((list->sizeHintForRow(0) * files.size()) + 10);
    adjustSize();

    connect(list, &QListWidget::currentRowChanged, this, &DlgListSelect::slot_currentRow);
}

void DlgListSelect::slot_currentRow(int row)
{
    if (row == -1)
    {
        selectedFile.clear();
        return;
    }

    QListWidgetItem * item = list->item(row);
    selectedFile = item->text();
}
