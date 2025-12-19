#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QComboBox>

#include "gui/map_editor/map_editor.h"
#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/design_element_selector.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/enums/erender.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"


using std::make_shared;

page_motif_maker::page_motif_maker(ControlPanel * cpanel) : panel_page(cpanel,PAGE_MOTIF_MAKER,"Motif Maker")
{
    setPageStatusString();

    AQPushButton * pbEnlarged = new AQPushButton("Single");
    AQPushButton * pbSelected = new AQPushButton("Selected");
    AQPushButton * pbTiled    = new AQPushButton("Unit");
    pbEnlarged->setMinimumWidth(10);
    pbSelected->setMinimumWidth(10);
    pbTiled->setMinimumWidth(10);

    QButtonGroup * bgrp    = new QButtonGroup();
    bgrp->setExclusive(true);
    bgrp->addButton(pbEnlarged);
    bgrp->addButton(pbSelected);
    bgrp->addButton(pbTiled);

    smxWidget  = new SMXWidget(Sys::motifMakerView.get(),true,true);
    smxWidget->setObjectName("abc");
    smxWidget->setStyleSheet("#abc {border: 1px solid black; }");

    SpinSet * widthSpin = new SpinSet("Line Width",3,1,9);
    widthSpin->setValue((int)config->motifViewWidth);

    QPushButton * pbDup      = new QPushButton("Duplicate Motif");
    QPushButton * pbDel      = new QPushButton("Delete Motif");
    QPushButton * pbEdit     = new QPushButton("Edit Map");
    QPushButton * pbCombine  = new QPushButton("Combine Motifs");
    QPushButton * pbSwapReg  = new QPushButton("Swap Tile Regularity");
    QPushButton * pbRender   = new QPushButton("Render Motifs");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QCheckBox   * pbPropagate = new QCheckBox("Propagate");
    pbPropagate->setChecked(true);
    connect(pbPropagate, &QCheckBox::clicked, prototypeMaker, &PrototypeMaker::slot_propagateChanged);

    protoLabel              = new QLabel("Prototypes");
    prototypeCombo          = new QComboBox();
    prototypeCombo->setMinimumWidth(131);

    QHBoxLayout * hbox      = new QHBoxLayout;
    hbox->addWidget(protoLabel);
    hbox->addWidget(prototypeCombo);
    hbox->addWidget(pbEnlarged);
    hbox->addWidget(pbSelected);
    hbox->addWidget(pbTiled);
    hbox->addWidget(smxWidget);
    hbox->addLayout(widthSpin);

    vbox->addLayout(hbox);

    if (config->insightMode)
    {
        QCheckBox * hiliteUnit   = new QCheckBox("Highlight Unit");
                    replicate    = new QCheckBox("Replicate");
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
        connect(Sys::sysview,       &SystemView::sig_rebuildMotif, this, &page_motif_maker::slot_rebuildMotif);
        connect(hiliteUnit,         &QCheckBox::clicked, this, [this](bool checked) { Sys::highlightUnit           = checked; emit sig_updateView(); } );
        connect(showTile,           &QCheckBox::clicked, this, [this](bool checked) { config->showTileBoundary     = checked; emit sig_updateView(); } );
        connect(showMotifB,         &QCheckBox::clicked, this, [this](bool checked) { config->showMotifBoundary    = checked; emit sig_updateView(); } );
        connect(showMotif,          &QCheckBox::clicked, this, [this](bool checked) { config->showMotif            = checked; emit sig_updateView(); } );
        connect(showExt,            &QCheckBox::clicked, this, [this](bool checked) { config->showExtendedBoundary = checked; emit sig_updateView(); } );
        connect(showTileC,          &QCheckBox::clicked, this, [this](bool checked) { config->showTileCenter       = checked; emit sig_updateView(); } );
        connect(showMotifC,         &QCheckBox::clicked, this, [this](bool checked) { config->showMotifCenter      = checked; emit sig_updateView(); } );
    }
    else
    {
        Sys::highlightUnit              = false;
        Sys::dontReplicate              = false;
        config->showTileBoundary        = true;
        config->showMotifBoundary       = true;
        config->showMotif               = true;
        config->showExtendedBoundary    = true;
        config->showTileCenter          = false;
        config->showMotifCenter         = false;
    }

    hbox = new QHBoxLayout;
    hbox->addWidget(pbCombine);
    hbox->addWidget(pbDel);
    hbox->addWidget(pbDup);
    hbox->addWidget(pbEdit);
    hbox->addWidget(pbSwapReg);
    hbox->addStretch();
    hbox->addWidget(pbRender);
    hbox->addWidget(pbPropagate);
    vbox->addLayout(hbox);

    // the Motif maker widget
    motifMakerWidget  =  new MotifMakerWidget();
    prototypeMaker->setWidget(motifMakerWidget);

    // putting it together
    vbox->addSpacing(7);
    vbox->addWidget(motifMakerWidget);
    vbox->addStretch();

    switch (config->motifMakerView)
    {
    case MOTIF_VIEW_SOLO:
        pbEnlarged->setChecked(true);
        break;
    case MOTIF_VIEW_SELECTED:
        pbSelected->setChecked(true);
        break;
    case MOTIF_VIEW_DESIGN_UNIT:
        pbTiled->setChecked(true);
        break;
    }


    connect(pbRender,           &QPushButton::clicked,              this, [] {Sys::render(RENDER_RESET_MOTIFS);} );
    connect(pbSwapReg,          &QPushButton::clicked,              this,   &page_motif_maker::slot_swapTileRegularity);
    connect(pbDup,              &QPushButton::clicked,              this,   &page_motif_maker::slot_duplicateCurrent);
    connect(pbDel,              &QPushButton::clicked,              this,   &page_motif_maker::slot_deleteCurrent);
    connect(pbEdit,             &QPushButton::clicked,              this,   &page_motif_maker::slot_editCurrent);
    connect(pbCombine,          &QPushButton::clicked,              this,   &page_motif_maker::slot_combine);
    connect(pbEnlarged,         &QPushButton::toggled,              this,   &page_motif_maker::enlargedClicked);
    connect(pbSelected,         &QPushButton::toggled,              this,   &page_motif_maker::selectedClicked);
    connect(pbTiled,            &QPushButton::toggled,              this,   &page_motif_maker::tiledClicked);
    connect(prototypeCombo,     &QComboBox::currentIndexChanged,    this,   &page_motif_maker::slot_prototypeSelected);
    connect(widthSpin,          &SpinSet::valueChanged,             this,   &page_motif_maker::slot_widthChanged);
}

void page_motif_maker::onEnter()
{
    setPageStatus();
}

void page_motif_maker::onExit()
{
    clearPageStatus();
}

void page_motif_maker::onRefresh(void)
{
    loadProtoCombo();

    motifMakerWidget->refreshMotifMakerWidget();

    auto editor = motifMakerWidget->getMotifEditor();
    editor->onRefresh();

    if (config->insightMode)
    {
        replicate->blockSignals(true);
        replicate->setChecked(!Sys::dontReplicate);
        replicate->blockSignals(false);
    }

    smxWidget->refresh();
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

void page_motif_maker::loadProtoCombo()
{
    // determine if needs to be loaded
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

    QVector<ProtoPtr> currentProtos = prototypeMaker->getPrototypes();

    bool rebuild = false;
    if (loadedProtos.count() != currentProtos.count())
    {
        rebuild = true;
    }
    else
    {
        for (auto & current : currentProtos)
        {
            if (!loadedProtos.contains(current))
            {
                rebuild = true;
                break;
            }
        }
    }

    if (rebuild == false)
    {
        return;
    }

    // rebuild the combo
    prototypeCombo->blockSignals(true);

    prototypeCombo->clear();

    for (auto & proto : std::as_const(currentProtos))
    {
        prototypeCombo->addItem(proto->getTiling()->getVName().get(),QVariant::fromValue(WeakProtoPtr(proto)));
    }

    protoLabel->setText(QString("Prototypes (%1)").arg(prototypeCombo->count()));

    if (prototypeCombo->count() == 0)
    {
        return;
    }

    ProtoPtr selected = prototypeMaker->getSelectedPrototype();
    if (selected)
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
            prototypeMaker->select(MVD_DELEM,proto,false);
        }
    }
    else
    {
        prototypeCombo->setCurrentIndex(0);
        auto v = prototypeCombo->itemData(0);
        WeakProtoPtr wproto = v.value<WeakProtoPtr>();
        auto proto = wproto.lock();
        prototypeMaker->select(MVD_DELEM,proto,false);
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
    prototypeMaker->rebuildCurrentMotif();
    emit sig_reconstructView();
}

void  page_motif_maker::enlargedClicked()
{
    config->motifMakerView = MOTIF_VIEW_SOLO;

    // re-delegate currnent selection
    auto btn = motifMakerWidget->getDelegatedButton();
    motifMakerWidget->delegate(btn, false, true);

    emit sig_reconstructView();
}

void  page_motif_maker::selectedClicked()
{
    config->motifMakerView = MOTIF_VIEW_SELECTED;
    auto btn = motifMakerWidget->getDelegatedButton();
    motifMakerWidget->delegate(btn, true, true);
    emit sig_reconstructView();
}

void  page_motif_maker::tiledClicked()
{
    config->motifMakerView = MOTIF_VIEW_DESIGN_UNIT;

    // re-delegate currnent selection
    auto btn = motifMakerWidget->getDelegatedButton();
    motifMakerWidget->delegate(btn, false, true);

    emit sig_reconstructView();
}

void page_motif_maker::slot_editCurrent()
{
    if (Sys::mapEditor->loadSelectedMotifs())
    {
        panel->setCurrentPage("Map Editor");
    }
}

void page_motif_maker::slot_combine()
{
    auto delps = prototypeMaker->getSelectedDELs(MVD_DELEM);
    if (delps.size() < 2)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Cannot combine unless two or more motifs are multi-selected");
        box.exec();
        return;
    }

    qreal tolerance = config->mapedMergeSensitivity;
    qDebug() << "combining" << delps.size() << "maps, tolerance =" << tolerance;

    qsizetype sides = 0;
    TilePtr tp;

    MapPtr compositeMap = make_shared<Map>("Composite");
    for (auto & delp : std::as_const(delps))
    {
        auto tile = delp->getTile();
        if (tile->numEdges() > sides)
        {
            sides = tile->numEdges();
            tp    = tile;
        }

        auto motif = delp->getMotif();
        MapPtr map = motif->getMotifMap();
        compositeMap->mergeMap(map,tolerance);
    }
    Q_ASSERT(tp);

    MapCleanser mc(compositeMap);
    mc.deDuplicateVertices(tolerance);

    MapVerifier mv(compositeMap);
    mv.verify(true);

    auto emm = make_shared<ExplicitMapMotif>(compositeMap);
    emm->setN(sides);
    auto delp = make_shared<DesignElement>(tp,emm);
    auto prototype = prototypeMaker->getSelectedPrototype();
    prototype->addDesignElement(delp);
    prototypeMaker->select(MVD_DELEM,prototype,false);
}

void page_motif_maker::slot_duplicateCurrent()
{
    prototypeMaker->duplicateDesignElement();

    //prototypeMaker->setSelectedPrototype(prototypeMaker->getSelectedPrototype());
    prototypeMaker->select(MVD_DELEM,prototypeMaker->getSelectedPrototype(),false);
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

    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto) return;

    auto del = prototypeMaker->getSelectedDEL();
    if (!del) return;

    TilePtr tile = del->getTile();
    if (!tile) return;

    PlacedTiles deletions;
    TilingPtr tiling = proto->getTiling();
    const PlacedTiles tilingUnit = tiling->unit().getIncluded();
    for (const auto & placedTile : std::as_const(tilingUnit))
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
    proto->removeDesignElement(del);
    proto->wipeoutProtoMap();

    prototypeMaker->select(MVD_DELEM,proto,false);

    Sys::render(RENDER_RESET_STYLES);   // since protomap already reset
}

void page_motif_maker::slot_swapTileRegularity()
{
    auto proto = prototypeMaker->getSelectedPrototype();
    if (!proto) return;

    auto del   = prototypeMaker->getSelectedDEL();
    if (!del) return;

    auto tile  = del->getTile();
    tile->flipRegularity();

    ProtoEvent pevent;
    pevent.event  = PROM_TILE_UNIQUE_REGULARITY_CHANGED;
    pevent.tiling = proto->getTiling();
    pevent.tile = tile;
    prototypeMaker->sm_takeUp(pevent);

    auto btn =  motifMakerWidget->getButton(del);
    motifMakerWidget->delegate(btn,false,true);

    Sys::render(RENDER_RESET_MOTIFS);
}

void page_motif_maker::slot_prototypeSelected(int row)
{
    QVariant var = prototypeCombo->itemData(row);
    if (var.canConvert<WeakProtoPtr>())
    {
        WeakProtoPtr wpp = var.value<WeakProtoPtr>();
        ProtoPtr pp      = wpp.lock();
        prototypeMaker->select(MVD_DELEM,pp,false);
    }
}

void page_motif_maker::slot_widthChanged(int val)
{
    config->motifViewWidth = qreal(val);
    emit sig_updateView();
}

