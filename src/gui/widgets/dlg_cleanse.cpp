#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMessageBox>

#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/layout_sliderset.h"
#include "sys/geometry/map.h"
#include "model/mosaics/mosaic.h"

DlgCleanse::DlgCleanse(MapPtr map, uint level, qreal sensitivity, QWidget * parent) :  QDialog(parent)
{
    this->map = map;
    this->sensitivity = sensitivity;

    badV0       = new QCheckBox("Remove vertices with edge count 0");
    badV1       = new QCheckBox("Remove vertices with edge count 1");
    badEdges0   = new QCheckBox("Coalesce edges");
    nearPoints  = new QCheckBox("Coalesce vertices");
    joinEdges   = new QCheckBox("Join Colinear edges");
    divEdges    = new QCheckBox("Divide intersrecting edges");
    cleanNeigh  = new QCheckBox("Clean Neighbours");
    buildNeigh  = new QCheckBox("Build Neighbours");

    sen = new DoubleSpinSet("Sensitivity", sensitivity,1e-16, 0.5);
    sen->setSingleStep(1e-4);

    QPushButton * qset = new QPushButton("Qick set");

    connect(sen,  &DoubleSpinSet::valueChanged, this, &DlgCleanse::slot_mergeSensitivity);
    connect(qset, &QPushButton::clicked,        this, &DlgCleanse::slot_quickset);


    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addStretch();
    hbox1->addLayout(sen);
    hbox1->addWidget(qset);
    hbox1->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(badV0);
    vbox->addWidget(badV1);
    vbox->addWidget(nearPoints);
    vbox->addLayout(hbox1);
    vbox->addWidget(badEdges0);
    vbox->addWidget(joinEdges);
    vbox->addWidget(divEdges);
    vbox->addWidget(cleanNeigh);
    vbox->addWidget(buildNeigh);

    status  = new QLabel;
    status2 = new QLabel;
    vbox->addWidget(status);
    vbox->addWidget(status2);

    QPushButton * anlBtn = new QPushButton("Analyse");
    QPushButton * clnBtn = new QPushButton("Cleanse");
    QPushButton * canBtn = new QPushButton("Cancel");
    QPushButton * okBtn  = new QPushButton("OK");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(anlBtn);
    hbox->addWidget(clnBtn);
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
    connect(clnBtn, &QPushButton::clicked, this, &DlgCleanse::slot_cleanse);

    toCheckboxes(level);

    if (map)
        status->setText(map->info());
}

void DlgCleanse::toCheckboxes(uint level)
{
    badV0->setChecked(      level & badVertices_0);
    badV1->setChecked(      level & badVertices_1);
    badEdges0->setChecked(  level & coalesceEdges);
    nearPoints->setChecked( level & coalescePoints);
    joinEdges->setChecked(  level & joinupColinearEdges);
    divEdges->setChecked(   level & divideupIntersectingEdges);
    cleanNeigh->setChecked( level & cleanupNeighbours);
    buildNeigh->setChecked( level & buildNeighbours);
}

void DlgCleanse::slot_analyse()
{
    if (!map)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Analysis requires a map");
        box.exec();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    status->setText(map->info());
    uint level = map->cleanseAnalysis(sensitivity);
    toCheckboxes(level);
    status2->setText(map->info());

    QApplication::restoreOverrideCursor();

    emit sig_cleansed();
}

void DlgCleanse::slot_cleanse()
{
    if (!map)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Cleanse requires a map");
        box.exec();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    status->setText(map->info());
    uint level = getLevel();
    map->cleanse(level,sensitivity);
    status2->setText(map->info());

    QApplication::restoreOverrideCursor();

    emit sig_cleansed();
}

uint DlgCleanse::getLevel()
{
    uint level = 0;
    if (badV0->isChecked())
        level |= badVertices_0;
    if (badV1->isChecked())
        level |= badVertices_1;
    if (nearPoints->isChecked())
        level |= coalescePoints;
    if (badEdges0->isChecked())
        level |= coalesceEdges;
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

void DlgCleanse::slot_mergeSensitivity(qreal r)
{
    sensitivity = r;
}

void DlgCleanse::slot_quickset()
{
    QuicksetCleanse qs(this);
    auto rv = qs.exec();
    if (rv == QDialog::Accepted)
    {
        qreal sens = qs.sensitivity;
        sensitivity = sens;
        sen->setValue(sens);
    }
}

QuicksetCleanse::QuicksetCleanse(QWidget * parent) : QDialog(parent)
{
    sensitivity = 1e-1;

    box = new QComboBox();
    box->addItem("1e-1");
    box->addItem("1e-2");
    box->addItem("1e-3");
    box->addItem("1e-4");
    box->addItem("1e-5");
    box->addItem("1e-6");
    box->addItem("1e-7");
    box->addItem("1e-8");
    box->addItem("1e-9");
    box->addItem("1e-10");
    box->addItem("1e-11");
    box->addItem("1e-12");
    box->addItem("1e-13");
    box->addItem("1e-14");
    box->addItem("1e-15");
    box->addItem("1e-16");

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    connect(box, &QComboBox::currentIndexChanged, this, &QuicksetCleanse::slot_mergeSensitivity);
#else
    connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QuicksetCleanse::slot_mergeSensitivity);
#endif

    QPushButton * canBtn = new QPushButton("Cancel");
    QPushButton * okBtn  = new QPushButton("OK");

    connect(okBtn,  &QPushButton::clicked, this, [this] { accept(); });
    connect(canBtn, &QPushButton::clicked, this, [this] { reject(); });

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(canBtn);
    hbox->addWidget(okBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(box);
    vbox->addSpacing(15);
    vbox->addLayout(hbox);

    setLayout(vbox);
}

void QuicksetCleanse::slot_mergeSensitivity(int sens)
{
    static qreal rvals[16] = {1e-1,1e-2,1e-3,1e-4,1e-5,1e-6,1e-7,1e-8,1e-9,1e-10,1e-11,1e-12,1e-13,1e-14,1e-15,1e-16};

    qreal r = 1e-2;
    if (sens < 16)
    {
        r = rvals[sens];
    }

    sensitivity = r;
}
