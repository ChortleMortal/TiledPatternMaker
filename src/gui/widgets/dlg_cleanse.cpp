#include <QApplication>
#include <QMessageBox>

#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/mosaics/mosaic.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"

qreal QuicksetCleanse::rvals[16]  = {1e-1,1e-2,1e-3,1e-4,1e-5,1e-6,1e-7,1e-8,1e-9,1e-10,1e-11,1e-12,1e-13,1e-14,1e-15,1e-16};

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
    cleanEdges  = new QCheckBox("Clean edges");

    sen = new DoubleSpinSet("Sensitivity", sensitivity,1e-16, 0.5);
    sen->setSingleStep(1e-4);

    QPushButton * qset = new QPushButton("Qick set");

    connect(sen,  &DoubleSpinSet::valueChanged, this, &DlgCleanse::slot_mergeSensitivity);
    connect(qset, &QPushButton::clicked,        this, &DlgCleanse::slot_quickset);


    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addSpacing(20);
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
    vbox->addWidget(cleanEdges);

    status1 = new QLabel;
    status2 = new QLabel;
    vbox->addWidget(status1);
    vbox->addWidget(status2);

    QPushButton * anlBtn = new QPushButton("Analyse");
    QPushButton * clnBtn = new QPushButton("Cleanse");
    QPushButton * clrBtn = new QPushButton("Clear");
    QPushButton * canBtn = new QPushButton("Quit");
    QPushButton * okBtn  = new QPushButton("Apply");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(anlBtn);
    hbox->addWidget(clnBtn);
    hbox->addWidget(clrBtn);
    hbox->addStretch();
    hbox->addWidget(canBtn);
    hbox->addWidget(okBtn);

    vbox->addSpacing(13);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(canBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn,  &QPushButton::clicked, this, &QDialog::accept);
    connect(anlBtn, &QPushButton::clicked, this, &DlgCleanse::slot_analyse);
    connect(clnBtn, &QPushButton::clicked, this, &DlgCleanse::slot_cleanse);
    connect(clrBtn, &QPushButton::clicked, this, [this] { toCheckboxes(0);});

    toCheckboxes(level);

    if (map)
        setStatus(status1);
}

void DlgCleanse::toCheckboxes(uint level)
{
    badV0->setChecked(      level & badVertices_0);
    badV1->setChecked(      level & badVertices_1);
    badEdges0->setChecked(  level & coalesceEdges);
    nearPoints->setChecked( level & coalescePoints);
    joinEdges->setChecked(  level & joinupColinearEdges);
    divEdges->setChecked(   level & divideupIntersectingEdges);
    cleanEdges->setChecked( level & cleanupEdges);
}

uint DlgCleanse::fromCheckboxes()
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
    if (cleanEdges->isChecked())
        level |= cleanupEdges;
    return level;
}

void DlgCleanse::setStatus(QLabel * label)
{
    label->setText(map->info() + map->displayVertexEdgeCounts());
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

    setStatus(status1);

    // non-destructively analyse map using given sensitivity
    auto map2 = map->copy();
    MapCleanser mc(map2);
    uint level = mc.analyze(sensitivity);
    toCheckboxes(level);

    setStatus(status2);

    QApplication::restoreOverrideCursor();
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

    setStatus(status1);
    uint level = fromCheckboxes();
    MapCleanser mc(map);
    mc.cleanse(level,sensitivity);
    setStatus(status2);

    QApplication::restoreOverrideCursor();

    emit sig_cleansed();
}

void DlgCleanse::slot_mergeSensitivity(qreal r)
{
    sensitivity = r;
}

void DlgCleanse::slot_quickset()
{
    QuicksetCleanse qs(this,sensitivity);
    auto rv = qs.exec();
    if (rv == QDialog::Accepted)
    {
        qreal sens = qs.qs_sensitivity;
        sensitivity = sens;
        sen->setValue(sens);
        update();
    }
}

QuicksetCleanse::QuicksetCleanse(QWidget * parent, qreal existing) : QDialog(parent)
{
    qs_sensitivity = existing;
    int index      = 5;          // default

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

    for (int i=0; i < 16; i++)
    {
        if (rvals[i] == qs_sensitivity)
        {
            index = i;
            break;
        }
    }
    box->setCurrentIndex(index);

    connect(box, &QComboBox::currentIndexChanged, this, &QuicksetCleanse::slot_sensitivitySelected);

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

void QuicksetCleanse::slot_sensitivitySelected(uint sens)
{
    qreal r = 1e-2;
    if (sens < 16)
    {
        r = rvals[sens];
    }

    qs_sensitivity = r;
}
