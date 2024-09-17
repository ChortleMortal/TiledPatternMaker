#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStringList>
#include <QRadioButton>
#include <QLineEdit>
#include <QButtonGroup>

#include "gui/widgets/dlg_wlist_create.h"
#include "sys/enums/emotiftype.h"
#include "sys/sys.h"

DlgWorklistCreate::DlgWorklistCreate(QWidget * parent) :  QDialog(parent)
{
    setWindowTitle("Create Worklist");

    selMosaic     = new QRadioButton("Mosaic");
    selTiling     = new QRadioButton("Tiling");

    QButtonGroup * qbg1 = new QButtonGroup();
    qbg1->addButton(selMosaic);
    qbg1->addButton(selTiling);

    chkLoadFilter = new QCheckBox("Use load filter");
    radStyle      = new QRadioButton("Style");
    radMotif      = new QRadioButton("Motif");
    radText       = new QRadioButton("XML text");

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

    for (const auto & style : std::as_const(styles))
    {
        styleNames->addItem(style);
    }

    motifNames = new QComboBox();
    for (int i = 0; i <=  MAX_MOTIF_TYPE; i++)
    {
        motifNames->addItem(sMotifType[i]);
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
    grid->addWidget(radText, row, 0);
    grid->addWidget(text, row++, 1);
    grid->addWidget(radStyle, row, 0);
    grid->addWidget(styleNames, row++, 1);
    grid->addWidget(radMotif, row, 0);
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
    radMotif->setChecked(true);

    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &QDialog::accept);
}

QStringList DlgWorklistCreate::selectedMotifNames()
{
    eMotifType type = (eMotifType) motifNames->currentIndex();
    QStringList qls = Sys::XMLgetMotifNames(type);
    return qls;
}
