#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStringList>
#include <QRadioButton>
#include <QLineEdit>

#include "widgets/dlg_wlist_create.h"

DlgWorklistCreate::DlgWorklistCreate(QWidget * parent) :  QDialog(parent), MosaicIOBase()
{
    setWindowTitle("Create Worklist");

    selMosaic     = new QRadioButton("Mosaic");
    selTiling     = new QRadioButton("Tiling");

    chkLoadFilter = new QCheckBox("Load filter");
    chkStyle      = new QCheckBox("Style");
    chkMotif      = new QCheckBox("Motif");
    chkText       = new QCheckBox("XML text");

    text          = new QLineEdit;

    styleNames    = new QComboBox();
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

    for (const auto & style : styles)
    {
        styleNames->addItem(style);
    }

    motifNames = new QComboBox();
    QStringList motifs  = motifRepresentation.values();
    for (const auto & name : motifs)
    {
        motifNames->addItem(name);
    }

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(selMosaic);
    hbox->addSpacing(7);
    hbox->addWidget(selTiling);
    hbox->addStretch();

    int row = 0;
    QGridLayout * grid = new QGridLayout();
    grid->addLayout(hbox, row++, 1);
    grid->addWidget(chkLoadFilter, row++, 0);
    grid->addWidget(chkText, row, 0);
    grid->addWidget(text, row++, 1);
    grid->addWidget(chkStyle, row, 0);
    grid->addWidget(styleNames, row++, 1);
    grid->addWidget(chkMotif, row, 0);
    grid->addWidget(motifNames, row++, 1);

    QPushButton * okBtn  = new QPushButton("OK");
    QPushButton * canBtn = new QPushButton("Cancel");
    hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(okBtn);
    hbox->addWidget(canBtn);


    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(grid);
    vbox->addLayout(hbox);

    setLayout(vbox);

    selMosaic->setChecked(true);

    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &QDialog::accept);
}
