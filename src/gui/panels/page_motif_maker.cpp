#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QComboBox>
#include <QFile>

#include "gui/map_editor/map_editor.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/model_editors/motif_edit/tile_motif_button.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/widgets/dlg_listnameselect.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/panel_misc.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/motifs/adaptive_motif.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/motif.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tile_writer.h"
#include "model/tilings/tiling.h"
#include "sys/enums/erender.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/sys/fileservices.h"

using std::make_shared;
using Qt::endl;

page_motif_maker::page_motif_maker(ControlPanel * cpanel) : panel_page(cpanel,PAGE_MOTIF_MAKER,"Motif Maker")
{
    setPageStatusString();

    AQPushButton * pbEnlarged = new AQPushButton("Single");
    AQPushButton * pbSelected = new AQPushButton("Selected");
    AQPushButton * pbTileUnit = new AQPushButton("Unit");
    pbEnlarged->setMinimumWidth(10);
    pbSelected->setMinimumWidth(10);
    pbTileUnit->setMinimumWidth(10);

    QButtonGroup * bgrp    = new QButtonGroup();
    bgrp->setExclusive(true);
    bgrp->addButton(pbEnlarged);
    bgrp->addButton(pbSelected);
    bgrp->addButton(pbTileUnit);

    smxWidget  = new SMXWidget(Sys::motifMakerView.get(),true,true);
    smxWidget->setObjectName("abc");
    smxWidget->setStyleSheet("#abc {border: 1px solid black; }");

    SpinSet * widthSpin = new SpinSet("Line Width",3,1,9);
    widthSpin->setValue((int)config->motifViewWidth);

    QPushButton * pbDup      = new QPushButton("Duplicate");
    QPushButton * pbDel      = new QPushButton("Delete");
    QPushButton * pbEdit     = new QPushButton("Edit Map");
    QPushButton * pbCombine  = new QPushButton("Combine");
    QPushButton * pbRender   = new QPushButton("Render");
    QPushButton * pbSave     = new QPushButton("Save as tile");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QCheckBox   * pbPropagate = new QCheckBox("Propagate");
    pbPropagate->setChecked(true);
    connect(pbPropagate, &QCheckBox::clicked, motifMaker, &MotifMaker::slot_propagateChanged);

    protoLabel              = new QLabel("Tilings");
    prototypeCombo          = new QComboBox();
    prototypeCombo->setMinimumWidth(131);

    QHBoxLayout * hbox      = new QHBoxLayout;
    hbox->addWidget(protoLabel);
    hbox->addWidget(prototypeCombo);
    hbox->addWidget(pbEnlarged);
    hbox->addWidget(pbSelected);
    hbox->addWidget(pbTileUnit);
    hbox->addWidget(smxWidget);
    hbox->addLayout(widthSpin);

    vbox->addLayout(hbox);

    if (config->insightMode)
    {
        QCheckBox * hiliteUnit   = new QCheckBox("Highlight Unit");
                    replicate    = new QCheckBox("Radial Replicate");
        QCheckBox * showMotif    = new QCheckBox("Motif");
        QCheckBox * showTile     = new QCheckBox("Tile");
        QCheckBox * showMotifB   = new QCheckBox("Motif");
        QCheckBox * showTileC    = new QCheckBox("Tile");
        QCheckBox * showMotifC   = new QCheckBox("Motif");

        QCheckBox * showExt      = new QCheckBox("Extended");
        QFrame    * line1        = new QFrame();
        line1->setFrameShape(QFrame::VLine);
        line1->setFrameShadow(QFrame::Sunken);
        line1->setLineWidth(1);
        QFrame    * line2        = new QFrame();
        line2->setFrameShape(QFrame::VLine);
        line2->setFrameShadow(QFrame::Sunken);
        line2->setLineWidth(1);
        QFrame    * line3        = new QFrame();
        line3->setFrameShape(QFrame::VLine);
        line3->setFrameShadow(QFrame::Sunken);
        line3->setLineWidth(1);

        QLabel    * l_bounds     = new QLabel("Boundaries:");
        QLabel    * l_cents      = new QLabel("Centres:");

        hbox = new QHBoxLayout;
        hbox->addWidget(showMotif);
        hbox->addWidget(showTile);
        hbox->addWidget(line1);
        hbox->addWidget(l_bounds);
        hbox->addWidget(showExt);
        hbox->addWidget(showMotifB);
        hbox->addWidget(line2);
        hbox->addWidget(l_cents);
        hbox->addWidget(showTileC);
        hbox->addWidget(showMotifC);
        hbox->addWidget(line3);
        hbox->addStretch();
        hbox->addWidget(hiliteUnit);
        hbox->addWidget(replicate);
        vbox->addLayout(hbox);

        replicate->setChecked(!Sys::dontReplicate);
        hiliteUnit->setChecked(Sys::highlightUnit);
        showTile->setChecked(config->showTileBoundary);
        showMotifB->setChecked(config->showMotifBoundary);
        showMotif->setChecked(config->showMotif);
        showExt->setChecked(config->showExtendedBoundary);
        showTileC->setChecked(config->showTileCenter);
        showMotifC->setChecked(config->showMotifCenter);

        connect(replicate,          &QCheckBox::clicked, this,           &page_motif_maker::replicateClicked);
        connect(Sys::systemViewController->getView(), &SystemView::sig_rebuildMotif, this, &page_motif_maker::slot_rebuildMotif);
        connect(hiliteUnit,         &QCheckBox::clicked, this, [this](bool checked) { Sys::highlightUnit           = checked; emit sig_updateView(); } );
        connect(showTile,           &QCheckBox::clicked, this, [this](bool checked) { config->showTileBoundary     = checked; emit sig_updateView(); } );
        connect(showMotifB,         &QCheckBox::clicked, this, [this](bool checked) { config->showMotifBoundary    = checked; emit sig_updateView(); } );
        connect(showMotif,          &QCheckBox::clicked, this, [this](bool checked) { config->showMotif            = checked; emit sig_updateView(); } );
        connect(showExt,            &QCheckBox::clicked, this, [this](bool checked) { config->showExtendedBoundary = checked; emit sig_updateView(); } );
        connect(showTileC,          &QCheckBox::clicked, this, [this](bool checked) { config->showTileCenter       = checked; emit sig_updateView(); } );
        connect(showMotifC,         &QCheckBox::clicked, this, [this](bool checked) { config->showMotifCenter      = checked; emit sig_updateView(); } );
    }

    hbox = new QHBoxLayout;
    hbox->addWidget(pbCombine);
    hbox->addWidget(pbDel);
    hbox->addWidget(pbDup);
    hbox->addWidget(pbEdit);
    hbox->addWidget(pbSave);
    hbox->addWidget(pbRender);
    hbox->addWidget(pbPropagate);

    vbox->addLayout(hbox);

    // the Motif maker widget
    motifMakerWidget  =  new MotifMakerWidget();

    // putting it together
    vbox->addSpacing(7);
    vbox->addWidget(motifMakerWidget);
    vbox->addStretch();

    switch (config->motifMakerViewType)
    {
    case MotifMakerView::ViewType::Single:
        pbEnlarged->setChecked(true);
        break;
    case MotifMakerView::ViewType::Multiple:
        pbSelected->setChecked(true);
        break;
    case MotifMakerView::ViewType::All:
        pbTileUnit->setChecked(true);
        break;
    case MotifMakerView::ViewType::Undefined:
        break;
    }

    connect(pbRender,           &QPushButton::clicked,              this, [] {Sys::render(RENDER_RESET_MOTIFS);} );
    connect(pbDup,              &QPushButton::clicked,              this,   &page_motif_maker::slot_duplicateCurrent);
    connect(pbDel,              &QPushButton::clicked,              this,   &page_motif_maker::slot_deleteCurrent);
    connect(pbEdit,             &QPushButton::clicked,              this,   &page_motif_maker::slot_editCurrent);
    connect(pbCombine,          &QPushButton::clicked,              this,   &page_motif_maker::slot_combine);
    connect(pbSave,             &QPushButton::clicked,              this,   &page_motif_maker::slot_saveAsTile);
    connect(pbEnlarged,         &QPushButton::toggled,              this,   &page_motif_maker::enlargedClicked);
    connect(pbSelected,         &QPushButton::toggled,              this,   &page_motif_maker::selectedClicked);
    connect(pbTileUnit,         &QPushButton::toggled,              this,   &page_motif_maker::tileUnitClicked);
    connect(prototypeCombo,     &QComboBox::currentIndexChanged,    this,   &page_motif_maker::slot_prototypeSelected);
    connect(widthSpin,          &SpinSet::valueChanged,             this,   &page_motif_maker::slot_widthChanged);
}

void page_motif_maker::onEnter()
{
    setPageStatus();
    motifMakerWidget->scrollToDelegated();
}

void page_motif_maker::onExit()
{
    clearPageStatus();
}

void page_motif_maker::onRefresh(void)
{
    if (protosHaveChanged())
    {
        loadProtoCombo();
    }

    if (config->insightMode)
    {
        replicate->blockSignals(true);
        replicate->setChecked(!Sys::dontReplicate);
        replicate->blockSignals(false);
    }

    smxWidget->refresh();

    motifMakerWidget->refresh();    // the main refresh of the page
}

bool page_motif_maker::protosHaveChanged()
{
    bool changed = false;

    QVector<ProtoPtr> loadedProtos;

    for (int i=0; i < prototypeCombo->count(); i++)
    {
        auto v = prototypeCombo->itemData(i);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        auto proto  = wproto.lock();
        if (proto)
        {
            loadedProtos.push_back(proto);
        }
    }

    QVector<ProtoPtr> currentProtos;
    for (auto & proto : motifMaker->protoData.getPrototypes())
    {
        if (proto->getTiling())    // the combo only ever holds tiling-based prototypes, see loadProtoCombo()
            currentProtos.push_back(proto);
    }

    if (loadedProtos.count() != currentProtos.count())
    {
        changed = true;
    }
    else
    {
        for (auto & current : currentProtos)
        {
            if (!loadedProtos.contains(current))
            {
                changed = true;
                break;
            }
        }
    }
    return changed;

}

void page_motif_maker::loadProtoCombo()
{
    // rebuild the combo - this does not affect anything except the axctual selection of a proto
    // detection pf this change is made in the motif maker widget

    prototypeCombo->blockSignals(true);

    prototypeCombo->clear();

    const QVector<ProtoPtr> & currentProtos = motifMaker->protoData.getPrototypes();
    for (auto & proto : std::as_const(currentProtos))
    {
        if (!proto->getTiling())
            continue;   // tiling-less (explicit map) prototype - nothing for the Motif Maker to edit
        prototypeCombo->addItem(proto->getTiling()->getVName().get(),QVariant::fromValue(WeakProtoPtr(proto)));
    }

    protoLabel->setText(QString("Tilings (%1)").arg(prototypeCombo->count()));

    if (prototypeCombo->count() == 0)
    {
        return;
    }

    ProtoPtr selected = motifMaker->motifData.getActivePrototype();
    if (selected && selected->getTiling())
    {
        QString name = selected->getTiling()->getVName().get();
        qDebug() << "page_motif_maker::reload() selected tiling:" << name;
        int index = prototypeCombo->findText(name);
        if (index >= 0)
        {
            prototypeCombo->setCurrentIndex(index);
        }
        else
        {
            prototypeCombo->setCurrentIndex(0);
            auto v = prototypeCombo->itemData(0);
            WeakProtoPtr wproto = v.value<WeakProtoPtr>();
            auto proto = wproto.lock();
            motifMaker->motifData.usePrototype(proto);
        }
    }
    else
    {
        prototypeCombo->setCurrentIndex(0);
        auto v = prototypeCombo->itemData(0);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        auto proto = wproto.lock();
        motifMaker->motifData.usePrototype(proto);
    }

    prototypeCombo->blockSignals(false);
}

void  page_motif_maker::replicateClicked(bool state)
{
    Sys::dontReplicate = !state;
    slot_rebuildMotif();
}

void page_motif_maker::slot_rebuildMotif()
{
    // this is triggered when toggling debug of motif or unusually when regularity is changed
    motifMaker->motifData.rebuildCurrentMotif();
    emit sig_reconstructView();
}

void  page_motif_maker::enlargedClicked()
{
    motifMakerWidget->setupViewMode(MotifMakerView::ViewType::Single);
}

void  page_motif_maker::selectedClicked()
{
    motifMakerWidget->setupViewMode(MotifMakerView::ViewType::Multiple);
}

void  page_motif_maker::tileUnitClicked()
{
    motifMakerWidget->setupViewMode(MotifMakerView::ViewType::All);
}

void page_motif_maker::slot_editCurrent()
{
    if (Sys::mapEditor->loadSelectedMotifs())
    {
        panel->setCurrentPage("Map Editor");
    }
}

void page_motif_maker::slot_saveAsTile()
{
    auto timo = motifMaker->motifData.getDelegatedTileMotif();
    if (!timo) return;

    auto motif= timo->getMotif();
    if (!motif) return;

    // create EdgePoly
    auto map = motif->getMotifMap();
    auto edges = map->getEdges();
    QList<QLineF> lines;
    for (auto & edge : edges)
    {
        QLineF line = edge->getLine();
        lines << line;
    }
    EdgePoly ep(lines,false);

    // make name for export
    VersionFileList xfl = FileServices::getFiles(FILE_GIRIH);

    DlgListNameSelect dlg(xfl,0);
    dlg.setWindowTitle("Poly Name");
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    VersionedName vname(dlg.newEdit->text());

    // export
    QString filename = Sys::girihDir + vname.get() + ".xml";

    VersionedFile xfile;
    xfile.setFromFullPathname(filename);

    bool rv;
    QFile data(filename);
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream ts(&data);
        ts.setRealNumberPrecision(16);

        TileWriter tileWriter;

        ts << "<?xml version=\"1.0\"?>" << endl;

        QString str = QString("<Poly name=\"%1\">").arg(vname.get());
        ts << str << endl;
        tileWriter.setEdgePoly(ts,ep);

        //tileWriter.setTransform(ts,placement);
        ts << "</Poly>" << endl;

        data.close();

        if (FileServices::reformatXML(xfile))
        {
            rv = true;
        }
        else
        {
            qWarning() << "Could not write tile file:"  << filename;
            rv = false;
        }
    }
    else
    {
        rv = false;
    }

    // report
    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Saved: %1 - OK").arg(vname.get()));
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Error saving: %1 - FAILED").arg(vname.get()));
    }
    box.exec();
}

void page_motif_maker::slot_combine()
{
    auto timos = motifMaker->motifData.getVisibleTileMotifs();
    if (timos.size() < 2)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Cannot combine unless two or more motifs are multi-selected");
        box.exec();
        return;
    }

    qreal tolerance = config->mapedMergeSensitivity;
    qDebug() << "combining" << timos.size() << "maps, tolerance =" << tolerance;

    qsizetype sides = 0;
    TilePtr tp;

    MapPtr compositeMap = make_shared<Map>("Composite");
    for (auto & timo : std::as_const(timos))
    {
        auto tile = timo->getTile();
        if (tile->numEdges() > sides)
        {
            sides = tile->numEdges();
            tp    = tile;
        }

        auto motif = timo->getMotif();
        MapPtr map = motif->getMotifMap();
        compositeMap->mergeMap(map,tolerance);
    }
    Q_ASSERT(tp);

    MapCleanser mc(compositeMap);
    mc.deDuplicateVertices(tolerance);

    MapVerifier mv(compositeMap);
    mv.verify(true);

    auto prototype = motifMaker->motifData.getActivePrototype();
    auto tiling = prototype->getTiling();

    auto emm = make_shared<ExplicitMapMotif>(compositeMap);
    emm->setN(sides);

    auto timo = make_shared<TileMotif>(tiling, tp,emm);

    prototype->addTileMotif(timo);

    motifMaker->motifData.usePrototype(prototype);
}

void page_motif_maker::slot_duplicateCurrent()
{
    motifMaker->duplicateTileMotif();
}

void page_motif_maker::slot_deleteCurrent()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Delete Tile. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    auto proto = motifMaker->motifData.getActivePrototype();
    if (!proto) return;

    auto timo = motifMaker->motifData.getDelegatedTileMotif();
    if (!timo) return;

    TilePtr tile = timo->getTile();
    if (!tile) return;

    PlacedTileSet deletions;
    TilingPtr tiling = proto->getTiling();
    if (!tiling) return;

    const PlacedTileSet tileSet = tiling->unit().getIncluded();
    for (const auto & placedTile : std::as_const(tileSet))
    {
        if (placedTile->getTile() == tile)
        {
            deletions.push_back(placedTile);
        }
    }
    for (auto & ptp : std::as_const(deletions))
    {
        tiling->unit().removePlacedTile(ptp);
    }
    proto->removeTileMotif(timo);
    proto->wipeoutProtoMap();

    motifMaker->motifData.usePrototype(proto);
    motifMaker->protoData.remove(proto);
    motifMaker->protoData.add(proto);

    Sys::render(RENDER_RESET_STYLES);   // since protomap already reset
}

void page_motif_maker::slot_prototypeSelected(int row)
{
    QVariant var = prototypeCombo->itemData(row);
    if (var.canConvert<WeakProtoPtr>())
    {
        WeakProtoPtr wpp = var.value<WeakProtoPtr>();
        ProtoPtr pp      = wpp.lock();
        motifMaker->motifData.usePrototype(pp);
    }
}

void page_motif_maker::slot_widthChanged(int val)
{
    config->motifViewWidth = qreal(val);
    emit sig_updateView();
}

void page_motif_maker::setPageStatusString()
{
    QString msg("<body>"
                "<font color=blue>motif</font>  |  "
                "<font color=magenta>tile boundary</font>  |  "
                "<font color=red>motif boundary</font>  |  "
                "<font color=yellow>extended boundary</font>"
                "</body>");

    pageStatusString = msg;
}
