#include <QRadioButton>
#include <QPushButton>
#include <QGridLayout>

#include "widgets/dlg_push_select.h"

DlgPushSelect::DlgPushSelect(QWidget *parent) : QDialog(parent)
{
    retval = 0;

    rbMosaic = new QRadioButton("Push to Mosaic");
    rbMotif  = new QRadioButton("Push to Motif");
    rbTiling = new QRadioButton("Push to Tiling");

    QPushButton * pbSelect = new QPushButton("Select");
    QPushButton * pbCancel = new QPushButton("Cancel");

    QGridLayout * grid = new QGridLayout;
    grid->addWidget(rbMosaic,0,0);
    grid->addWidget(rbMotif, 0,1);
    grid->addWidget(rbTiling,0,2);

    grid->addWidget(pbSelect,2,1);
    grid->addWidget(pbCancel,2,2);

    setLayout(grid);

    connect(pbCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(pbSelect, &QPushButton::clicked, this, &DlgPushSelect::select);
}

void DlgPushSelect::select()
{
    if (rbMosaic->isChecked())
        retval = 1;
    else if (rbMotif->isChecked())
        retval = 2;
    else if (rbTiling->isChecked())
        retval = 3;

    if (retval > 0)
        accept();
}
