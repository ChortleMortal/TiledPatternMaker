#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>

#include "widgets/dlg_cleanse.h"
#include "geometry/map.h"
#include "mosaic/mosaic.h"

DlgCleanse::DlgCleanse(MosaicPtr mosaic, uint level, QWidget * parent) :  QDialog(parent)
{
    this->mosaic = mosaic;

    badV0       = new QCheckBox("Remove vertices with edge count 0");
    badV1       = new QCheckBox("Remove vertices with edge count 1");
    badEdges0   = new QCheckBox("Remove bad edges");
    joinEdges   = new QCheckBox("Join Colinear edges");
    divEdges    = new QCheckBox("Divide intersrecting edges");
    cleanNeigh  = new QCheckBox("Clean Neighbours");
    buildNeigh  = new QCheckBox("Build Neighbours");

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(badV0);
    vbox->addWidget(badV1);
    vbox->addWidget(badEdges0);
    vbox->addWidget(joinEdges);
    vbox->addWidget(divEdges);
    vbox->addWidget(cleanNeigh);
    vbox->addWidget(buildNeigh);

    QPushButton * anlBtn = new QPushButton("Analyse");
    QPushButton * canBtn = new QPushButton("Cancel");
    QPushButton * okBtn  = new QPushButton("OK");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(anlBtn);
    hbox->addStretch();
    hbox->addWidget(canBtn);
    hbox->addWidget(okBtn);
    vbox->addSpacing(13);
    vbox->addLayout(hbox);

    setLayout(vbox);

    setFixedWidth(300);

    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &QDialog::accept);
    connect(anlBtn, &QPushButton::clicked, this, &DlgCleanse::slot_analyse);

    toCheckboxes(level);
}

void DlgCleanse::toCheckboxes(uint level)
{
    if (level & badVertices_0)
        badV0->setChecked(true);
    if (level & badVertices_1)
        badV1->setChecked(true);
    if (level & badEdges)
        badEdges0->setChecked(true);
    if (level & joinupColinearEdges)
        joinEdges->setChecked(true);
    if (level & divideupIntersectingEdges)
        divEdges->setChecked(true);
    if (level & cleanupNeighbours)
        cleanNeigh->setChecked(true);
    if (level & buildNeighbours)
        buildNeigh->setChecked(true);
}

void DlgCleanse::slot_analyse()
{
    mosaic->resetProtoMaps();
    mosaic->setCleanseLevel(0);
    auto map = mosaic->getPrototypeMap();
    uint level = map->cleanseAnalysis();
    toCheckboxes(level);
}

uint DlgCleanse::getLevel()
{
    uint level = 0;
    if (badV0->isChecked())
        level |= badVertices_0;
    if (badV1->isChecked())
        level |= badVertices_1;
    if (badEdges0->isChecked())
        level |= badEdges;
    if (joinEdges->isChecked())
        level |= joinupColinearEdges;
    if (divEdges->isChecked())
        level |= divideupIntersectingEdges;
    if (cleanNeigh->isChecked())
        level |= cleanupNeighbours;
    if (buildNeigh->isChecked())
        level |= buildNeighbours;
    return level;
}
