#include <QDebug>

#include "gui/model_editors/motif_edit/adaptive_motif_editors.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/widgets/dlg_cleanse.h"
#include "model/makers/motif_maker.h"
#include "model/motifs/adaptive_girih.h"
#include "model/motifs/adaptive_motif.h"
#include "model/motifs/adaptive_hourglass.h"
#include "model/motifs/adaptive_inference.h"
#include "model/motifs/adaptive_intersect.h"
#include "model/motifs/adaptive_rosette.h"
#include "model/motifs/adaptive_star.h"
#include "model/motifs/adaptive_tile.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/motif.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/prototypes/tile_motif.h"
#include "sys/sys.h"

using std::static_pointer_cast;

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the TileMotif being edited is changed.

MotifEditorWidget::MotifEditorWidget(MotifMakerWidget * parent)
{
    this->parent = parent;
    createLayout();
}

void MotifEditorWidget::createLayout()
{
    QPushButton * addExt    = new QPushButton("Add Extender");
    QPushButton * delExt    = new QPushButton("Delete Extender");
    QPushButton * pbSwapReg = new QPushButton("Swap Tile Regularity");
                  pbLock    = new QCheckBox("Lock view scale");
                  cropChk   = new QCheckBox("Crop to tile");
                  cleanBtn  = new QPushButton("Motif Cleanse : OFF");

    pbLock->setChecked(false);
    cropChk->setEnabled(false);

    connect(cleanBtn,   &QPushButton::clicked,                 this, &MotifEditorWidget::slot_cleanse);
    connect(addExt,     &QPushButton::clicked,                 this, &MotifEditorWidget::slot_addExtender);
    connect(delExt,     &QPushButton::clicked,                 this, &MotifEditorWidget::slot_deleteExtender);
    connect(pbSwapReg,  &QPushButton::clicked,                 this, &MotifEditorWidget::slot_swapTileRegularity);
    connect(pbLock,     &QCheckBox::clicked,                   this, &MotifEditorWidget::slot_lockBtnView);
    connect(cropChk,    &QCheckBox::clicked,                   this, &MotifEditorWidget::slot_cropToTile);
    connect(&typeSelector, &MotifTypeSelector::sig_motifTypeChanged,    this, &MotifEditorWidget::slot_motifTypeChanged);
    connect(&typeSelector, &MotifTypeSelector::sig_motifVersionChanged, this, &MotifEditorWidget::slot_motifVersionChanged);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(&typeSelector);
    hbox->addStretch();
    hbox->addWidget(pbSwapReg);
    hbox->addSpacing(20);
    hbox->addWidget(cleanBtn);
    hbox->addStretch();
    hbox->addWidget(addExt);
    hbox->addSpacing(5);
    hbox->addWidget(delExt);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(cropChk);
    hbox2->addWidget(pbLock);
    hbox2->setContentsMargins(0,0,0,0);

    cornerWidget = new QWidget;
    cornerWidget->setContentsMargins(0,0,0,3);
    cornerWidget->setLayout(hbox2);

    for (auto & lt : lowerThirds)
    {
        lt = new LowerThirdWidget;
        thirdStack.addWidget(lt);
    }
    frontThirdWidget = lowerThirds[0];
    backThirdWidget  = lowerThirds[1];
    frontThirdWidget->setCornerWidget(cornerWidget);
    thirdStack.setCurrentWidget(frontThirdWidget);

    AQVBoxLayout * layout = new AQVBoxLayout();
    layout->setContentsMargins(0,9,0,5);
    layout->addLayout(hbox);
    layout->addWidget(&thirdStack);
    setLayout(layout);
}

void MotifEditorWidget::refresh()
{
    auto del = delegatedTileMotif.lock();
    if (!del) return;

    bool dontAutoFit = Sys::motifMakerView->getDontAutoFit();

    pbLock->blockSignals(true);
    pbLock->setChecked(dontAutoFit);
    if (dontAutoFit || Sys::motifMakerView->getViewMode() == MotifMakerView::ViewType::Single)
        pbLock->setEnabled(true);
    else
        pbLock->setEnabled(false);
    pbLock->blockSignals(false);

    auto motif = del->getMotif();
    cleanBtn->setText((motif->getCleanse() > 0) ? "Motif Cleanse : ON " : "Motif Cleanse : OFF");

    // refreshes the 'lower third'
    frontThirdWidget->refresh();
}

void MotifEditorWidget::delegateComboAndLowerThird(TileMotifPtr tileMotif)
{
    delegatedTileMotif = tileMotif;

    if (!tileMotif)
    {
        //lowerThirdWidget.set(nullptr,nullptr);
        return;
    }

    MotifPtr motif = tileMotif->getMotif();
    if (!motif)
    {
        //lowerThirdWidget.set(nullptr,nullptr);
        return;
    }

    // DAC note:  When a TileMotif is creeated from a tile in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "MotifEditor::selectMotif :" << motif->getMotifDesc();

    typeSelector.updateTypeChoices(motif);

    delegateLowerThird(tileMotif);
}


void MotifEditorWidget::delegateChangedMotifType(TileMotifPtr tileMotif, Motif::Type type)
{
    // places a new motif into an existing TileMotif
    MotifPtr oldMotif = tileMotif->getMotif();

    // create new motif
    MotifPtr motif = Sys::motifMaker->create(type,tileMotif);

    // place into TileMotif
    tileMotif->setMotif(motif);

    // reparent extenders
    for (auto & ext : oldMotif->getExtenders())
    {
        ext->reparent(motif.get());
    }

    // delegate lower third editor
    delegateLowerThird(tileMotif);

    // tigger rebuild and paint
    frontThirdWidget->getCurrentEditor()->motifModified(motif);
}

void MotifEditorWidget::updateCropCheckbox(MotifPtr motif)
{
    auto adaptive = std::dynamic_pointer_cast<AdaptiveMotif>(motif);
    cropChk->setEnabled(adaptive != nullptr);
    cropChk->blockSignals(true);
    cropChk->setChecked(adaptive ? adaptive->cropToTile : false);
    cropChk->blockSignals(false);
}

void MotifEditorWidget::delegateLowerThird(TileMotifPtr tileMotif, bool force)
{
    // replaces lower third editor with a new editor
    MotifPtr motif = tileMotif->getMotif();

    MotifEditor * ed = frontThirdWidget->getCurrentEditor();

    if (force || !ed || ed->getMotif() != motif)
    {
        switch (motif->getMotifType())
        {
        case Motif::Type::Undefined:
        case Motif::Type::Radial:
            qCritical("unexpected motif type");
            break;

        case Motif::Type::Star:
        {
            StarPtr star = std::static_pointer_cast<Star>(motif);
            ed = new StarEditor("radial_star_edit",star);
        }   break;

        case Motif::Type::Star2:
        {
            Star2Ptr star = std::static_pointer_cast<Star2>(motif);
            ed = new Star2Editor("radial_star2_edit",star);
        }   break;

        case Motif::Type::Rosette:
        {
            RosettePtr rose = std::static_pointer_cast<Rosette>(motif);
            ed = new RosetteEditor("radial_rosette_edit",rose);
        }   break;

        case Motif::Type::Rosette2:
        {
            Rosette2Ptr rose = std::static_pointer_cast<Rosette2>(motif);
            ed = new Rosette2Editor("radial_rosette2_edit",rose);
        }   break;

        case Motif::Type::MapMotif:
        {
            shared_ptr<ExplicitMapMotif> expmap = std::static_pointer_cast<ExplicitMapMotif>(motif);
            ed = new ExplicitMapEditor("explcit_map_edit",expmap);
        }   break;
        case Motif::Type::NoMapMotif:
        {
            auto nomap = std::static_pointer_cast<IrregularNoMap>(motif);
            ed = new IrregularNoMapEditor("irregular_npomap_edit",nomap);
        }   break;

        case Motif::Type::Inferred:
        {
            auto infer = std::static_pointer_cast<AdaptiveInference>(motif);
            ed = new InferEditor("infer_edit",infer);
        }   break;

        case Motif::Type::AdaptiveRosette:
        {
            auto rose = std::static_pointer_cast<AdaptiveRosette>(motif);
            ed = new IrregularRosetteEditor("irregular_rosette_edit",rose);
        }   break;

        case Motif::Type::Hourglass:
        {
            auto hour = std::static_pointer_cast<AdaptiveHourglass>(motif);
            ed = new HourglassEditor("hourglass_edit",hour);
        }   break;

        case Motif::Type::Intersect:
        {
            auto isect = std::static_pointer_cast<AdaptiveIntersect>(motif);
            ed = new IntersectEditor("intersect_edit",isect);
        }   break;

        case Motif::Type::Girih:
        {
            auto girih = std::static_pointer_cast<AdaptiveGirih>(motif);
            ed = new GirihEditor("girih_edit",girih);
        }   break;

        case Motif::Type::AdaptiveStar:
        {
            auto star = std::static_pointer_cast<AdaptiveStar>(motif);
            ed = new IrregularStarEditor("irregular_star_edit",star);
        }   break;

        case Motif::Type::ExplicitTile:
        {
            auto tmotif = std::static_pointer_cast<AdaptiveTile>(motif);
            ed = new ExplicitTileEditor("explicit_tile_edit",tmotif);
        }   break;

        }

        // build the new editor into the currently-hidden "back" tab widget and let it
        // fully refresh/settle there. It is never shown until the swap below, so Qt never
        // paints it in a provisional, not-yet-laid-out state - this is what caused the
        // sliders' spin boxes to visibly jump when rebuilding a live, visible tab.
        backThirdWidget->set(ed,motif);
        ed->refresh();
        updateCropCheckbox(motif);

        backThirdWidget->setCornerWidget(cornerWidget);
        thirdStack.setCurrentWidget(backThirdWidget);
        std::swap(frontThirdWidget,backThirdWidget);
    }
    else
    {
        ed->refresh();
        updateCropCheckbox(motif);
    }

    adjustSize();
}

// the type combo has changed
void MotifEditorWidget::slot_motifTypeChanged(Motif::Type type)
{
    // this is where a new motif needs to be put into the existing TileMotif
    auto timo = delegatedTileMotif.lock();
    if (!timo)
    {
        qWarning("MotifEditorWidget::slot_motifTypeChanged - no TileMotif");
        return;
    }

    delegateChangedMotifType(timo,type);

    typeSelector.setVersion(timo->getMotif());  // uses modified timo

    Sys::motifMaker->motifData.show(timo);
}

void MotifEditorWidget::slot_motifVersionChanged(int version)
{
    // this is where a new motif needs to be put into the existing TileMotif
    auto timo = delegatedTileMotif.lock();
    if (!timo)
    {
        qWarning("MotifEditorWidget::slot_motifVersionChanged - no TileMotif");
        return;
    }

    auto motif = timo->getMotif();
    motif->setVersion(version);

    // delegate lower third editor
    delegateLowerThird(timo);

    // tigger rebuild and paint
    frontThirdWidget->getCurrentEditor()->motifModified(motif);
}


void MotifEditorWidget::slot_addExtender()
{
    auto ed = frontThirdWidget->getCurrentEditor();
    if (ed)
    {
        ed->addExtender();

        auto del = delegatedTileMotif.lock();
        delegateLowerThird(del,true);
        frontThirdWidget->getCurrentEditor()->motifModified(del->getMotif());
        frontThirdWidget->setCurrentIndex(1);
    }
}

void MotifEditorWidget::slot_deleteExtender()
{
    MotifEditor * ed = frontThirdWidget->getCurrentEditor();
    if (ed)
    {
        ed->deleteExtender();

        auto del  = delegatedTileMotif.lock();
        delegateLowerThird(del,true);
        frontThirdWidget->getCurrentEditor()->motifModified(del->getMotif());
    }
}

void MotifEditorWidget::slot_cleanse()
{
    auto timo  = delegatedTileMotif.lock();
    if (!timo) return;
    auto motif = timo->getMotif();
    auto map   = motif->getMotifMap();
    auto level = motif->getCleanse();
    auto sens  = motif->getCleanseSensitivity();

    DlgCleanse  dlg(map,level,sens,this);
    auto rv = dlg.exec();
    if (rv == QDialog::Accepted)
    {
        level = dlg.fromCheckboxes();
        sens  = dlg.getSsnsitivity();
        motif->setCleanse(level);
        motif->setCleanseSensitivity(sens);
        frontThirdWidget->getCurrentEditor()->motifModified(timo->getMotif());
        //motif->resetMotifMap();
        //motif->buildMotifMap();
    }
}

void MotifEditorWidget::slot_swapTileRegularity()
{
    auto timo  = delegatedTileMotif.lock();
    if (!timo) return;

    auto tile  = timo->getTile();
    tile->flipRegularity();

    MotifEvent pevent;
    pevent.event  = MotifMaker::Event::UniqueTileRegularityChanged;
    pevent.tiling = timo->getTiling();
    pevent.tile = tile;
    Sys::motifMaker->sm_takeUp(pevent);

    TileMotifBtnPtr btn = parent->getButton(timo);
    parent->delegate(btn,true);

    Sys::render(RENDER_RESET_MOTIFS);
}

void MotifEditorWidget::slot_lockBtnView(bool checked)
{
    Sys::motifMakerView->setDontAutoFit(checked);
}

void MotifEditorWidget::slot_cropToTile(bool checked)
{
    auto timo = delegatedTileMotif.lock();
    if (!timo) return;

    auto adaptive = std::dynamic_pointer_cast<AdaptiveMotif>(timo->getMotif());
    if (!adaptive) return;

    adaptive->cropToTile = checked;

    frontThirdWidget->getCurrentEditor()->motifModified(adaptive);
}
