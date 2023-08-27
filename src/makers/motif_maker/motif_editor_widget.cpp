#include <QDebug>

#include "motifs/motif.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/motif_maker_widgets.h"
#include "makers/motif_maker/irregular_motif_editors.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "tiledpatternmaker.h"
#include "settings/configuration.h"

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MotifEditorWidget::MotifEditorWidget()
{
    protoMakerData  = PrototypeMaker::getInstance()->getProtoMakerData();
    config         = Configuration::getInstance();

    // Explicit motif editors.
    explicit_map_edit       = new ExplicitMapEditor("explcit_map_edit");
    irregular_nomap_edit    = new IrregularNoMapEditor("irregular_npomap_edit");
    infer_edit              = new InferEditor("infer_edit");
    irregular_star_edit     = new IrregularStarEditor("irregular_star_edit");
    irregular_rosette_edit  = new IrregularRosetteEditor("irregular_rosette_edit");
    hourglass_edit          = new HourglassEditor("hourglass_edit");
    girih_edit              = new GirihEditor("girih_edit");
    intersect_edit          = new IntersectEditor("intersect_edit");
    explicit_tile_edit      = new ExplicitTileEditor("explicit_tile_edit");

    // Radial motif editors.
    radial_star_edit       = new StarEditor("radial_star_edit");
    radial_rosette_edit    = new RosetteEditor("radial_rosette_edit");
    connect_rosette_edit   = new ConnectRosetteEditor("connect_rosette_edit");
    connect_star_edit      = new ConnectStarEditor("connect_star_edit");
    ex_star_edit           = new ExtendedStarEditor("ex_star_edit");
    ex_rosette_edit        = new ExtendedRosetteEditor("ex_rosette_edit");

    // combo to select motif type for tile
    typeCombo              = new MotifTypeCombo();

    connect(typeCombo, &MotifTypeCombo::sig_motifTypeChanged, this, &MotifEditorWidget::slot_motifTypeChanged);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(typeCombo);
    hbox->addStretch();

    // Panel containing the editors.
    specificEditorWidget = new SpecificEditorWidget();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(specificEditorWidget);
    layout->addLayout(hbox);
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
#if 0
        explicit_edit->setMotif(del,false);
        explcit_infer_edit->setMotif(del,false);
        explict_star_edit->setMotif(del,false);
        explicit_rosette_edit->setMotif(del,false);
        explicit_hourglass_edit->setMotif(del,false);
        explicit_girih_edit->setMotif(del,false);
        explicit_intersect_edit->setMotif(del,false);
        explicit_tile_edit->setMotif(del,false);

        radial_star_edit->setMotif(del,false);
        radial_rosette_edit->setMotif(del,false);
        connect_rosette_edit->setMotif(del,false);
        connect_star_edit->setMotif(del,false);
        ex_star_edit->setMotif(del,false);
        ex_rosette_edit->setMotif(del,false);
#endif
        return;
    }

    // DAC note:  When a design element is creeated from a tile in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "MotifEditor::selectMotif :" << motif->getMotifDesc();

    typeCombo->updateChoices(motif);

    eMotifType motifType = motif->getMotifType();
    switch (motifType)
    {
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
        qCritical("unexpected motif type");
        break;
    case MOTIF_TYPE_EXTENDED_STAR:
        delgate(ex_star_edit);
        ex_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXTENDED_ROSETTE:
        delgate(ex_rosette_edit);
        ex_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_STAR:
        delgate(radial_star_edit);
        radial_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_ROSETTE:
        delgate(radial_rosette_edit);
        radial_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_CONNECT_ROSETTE:
        delgate(connect_rosette_edit);
        connect_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_CONNECT_STAR:
        delgate(connect_star_edit);
        connect_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_MAP:
        delgate(explicit_map_edit);
        explicit_map_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
        delgate(irregular_nomap_edit);
        irregular_nomap_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_INFERRED:
        delgate(infer_edit);
        infer_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_IRREGULAR_ROSETTE:
        delgate(irregular_rosette_edit);
        irregular_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_HOURGLASS:
        delgate(hourglass_edit);
        hourglass_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_INTERSECT:
        delgate(intersect_edit);
        intersect_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_GIRIH:
        delgate(girih_edit);
        girih_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_IRREGULAR_STAR:
        delgate(irregular_star_edit);
        irregular_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLCIT_TILE:
        delgate(explicit_tile_edit);
        explicit_tile_edit->setMotif(del,false);
        break;
    }
}

void MotifEditorWidget::delgate(NamedMotifEditor* fe)
{
    specificEditorWidget->setEditor(fe);
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

    NamedMotifEditor * editor = getEditor(type);
    if (editor)
    {
        delgate(editor);
        editor->setMotif(del,true);
        protoMakerData->select(MVD_DELEM,del,config->motifMultiView);
    }
}

NamedMotifEditor * MotifEditorWidget ::getEditor(eMotifType type)
{
    switch (type)
    {
    case MOTIF_TYPE_RADIAL:
        break;
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_ROSETTE:
        return radial_rosette_edit;
    case MOTIF_TYPE_STAR:
        return radial_star_edit;
    case MOTIF_TYPE_CONNECT_STAR:
        return connect_star_edit;
    case MOTIF_TYPE_CONNECT_ROSETTE:
        return connect_rosette_edit;
    case MOTIF_TYPE_EXTENDED_ROSETTE:
        return ex_rosette_edit;
    case MOTIF_TYPE_EXTENDED_STAR:
        return ex_star_edit;
    case MOTIF_TYPE_EXPLICIT_MAP:
        return explicit_map_edit;
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
        return irregular_nomap_edit;
    case MOTIF_TYPE_INFERRED:
        return infer_edit;
    case MOTIF_TYPE_IRREGULAR_ROSETTE:
        return irregular_rosette_edit;
    case MOTIF_TYPE_HOURGLASS:
        return hourglass_edit;
    case MOTIF_TYPE_INTERSECT:
        return intersect_edit;
    case MOTIF_TYPE_GIRIH:
        return  girih_edit;
    case MOTIF_TYPE_IRREGULAR_STAR:
        return irregular_star_edit;
    case MOTIF_TYPE_EXPLCIT_TILE:
        return explicit_tile_edit;
    }
    qCritical("Unexpected motif type (2");
    return nullptr;
}

