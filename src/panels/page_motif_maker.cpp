#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>

#include "panels/page_motif_maker.h"
#include "motifs/motif.h"
#include "motifs/explicit_motif.h"
#include "motifs/radial_motif.h"
#include "geometry/map.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/motif_button.h"
#include "makers/motif_maker/motif_selector.h"
#include "makers/motif_maker/motif_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "tile/tile.h"
#include "tiledpatternmaker.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

typedef std::weak_ptr<Prototype> WeakPrototypePtr;

Q_DECLARE_METATYPE(WeakPrototypePtr)

page_motif_maker::page_motif_maker(ControlPanel * cpanel) : panel_page(cpanel,"Motif Maker")
{
    QRadioButton * rEnlarge  = new QRadioButton("Enlarge to fill");
    QRadioButton * rActual   = new QRadioButton("Actual size");

    whiteBackground          = new QCheckBox("White background");
    QCheckBox * chkMulti     = new QCheckBox("Multi-Select Motifs");

    QPushButton * pbDup      = new QPushButton("Duplicate Motif");
    QPushButton * pbDel      = new QPushButton("Delete Motif");
    QPushButton * pbEdit     = new QPushButton("Edit Map");
    QPushButton * pbCombine  = new QPushButton("Combine Motifs");
    QPushButton * pbRender   = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QCheckBox   * pbPropagate = new QCheckBox("Propagate Changes");
    pbPropagate->setChecked(config->motifPropagate);
    connect(pbPropagate,         &QCheckBox::clicked, [this](bool checked) { config->motifPropagate = checked; motifMaker->setPropagate(checked); } );

    QLabel * tlabel          = new QLabel("Loaded tilings");
    tilingListBox            = new QComboBox();
    tilingListBox->setMinimumWidth(131);

    AQWidget * motifWidget  =  createMotifWidget();

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(tlabel);
    hbox->addWidget(tilingListBox);
    hbox->addStretch();
    hbox->addWidget(rEnlarge);
    hbox->addWidget(rActual);
    hbox->addStretch();
    hbox->addWidget(chkMulti);
    hbox->addWidget(whiteBackground);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addWidget(pbPropagate);
    hbox->addStretch();
    hbox->addWidget(pbCombine);
    hbox->addWidget(pbDel);
    hbox->addWidget(pbDup);
    hbox->addWidget(pbEdit);
    hbox->addWidget(pbRender);
    vbox->addLayout(hbox);

    if (config->insightMode)
    {
        QCheckBox * hiliteUnit   = new QCheckBox("Highlight Unit");
                    replicate    = new QCheckBox("Replicate");
        QCheckBox * showMotif    = new QCheckBox("Motif");
        QCheckBox * showTile     = new QCheckBox("Tile Boundary");
        QCheckBox * showMotifB   = new QCheckBox("Motif Boundary");
        QCheckBox * showExt      = new QCheckBox("Extended Boundary");
        QPushButton * pbSwap     = new QPushButton("Swap Type");

        hbox = new QHBoxLayout;
        hbox->addWidget(showMotif);
        hbox->addWidget(showTile);
        hbox->addWidget(showMotifB);
        hbox->addWidget(showExt);
        hbox->addWidget(hiliteUnit);
        hbox->addWidget(replicate);
        hbox->addWidget(pbSwap);
        vbox->addLayout(hbox);

        replicate->setChecked(!config->dontReplicate);
        hiliteUnit->setChecked(config->highlightUnit);
        showTile->setChecked(config->showTileBoundary);
        showMotifB->setChecked(config->showMotifBoundary);
        showMotif->setChecked(config->showMotif);
        showExt->setChecked(config->showExtendedBoundary);

        connect(replicate,          &QCheckBox::clicked, this,  &page_motif_maker::replicateClicked);
        connect(hiliteUnit,         &QCheckBox::clicked, [this](bool checked) { config->highlightUnit        = checked; view->update(); } );
        connect(showTile,           &QCheckBox::clicked, [this](bool checked) { config->showTileBoundary     = checked; view->update(); } );
        connect(showMotifB,         &QCheckBox::clicked, [this](bool checked) { config->showMotifBoundary   = checked; view->update(); } );
        connect(showMotif,          &QCheckBox::clicked, [this](bool checked) { config->showMotif           = checked; view->update(); } );
        connect(showExt,            &QCheckBox::clicked, [this](bool checked) { config->showExtendedBoundary = checked; view->update(); } );
        connect(pbSwap,             &QPushButton::clicked,      this, &page_motif_maker::slot_swapFeatureType);
        connect(view,               &View::sig_refreshMotifMenu,this, &page_motif_maker::slot_rebuildCurrentFigure);
    }

    // putting it together
    vbox->addSpacing(7);
    vbox->addWidget(motifWidget);
    vbox->addStretch();

    whiteBackground->setChecked(config->motifBkgdWhite);
    chkMulti->setChecked(config->motifMultiView);
    if (config->motifEnlarge)
        rEnlarge->setChecked(true);
    else
        rActual->setChecked(true);

    connect(pbRender,           &QPushButton::clicked,                  this,   &panel_page::sig_render);
    connect(pbDup,              &QPushButton::clicked,                  this,   &page_motif_maker::slot_duplicateCurrent);
    connect(pbDel,              &QPushButton::clicked,                  this,   &page_motif_maker::slot_deleteCurrent);
    connect(pbEdit,             &QPushButton::clicked,                  this,   &page_motif_maker::slot_editCurrent);
    connect(pbCombine,          &QPushButton::clicked,                  this,   &page_motif_maker::slot_combine);
    connect(whiteBackground,    &QCheckBox::clicked,                    this,   &page_motif_maker::whiteClicked);
    connect(chkMulti,           &QCheckBox::clicked,                    this,   &page_motif_maker::multiClicked);
    connect(tilingListBox,      SIGNAL(currentIndexChanged(int)),       this,   SLOT(slot_prototypeSelected(int)));
    connect(rEnlarge,           &QRadioButton::clicked,                 this,   [=] { config->motifEnlarge = true;  view->update(); });
    connect(rActual ,           &QRadioButton::clicked,                 this,   [=] { config->motifEnlarge = false; view->update(); });

    connect(motifMaker,         &MotifMaker::sig_tilingChoicesChanged,  this,   &page_motif_maker::slot_tilingChoicesChanged, Qt::QueuedConnection);
    connect(motifMaker,         &MotifMaker::sig_tilingChanged,         this,   &page_motif_maker::slot_tilingChanged, Qt::QueuedConnection);
    connect(motifMaker,         &MotifMaker::sig_tileChanged,           this,   &page_motif_maker::slot_tileChanged, Qt::QueuedConnection);
}

AQWidget * page_motif_maker::createMotifWidget()
{
    motifMaker = MotifMaker::getInstance();

    // Motif buttons
    motifSelector    = new MotifSelector();

    // larger slected feature button
    viewerBtn  = make_shared<MotifButton>(-1);

    // top row
    QHBoxLayout * btnBox = new QHBoxLayout();
    btnBox->addWidget(motifSelector);
    btnBox->addWidget(viewerBtn.get());
    btnBox->addStretch();

    // Editors
    motifEditor  = new MotifEditor(this);

    AQVBoxLayout * motifBox = new AQVBoxLayout;
    motifBox->addLayout(btnBox);
    motifBox->addWidget(motifEditor);

    AQWidget * w = new AQWidget();
    w->setLayout(motifBox);
    w->setMinimumWidth(610);

    connect(motifSelector,  &MotifSelector::sig_launcherButton, this, &page_motif_maker::slot_selectMotifButton);

    return w;
}

void page_motif_maker::onEnter()
{
    static QString msg("<body>"
                   "<font color=blue>motif</font>  |  "
                   "<font color=magenta>tile boundary</font>  |  "
                   "<font color=red>motif boundary</font>  |  "
                   "<font color=yellow>extended boundary</font>"
                   "</body>");

    panel->pushPanelStatus(msg);

}

void page_motif_maker::onExit()
{
    panel->popPanelStatus();
}

void page_motif_maker::onRefresh(void)
{
    static WeakPrototypePtr wpp;

    if (wpp.lock() != motifMaker->getSelectedPrototype())
    {
        auto pp = motifMaker->getSelectedPrototype();
        wpp     = pp;
        // it is possible that there is a new tiling for the prototype
        slot_tilingChoicesChanged();
        // setup the menu widget for the new prototype
        setPrototype(pp);
    }

    motifSelector->tallyButtons();

    whiteBackground->blockSignals(true);
    whiteBackground->setChecked(config->motifBkgdWhite);
    whiteBackground->blockSignals(false);

    if (config->insightMode)
    {
        replicate->blockSignals(true);
        replicate->setChecked(!config->dontReplicate);
        replicate->blockSignals(false);
    }
}

// this sets up the whole shebang
void page_motif_maker::setPrototype(PrototypePtr proto)
{
    motifSelector->setup(proto);
    update();
}

void page_motif_maker::motifModified(MotifPtr fp)
{
    DesignElementPtr dep = motifMaker->getSelectedDesignElement();
    dep->setMotif(fp);

    setButtonTransforms();

    // notify motif maker
    motifMaker->setSelectedDesignElement(dep);     // if this is the samne design element this does nothing
    auto tiling = motifMaker->getSelectedPrototype()->getTiling();
    motifMaker->sm_takeUp(tiling,SM_MOTIF_CHANGED);

}

// this is called when the figure type changes, say from Star to Rosette, etc
// but it's only effect is to change the transform of the buttons
void page_motif_maker::setButtonTransforms()
{
    viewerBtn->setViewTransform();
    MotifBtnPtr btn = motifSelector->getCurrentButton();
    if (btn)
    {
        btn->setViewTransform();
    }
    update();
}

// when a motif button is selected, this sets the motif in bot the motif maker and the motif editor
void page_motif_maker::slot_selectMotifButton(MotifBtnPtr fb)
{
    if (!fb) return;
    qDebug() << "MotifWidget::slot_selectMotifButton btn=" << fb->getIndex() << fb.get();

    DesignElementPtr designElement = fb->getDesignElement(); // DAC taprats cloned here
    if (!designElement) return;

    motifMaker->setSelectedDesignElement(designElement);
    auto tile = designElement->getTile();
    motifMaker->setActiveTile(tile);
    viewerBtn->setDesignElement(designElement);
    motifEditor->selectMotif(designElement);

    ViewControl * view = ViewControl::getInstance();
    view->update();
    update();
}

void page_motif_maker::slot_tileChanged()
{
    setButtonTransforms();
}

void page_motif_maker::slot_tilingChanged()
{
    setPrototype(motifMaker->getSelectedPrototype());
}

void page_motif_maker::slot_tilingChoicesChanged()
{
    tilingListBox->blockSignals(true);

    tilingListBox->clear();

    const QVector<PrototypePtr> & protos = motifMaker->getPrototypes();
    for (auto proto : protos)
    {
        tilingListBox->addItem(proto->getTiling()->getName(),QVariant::fromValue(WeakPrototypePtr(proto)));
    }

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    if (pp)
    {
        QString name = pp->getTiling()->getName();
        qDebug() << "page_motif_maker::reload() selected tiling:" << name;
        int index = tilingListBox->findText(name);
        tilingListBox->setCurrentIndex(index);
    }

    tilingListBox->blockSignals(false);
}

void  page_motif_maker::whiteClicked(bool state)
{
    config->motifBkgdWhite = state;
    emit sig_refreshView();
}

void  page_motif_maker::replicateClicked(bool state)
{
    config->dontReplicate = !state;
    slot_rebuildCurrentFigure();
}

void page_motif_maker::slot_rebuildCurrentFigure()
{
    auto del    = motifMaker->getSelectedDesignElement();
    auto motif  = del->getMotif();
    motif->resetMaps();
    emit sig_refreshView();

    auto btn = motifSelector->getCurrentButton();
    if (btn)
    {
        slot_selectMotifButton(btn);
    }
}

void  page_motif_maker::multiClicked(bool state)
{
    config->motifMultiView = state;
    emit sig_refreshView();
}

void page_motif_maker::slot_combine()
{
    auto delps = motifMaker->getSelectedDesignElements();
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
    TilePtr fp;

    MapPtr compositeMap = make_shared<Map>("Composite map");
    for (auto & delp : delps)
    {
        auto tile = delp->getTile();
        if (tile->numSides() > sides)
        {
            sides = tile->numSides();
            fp    = tile;
        }

        auto motif = delp->getMotif();
        MapPtr map = motif->getMap();
        compositeMap->mergeMap(map,tolerance);
    }

    compositeMap->deDuplicateVertices(tolerance);

    //compositeMap->buildNeighbours();

    compositeMap->verify(true);

    auto ef = make_shared<ExplicitMotif>(compositeMap,MOTIF_TYPE_EXPLICIT,sides);
    auto delp = make_shared<DesignElement>(fp,ef);
    auto prototype = motifMaker->getSelectedPrototype();
    prototype->addElement(delp);
    setPrototype(prototype);
}

void page_motif_maker::slot_duplicateCurrent()
{
    motifMaker->dupolicateMotif();

    setPrototype(motifMaker->getSelectedPrototype());
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

    motifMaker->deleteActiveTile();

    setPrototype(motifMaker->getSelectedPrototype());
}

void page_motif_maker::slot_editCurrent()
{
    MapEditor * maped = MapEditor::getInstance();

    if (maped->loadSelectedMotifs())
    {
        panel->setCurrentPage("Map Editor");
    }
}

void page_motif_maker::slot_prototypeSelected(int row)
{
    QVariant var = tilingListBox->itemData(row);
    if (var.canConvert<WeakPrototypePtr>())
    {
        WeakPrototypePtr wpp = var.value<WeakPrototypePtr>();
        PrototypePtr pp      = wpp.lock();
        select(pp);
    }
}

TilePtr page_motif_maker::getActiveTile()
{
    return motifMaker->getActiveTile();
}

void page_motif_maker::select(PrototypePtr prototype)
{
    qDebug() << "MotifMaker::select prototype="  << prototype.get();

    motifMaker->setSelectedPrototype(prototype);

    tilingMaker->select(prototype->getTiling());
}

// this is a change in the motif
void page_motif_maker::slot_motifModified(MotifPtr motif)
{
    motifModified(motif);
    emit sig_refreshView();
}

void page_motif_maker::slot_motifTypeChanged(eMotifType type)
{
    // placeholder if anything else needs to be  done here
    Q_UNUSED(type);

    emit sig_refreshView();
}

void page_motif_maker::slot_swapFeatureType()
{
    auto del   = motifMaker->getSelectedDesignElement();
    auto tile  = del->getTile();
    tile->setRegular(!tile->isRegular());

    motifMaker->sm_takeUp(tilingMaker->getSelected(),SM_TILE_CHANGED);

    slot_rebuildCurrentFigure();
}
