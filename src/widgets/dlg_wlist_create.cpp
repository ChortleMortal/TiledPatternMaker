#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStringList>

#include "widgets/dlg_wlist_create.h"

DlgWorklistCreate::DlgWorklistCreate(QWidget * parent) :  QDialog(parent), MosaicIOBase()
{
    chkLoadFilter = new QCheckBox("Load filter");
    chkStyle      = new QCheckBox("Style");
    chkMotif      = new QCheckBox("Motif");

    styleNames = new QComboBox();
    styleNames->setMinimumWidth(251);

    QStringList styles =
    {
     "style.Thick",
     "style.Filled",
     "style.Interlace",
     "style.Outline",
     "style.Plain",
     "style.Sketch",
     "style.Emboss",
     "style.TileColors",
     "designNotes",
     "design"
    };

    for (auto style : styles)
    {
        styleNames->addItem(style);
    }

    motifNames = new QComboBox();
    QStringList motifs  = motifRepresentation.values();
    for (auto name : motifs)
    {
        motifNames->addItem(name);
    }

    QGridLayout * grid = new QGridLayout();
    grid->addWidget(chkLoadFilter, 0, 0);
    grid->addWidget(chkStyle, 1, 0);
    grid->addWidget(styleNames, 1, 1);
    grid->addWidget(chkMotif, 2, 0);
    grid->addWidget(motifNames, 2, 1);

    QPushButton * okBtn  = new QPushButton("OK");
    QPushButton * canBtn = new QPushButton("Cancel");
    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(okBtn);
    hbox->addWidget(canBtn);


    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &QDialog::accept);
}
