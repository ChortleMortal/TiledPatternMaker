#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>

#include "panels/page_motif_maker.h"
#include "figures/figure.h"
#include "figures/explicit_figure.h"
#include "geometry/map.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/motif_maker/feature_selector.h"
#include "makers/motif_maker/motif_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tiling.h"
#include "tile/feature.h"
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
    QCheckBox * chkMulti     = new QCheckBox("Multi-Select Figures");

    QPushButton * pbDup      = new QPushButton("Duplicate Figure");
    QPushButton * pbDel      = new QPushButton("Delete Figure");
    QPushButton * pbEdit     = new QPushButton("Edit Map");
    QPushButton * pbCombine  = new QPushButton("Combine Figures");
    QPushButton * pbRender   = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

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
        QCheckBox * replicateRadial  = new QCheckBox("Replicate Radial");
        QCheckBox * showFeature  = new QCheckBox("Show Feature Boundary");
        QCheckBox * showFigure   = new QCheckBox("Show Figure Boundary");
        QCheckBox * showExt      = new QCheckBox("Show Extended Boundary");

        hbox = new QHBoxLayout;
        hbox->addWidget(showFeature);
        hbox->addWidget(showFigure);
        hbox->addWidget(showExt);
        hbox->addStretch();
        hbox->addWidget(hiliteUnit);
        hbox->addWidget(replicateRadial);
        vbox->addLayout(hbox);

        replicateRadial->setChecked(!config->dontReplicate);
        hiliteUnit->setChecked(config->highlightUnit);
        showFeature->setChecked(config->showFeatureBoundary);
        showFigure->setChecked(config->showFigureBoundary);
        showExt->setChecked(config->showExtendedBoundary);

        connect(replicateRadial,    &QCheckBox::clicked, this,  &page_motif_maker::replicateRadialClicked);
        connect(hiliteUnit,         &QCheckBox::clicked, [this](bool checked) { config->highlightUnit        = checked; view->update(); } );
        connect(showFeature,        &QCheckBox::clicked, [this](bool checked) { config->showFeatureBoundary  = checked; view->update(); } );
        connect(showFigure,         &QCheckBox::clicked, [this](bool checked) { config->showFigureBoundary   = checked; view->update(); } );
        connect(showExt,            &QCheckBox::clicked, [this](bool checked) { config->showExtendedBoundary = checked; view->update(); } );
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
    connect(motifMaker,         &MotifMaker::sig_featureChanged,        this,   &page_motif_maker::slot_featureChanged, Qt::QueuedConnection);
}

AQWidget * page_motif_maker::createMotifWidget()
{
    motifMaker = MotifMaker::getInstance();

    // Feature buttons
    featureSelector    = new FeatureSelector();

    // larger slected feature button
    viewerBtn  = make_shared<FeatureButton>(-1);

    // top row
    QHBoxLayout * btnBox = new QHBoxLayout();
    btnBox->addWidget(featureSelector);
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

    connect(featureSelector,  &FeatureSelector::sig_launcherButton, this, &page_motif_maker::slot_selectFeatureButton);

    return w;
}

void page_motif_maker::onEnter()
{
    static QString msg("<body>"
                   "<font color=blue>figure</font>  |  "
                   "<font color=magenta>feature boundary</font>  |  "
                   "<font color=red>radial figure boundary</font>  |  "
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

    featureSelector->tallyButtons();

    whiteBackground->blockSignals(true);
    whiteBackground->setChecked(config->motifBkgdWhite);
    whiteBackground->blockSignals(false);
}

// this sets up the whole shebang
void page_motif_maker::setPrototype(PrototypePtr proto)
{
    featureSelector->setup(proto);
    update();
}

void page_motif_maker::figureModified(FigurePtr fp)
{
    DesignElementPtr dep = motifMaker->getSelectedDesignElement();
    dep->setFigure(fp);

    setButtonTransforms();

    // notify motif maker
    motifMaker->setSelectedDesignElement(dep);     // if this is the samne design element this does nothing
    auto tiling = motifMaker->getSelectedPrototype()->getTiling();
    motifMaker->sm_take(tiling,SM_FIGURE_CHANGED);

}

// this is called when the figure type changes, say from Star to Rosette, etc
// but it's only effect is to change the transform of the buttons
void page_motif_maker::setButtonTransforms()
{
    viewerBtn->setViewTransform();
    FeatureBtnPtr btn = featureSelector->getCurrentButton();
    if (btn)
    {
        btn->setViewTransform();
    }
    update();
}

// when a feature button is selected, this sets the feature in the windows and in the editor
void page_motif_maker::slot_selectFeatureButton(FeatureBtnPtr fb)
{
    if (!fb) return;
    qDebug() << "MotifWidget::slot_selectFeatureButton btn=" << fb->getIndex() << fb.get();

    DesignElementPtr designElement = fb->getDesignElement(); // DAC taprats cloned here
    if (!designElement) return;

    motifMaker->setSelectedDesignElement(designElement);
    auto feature = designElement->getFeature();
    motifMaker->setActiveFeature(feature);
    viewerBtn->setDesignElement(designElement);
    motifEditor->selectFigure(designElement);

    ViewControl * view = ViewControl::getInstance();
    view->update();
    update();
}

void page_motif_maker::slot_featureChanged()
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

void  page_motif_maker::replicateRadialClicked(bool state)
{
    config->dontReplicate = !state;
    auto del    = motifMaker->getSelectedDesignElement();
    auto figure = del->getFigure();
    figure->buildMaps();
    emit sig_refreshView();

    auto btn = featureSelector->getCurrentButton();
    if (btn)
    {
        slot_selectFeatureButton(btn);
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
        box.setText("Cannot combine unless two or more figures are multi-selected");
        box.exec();
        return;
    }

    qreal tolerance = config->mapedMergeSensitivity;
    qDebug() << "combining" << delps.size() << "maps, tolerance =" << tolerance;

    qsizetype sides = 0;
    FeaturePtr fp;

    MapPtr compositeMap = make_shared<Map>("Composite map");
    for (auto & delp : delps)
    {
        auto feature = delp->getFeature();
        if (feature->getPoints().size() > sides)
        {
            sides = feature->getPoints().size();
            fp    = feature;
        }

        auto figure = delp->getFigure();
        MapPtr map = figure->getFigureMap();
        compositeMap->mergeMap(map,tolerance);
    }

    compositeMap->deDuplicateVertices(tolerance);

    //compositeMap->buildNeighbours();

    compositeMap->verify(true);

    auto ef = make_shared<ExplicitFigure>(compositeMap,FIG_TYPE_EXPLICIT,sides);
    auto delp = make_shared<DesignElement>(fp,ef);
    auto prototype = motifMaker->getSelectedPrototype();
    prototype->addElement(delp);
    setPrototype(prototype);
}

void page_motif_maker::slot_duplicateCurrent()
{
    motifMaker->duplicateActiveFeature();

    setPrototype(motifMaker->getSelectedPrototype());
}

void page_motif_maker::slot_deleteCurrent()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Delete Feature. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    motifMaker->deleteActiveFeature();

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

FeaturePtr page_motif_maker::getActiveFeature()
{
    return motifMaker->getActiveFeature();
}

void page_motif_maker::select(PrototypePtr prototype)
{
    qDebug() << "MotifMaker::select  prototype="  << prototype.get();

    motifMaker->setSelectedPrototype(prototype);

    tilingMaker->select(prototype->getTiling());
}

// this is a change in the figure,
void page_motif_maker::slot_figureModified(FigurePtr fp)
{
    figureModified(fp);
    emit sig_refreshView();
}

void page_motif_maker::slot_figureTypeChanged(eFigType type)
{
    // placeholder if anything else needs to be  done here
    Q_UNUSED(type);

    emit sig_refreshView();
}
