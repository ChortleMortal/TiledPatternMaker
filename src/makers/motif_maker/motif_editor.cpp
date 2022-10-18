#include <QDebug>

#include "motifs/motif.h"
#include "makers/motif_maker/explicit_motif_editors.h"
#include "makers/motif_maker/motif_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"

Q_DECLARE_METATYPE(NamedMotifEditor *)

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MotifEditor::MotifEditor(page_motif_maker * menu)
{
    config = Configuration::getInstance();

    // Explicit motif editors.
    explicit_edit           = new ExplicitEditor(menu,"explcit_edit");
    explcit_infer_edit      = new ExplicitInferEditor(menu,"explcit_infer_edit");
    explict_star_edit       = new ExplicitStarEditor(menu,"explict_star_edit");
    explicit_rosette_edit   = new ExplicitRosetteEditor(menu,"explicit_rosette_edit");
    explicit_hourglass_edit = new ExplicitHourglassEditor(menu,"explicit_hourglass_edit");
    explicit_girih_edit     = new ExplicitGirihEditor(menu,"explicit_girih_edit");
    explicit_intersect_edit = new ExplicitIntersectEditor(menu,"explicit_intersect_edit");
    explicit_tile_edit      = new ExplicitTileEditor(menu,"explicit_tile_edit");

    // Radial motif editors.
    radial_star_edit       = new StarEditor(menu,"radial_star_edit");
    radial_rosette_edit    = new RosetteEditor(menu,"radial_rosette_edit");
    connect_rosette_edit   = new ConnectRosetteEditor(menu,"connect_rosette_edit");
    connect_star_edit      = new ConnectStarEditor(menu,"connect_star_edit");
    ex_star_edit           = new ExtendedStarEditor(menu,"ex_star_edit");
    ex_rosette_edit        = new ExtendedRosetteEditor(menu,"ex_rosette_edit");

    // Panel containing the editors.
    mfw = new MotifWidget();

    choiceCombo = new MotifTypeChoiceCombo(this);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(choiceCombo);
    hbox->addStretch();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mfw);
    layout->addLayout(hbox);
    setLayout(layout);
}

void MotifEditor::selectMotif(DesignElementPtr del)
{
    currentDesignElement = del;
    MotifPtr motif = del->getMotif();
    if (!motif)
    {
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
        return;
    }

    // DAC note:  When a design element is creeated from a tile in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "MotifEditor::selectMotif :" << motif->getMotifDesc();

    choiceCombo->updateChoices(motif);

    eMotifType motiofType = motif->getMotifType();
    switch (motiofType)
    {
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
        qCritical("unexpected motif type");
        break;
    case MOTIF_TYPE_EXTENDED_STAR:
        selectCurrentEditor(ex_star_edit);
        ex_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXTENDED_ROSETTE:
        selectCurrentEditor(ex_rosette_edit);
        ex_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_STAR:
        selectCurrentEditor(radial_star_edit);
        radial_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_ROSETTE:
        selectCurrentEditor(radial_rosette_edit);
        radial_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_CONNECT_ROSETTE:
        selectCurrentEditor(connect_rosette_edit);
        connect_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_CONNECT_STAR:
        selectCurrentEditor(connect_star_edit);
        connect_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT:
        selectCurrentEditor(explicit_edit);
        explicit_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_INFER:
        selectCurrentEditor(explcit_infer_edit);
        explcit_infer_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_ROSETTE:
        selectCurrentEditor(explicit_rosette_edit);
        explicit_rosette_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_HOURGLASS:
        selectCurrentEditor(explicit_hourglass_edit);
        explicit_hourglass_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_INTERSECT:
        selectCurrentEditor(explicit_intersect_edit);
        explicit_intersect_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_GIRIH:
        selectCurrentEditor(explicit_girih_edit);
        explicit_girih_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_STAR:
        selectCurrentEditor(explict_star_edit);
        explict_star_edit->setMotif(del,false);
        break;
    case MOTIF_TYPE_EXPLICIT_TILE:
        selectCurrentEditor(explicit_tile_edit);
        explicit_tile_edit->setMotif(del,false);
        break;
    }
}

void MotifEditor::selectCurrentEditor(NamedMotifEditor* fe)
{
    mfw->setEditor(fe);
    adjustSize();
}

void MotifEditor::slot_motifTypeChanged(eMotifType type)
{
    if (!currentDesignElement.lock())
    {
        qWarning("MotifEditor::motifChoiceSelected - no design element");
        return;
    }

    NamedMotifEditor * editor = getEditor(type);
    if (editor)
    {
        selectCurrentEditor(editor);
        editor->setMotif(currentDesignElement.lock(),true);
    }
}

NamedMotifEditor * MotifEditor ::getEditor(eMotifType type)
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
    case MOTIF_TYPE_EXPLICIT:
        return explicit_edit;
    case MOTIF_TYPE_EXPLICIT_INFER:
        return explcit_infer_edit;
    case MOTIF_TYPE_EXPLICIT_ROSETTE:
        return explicit_rosette_edit;
    case MOTIF_TYPE_EXPLICIT_HOURGLASS:
        return explicit_hourglass_edit;
    case MOTIF_TYPE_EXPLICIT_INTERSECT:
        return explicit_intersect_edit;
    case MOTIF_TYPE_EXPLICIT_GIRIH:
        return  explicit_girih_edit;
    case MOTIF_TYPE_EXPLICIT_STAR:
        return explict_star_edit;
    case MOTIF_TYPE_EXPLICIT_TILE:
        return explicit_tile_edit;
    }
    qCritical("Unexpected motif type (2");
    return nullptr;
}

/////////////////////////////////////////////////////////
/// MotifmotifWidget
////////////////////////////////////////////////////////

MotifWidget::MotifWidget()
{
}

MotifWidget::MotifWidget(NamedMotifEditor * fe)
{
    setFixedWidth(600);
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);
    setLayout(aLayout);
}

void MotifWidget::setEditor(NamedMotifEditor * fe)
{
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);

    QLayout * l = layout();
    if (l)
    {
        QLayoutItem * item;
        while ( (item = l->itemAt(0)) != nullptr)
        {
            QWidget * w = item->widget();
            if (w)
            {
                w->setParent(nullptr);
            }
        }
        delete l;
    }
    setLayout(aLayout);
    adjustSize();
}


/////////////////////////////////////////////////////////
/// FigTypeChoiceCombo
////////////////////////////////////////////////////////

MotifTypeChoiceCombo:: MotifTypeChoiceCombo(MotifEditor *editor)
{
    setFixedWidth(221);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,   &MotifTypeChoiceCombo::slot_motifTypeSelected);
    connect(this, &MotifTypeChoiceCombo::sig_motifTypeChanged,          editor, &MotifEditor::slot_motifTypeChanged);
}

void MotifTypeChoiceCombo::updateChoices(MotifPtr motif)
{
    blockSignals(true);

    clear();

    if (motif->isRadial())
    {
        addChoice(MOTIF_TYPE_STAR,                "Star");
        addChoice(MOTIF_TYPE_CONNECT_STAR,        "Star Connect");
        addChoice(MOTIF_TYPE_EXTENDED_STAR,       "Star Extended");
        addChoice(MOTIF_TYPE_ROSETTE,             "Rosette");
        addChoice(MOTIF_TYPE_CONNECT_ROSETTE,     "Rosette Connect");
        addChoice(MOTIF_TYPE_EXTENDED_ROSETTE,    "Rosette Extended");
        addChoice(MOTIF_TYPE_EXPLICIT_TILE,    "Explicit Tile");
    }
    else
    {
        addChoice(MOTIF_TYPE_EXPLICIT,            "Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_INFER,      "Infer Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_STAR,       "Star Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_ROSETTE,    "Rosette Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_HOURGLASS,  "Hourglass Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_GIRIH,      "Girih Tiles Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_INTERSECT,  "Intersect Explicit");
        addChoice(MOTIF_TYPE_EXPLICIT_TILE,    "Explicit Tile");
    }

    int index = getChoiceIndex(motif->getMotifType());
    if (index >= 0)
    {
        setCurrentIndex(index);
        qDebug() << "MotifEditor - set choice index=" << index;
    }
    else
    {
        setCurrentIndex(0);
        qDebug() << "MotifEditor - forcing choice index=0";
    }

    blockSignals(false);
}

void MotifTypeChoiceCombo::addChoice(eMotifType type, QString name)
{
    addItem(name,QVariant(type));
}

int MotifTypeChoiceCombo::getChoiceIndex(eMotifType type)
{
    qDebug().noquote()  << "type is" << Motif::getTypeString(type);

    return findData(QVariant(type));
}

void MotifTypeChoiceCombo::slot_motifTypeSelected(int index)
{
    Q_UNUSED(index);

    QVariant qv = currentData();
    eMotifType type = static_cast<eMotifType>(qv.toInt());
    qDebug() << "MotifEditor type="  << sTileType[type];

    emit sig_motifTypeChanged(type);
}
