#include <QDebug>

#include "model/motifs/motif.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/motif_editors.h"
#include "gui/model_editors/motif_edit/irregular_motif_editors.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/design_element.h"
#include "sys/tiledpatternmaker.h"
#include "model/settings/configuration.h"
#include "gui/widgets/dlg_cleanse.h"

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MotifEditorWidget::MotifEditorWidget()
{
    // combo to select motif type for tile
    typeCombo              = new MotifTypeCombo();

                  cleanBtn = new QPushButton("Clean");
               cleanStatus = new QLabel();

                  QPushButton * addExt   = new QPushButton("Add Extender");
    QPushButton * delExt   = new QPushButton("Delete Extender");
    QPushButton * addCon   = new QPushButton("Add Connector");
    QLabel      * lMot     = new QLabel("Motif: ");

    connect(typeCombo, &MotifTypeCombo::sig_motifTypeChanged, this, &MotifEditorWidget::slot_motifTypeChanged);
    connect(addExt,    &QPushButton::clicked,                 this, &MotifEditorWidget::slot_addExtender);
    connect(delExt,    &QPushButton::clicked,                 this, &MotifEditorWidget::slot_deleteExtender);
    connect(addCon,    &QPushButton::clicked,                 this, &MotifEditorWidget::slot_addConnector);
    connect(cleanBtn,  &QPushButton::clicked,                 this, &MotifEditorWidget::slot_cleanse);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(lMot);
    hbox->addWidget(typeCombo);
    hbox->addStretch();
    hbox->addWidget(cleanBtn);
    hbox->addWidget(cleanStatus);

    hbox->addStretch();
    hbox->addWidget(addExt);
    hbox->addWidget(delExt);
    hbox->addWidget(addCon);

    // Panel containing the editors.
    specificEditorWidget = new SpecificEditorWidget();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(hbox);
    layout->addWidget(specificEditorWidget);
    setLayout(layout);
}

void MotifEditorWidget::delegate(DesignElementPtr del)
{
    delegatedDesignElement = del;

    if (!del)
    {
        specificEditorWidget->setEditor(nullptr);
        return;
    }

    MotifPtr motif = del->getMotif();
    if (!motif)
    {
        specificEditorWidget->setEditor(nullptr);
        return;
    }

    // DAC note:  When a design element is creeated from a tile in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "MotifEditor::selectMotif :" << motif->getMotifDesc();

    typeCombo->updateChoices(motif);

    eMotifType motifType = motif->getMotifType();
    delegate(del,motifType,false);
}

void MotifEditorWidget::delegate(DesignElementPtr del,eMotifType type,bool doEmit)
{
    NamedMotifEditor * ed = nullptr;
    switch (type)
    {
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
        qCritical("unexpected motif type");
        break;
    case MOTIF_TYPE_STAR:
        ed = new StarEditor("radial_star_edit",del,doEmit);
        break;
    case MOTIF_TYPE_STAR2:
        ed = new Star2Editor("radial_star2_edit",del,doEmit);
        break;
    case MOTIF_TYPE_ROSETTE:
        ed = new RosetteEditor("radial_rosette_edit",del,doEmit);
        break;
    case MOTIF_TYPE_ROSETTE2:
        ed = new Rosette2Editor("radial_rosette2_edit",del,doEmit);
        break;
    case MOTIF_TYPE_EXPLICIT_MAP:
        ed = new ExplicitMapEditor("explcit_map_edit",del,doEmit);
        break;
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
        ed = new IrregularNoMapEditor("irregular_npomap_edit",del,doEmit);
        break;
    case MOTIF_TYPE_INFERRED:
        ed = new InferEditor("infer_edit",del,doEmit);
        break;
    case MOTIF_TYPE_IRREGULAR_ROSETTE:
        ed = new IrregularRosetteEditor("irregular_rosette_edit",del,doEmit);
        break;
    case MOTIF_TYPE_HOURGLASS:
        ed = new HourglassEditor("hourglass_edit",del,doEmit);
        break;
    case MOTIF_TYPE_INTERSECT:
        ed = new IntersectEditor("intersect_edit",del,doEmit);
        break;
    case MOTIF_TYPE_GIRIH:
        ed = new GirihEditor("girih_edit",del,doEmit);
        break;
    case MOTIF_TYPE_IRREGULAR_STAR:
        ed = new IrregularStarEditor("irregular_star_edit",del,doEmit);
        break;
    case MOTIF_TYPE_EXPLCIT_TILE:
        ed = new ExplicitTileEditor("explicit_tile_edit",del,doEmit);
        break;
    }

    specificEditorWidget->setEditor(ed);

    adjustSize();
}

// the type combo has changed
void MotifEditorWidget::slot_motifTypeChanged(eMotifType type)
{
    auto del = delegatedDesignElement.lock();
    if (!del)
    {
        qWarning("MotifEditor::motifChoiceSelected - no design element");
        return;
    }

    delegate(del,type,true);
    Sys::prototypeMaker->select(MVD_DELEM,del,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED));
}

void MotifEditorWidget::slot_addConnector()
{
    auto ed = specificEditorWidget->getEditor();
    if (ed)
    {
        ed->addConnector();

        auto del = delegatedDesignElement.lock();
        auto type = del->getMotif()->getMotifType();
        delegate(del,type,true);
    }
}

void MotifEditorWidget::slot_addExtender()
{
    auto ed = specificEditorWidget->getEditor();
    if (ed)
    {
        ed->addExtender();

        auto del = delegatedDesignElement.lock();
        auto type = del->getMotif()->getMotifType();
        delegate(del,type,true);
    }
}

void MotifEditorWidget::slot_deleteExtender()
{
    auto ed = specificEditorWidget->getEditor();
    if (ed)
    {
        ed->deleteExtender();

        auto del = delegatedDesignElement.lock();
        auto type = del->getMotif()->getMotifType();
        delegate(del,type,true);
    }
}

void MotifEditorWidget::slot_cleanse()
{
    auto del = delegatedDesignElement.lock();
    if (!del) return;
    auto motif = del->getMotif();
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
        motif->resetMotifMap();
        motif->buildMotifMap();
    }
}

void MotifEditorWidget::onRefresh()
{
    auto del = delegatedDesignElement.lock();
    if (!del) return;
    auto motif = del->getMotif();
    if (motif->getCleanse() > 0)
    {
        cleanBtn->setText("Cleanse : ON ");
        uint level = motif->getCleanse();
        qreal sensitivity = motif->getCleanseSensitivity();
        QString str = QString("0x%1 : %2").arg(QString::number(level,16)).arg(sensitivity);
        cleanStatus->setText(str);
    }
    else
    {
        cleanBtn->setText("Cleanse : OFF");
        cleanStatus->setText("");
    }
}
