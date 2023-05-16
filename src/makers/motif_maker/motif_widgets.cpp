#include <QDebug>
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/motif_widgets.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "widgets/panel_misc.h"
#include "widgets/layout_sliderset.h"
#include "panels/page_motif_maker.h"
#include "motifs/motif.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "settings/configuration.h"

Q_DECLARE_METATYPE(NamedMotifEditor *)

/////////////////////////////////////////////////////////
///
/// MotifEditorWidget
///
////////////////////////////////////////////////////////

SpecificEditorWidget::SpecificEditorWidget()
{
}

SpecificEditorWidget::SpecificEditorWidget(NamedMotifEditor * fe)
{
    setFixedWidth(600);
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);
    setLayout(aLayout);
}

void SpecificEditorWidget::setEditor(NamedMotifEditor * fe)
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
///
/// MotifTypeChoiceCombo
///
////////////////////////////////////////////////////////


// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

NamedMotifEditor::NamedMotifEditor(QString motifName)
{
    name = motifName;

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    boundarySides = new SliderSet("Extended Boundary sides", 4, 1, 64);
    boundaryScale = new DoubleSliderSet("Extended Boundary Scale", 1.0, 0.1, 4.0, 100 );
    motifScale    = new DoubleSliderSet("Motif Scale", 1.0, 0.1, 4.0, 100 );
    motifRotate   = new DoubleSliderSet("Motif Rotation",0.0, -360.0, 360.0, 1);
    motifSides    = new SliderSet("Motif sides", 6, 1, 64);

    addLayout(boundarySides);
    addLayout(boundaryScale);
    addLayout(motifScale);
    addLayout(motifRotate);
    addLayout(motifSides);

    connect(this,          &NamedMotifEditor::sig_motif_modified, this, &NamedMotifEditor::slot_motifModified); //, Qt::QueuedConnection);

    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(boundarySides, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(motifSides,    &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(motifScale,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(motifRotate,   &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
}

void NamedMotifEditor::setMotif(MotifPtr motif, bool doEmit)
{
    Q_UNUSED(doEmit);
    wMotif = motif;
}

void NamedMotifEditor::motifToEditor()
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    const ExtendedBoundary & eb = motif->getExtendedBoundary();
    int    bs = eb.sides;
    qreal  sc = eb.scale;
    qreal  fs = motif->getMotifScale();
    qreal  rr = motif->getMotifRotate();
    int    nn = motif->getN();

    blockSignals(true);
    boundarySides->setValues(bs, 1, 64);
    boundaryScale->setValues(sc, 0.1, 4.0);
    motifScale->setValues(fs, 0.1, 4.0);
    motifRotate->setValues(rr,-360.0,360.0);
    motifSides->setValues(nn, 1, 64);
    blockSignals(false);
}

void NamedMotifEditor::editorToMotif(bool doEmit)
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    int   bsides = boundarySides->value();
    qreal bscale = boundaryScale->value();
    qreal fscale = motifScale->value();
    qreal rot    = motifRotate->value();
    int   sides  = motifSides->value();

    ExtendedBoundary & eb = motif->getRWExtendedBoundary();

    blockSignals(true);
    eb.sides = bsides;
    eb.scale = bscale;
    motif->setMotifScale(fscale);
    motif->setMotifRotate(rot);
    motif->setN(sides);
    blockSignals(false);

    if (doEmit)
        emit sig_motif_modified(motif);
}

void NamedMotifEditor::slot_motifModified(MotifPtr motif)
{
    auto protomaker = PrototypeMaker::getInstance();
    auto data       = protomaker->getProtoMakerData();
    auto del        = data->getSelectedDEL();
    if (!del)
    {
        return;
    }

    del->setMotif(motif);

    // notify motif maker
    bool multi = Configuration::getInstance()->motifMultiView;
    data->select(MVD_DELEM,del,multi);     // if this is the samne design element this does nothing

    auto tiling = data->getSelectedPrototype()->getTiling();
    protomaker->sm_takeUp(tiling,PROM_MOTIF_CHANGED);
    
    data->select(MVD_DELEM,del,multi);
}

/////////////////////////////////////////////////////////
///
/// MotifTypeChoiceCombo
///
////////////////////////////////////////////////////////

MotifTypeCombo:: MotifTypeCombo()
{
    setFixedWidth(221);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,   &MotifTypeCombo::slot_motifTypeSelected);
}

void MotifTypeCombo::updateChoices(MotifPtr motif)
{
    blockSignals(true);

    clear();

    if (motif->isRadial())
    {
        add(MOTIF_TYPE_STAR,                "Radial Star");
        add(MOTIF_TYPE_CONNECT_STAR,        "Radial Star Connect");
        add(MOTIF_TYPE_EXTENDED_STAR,       "Radial Star Extended");
        add(MOTIF_TYPE_ROSETTE,             "Radial Rosette");
        add(MOTIF_TYPE_CONNECT_ROSETTE,     "Radial Rosette Connect");
        add(MOTIF_TYPE_EXTENDED_ROSETTE,    "Raidal Rosette Extended");
        add(MOTIF_TYPE_EXPLCIT_TILE,        "Explicit Tile");
    }
    else
    {
        add(MOTIF_TYPE_EXPLICIT_MAP,        "Explicit Map");
        add(MOTIF_TYPE_INFERRED,            "Inferred");
        add(MOTIF_TYPE_IRREGULAR_STAR,      "Irregular Star");
        add(MOTIF_TYPE_IRREGULAR_ROSETTE,   "Irregular Rosette");
        add(MOTIF_TYPE_HOURGLASS,           "Hourglass");
        add(MOTIF_TYPE_GIRIH,               "Girih");
        add(MOTIF_TYPE_INTERSECT,           "Intersect");
        add(MOTIF_TYPE_EXPLCIT_TILE,        "Explicit Tile");
        add(MOTIF_TYPE_IRREGULAR_NO_MAP,    "No Motif");
    }

    int index = getIndex(motif->getMotifType());
    if (index >= 0)
    {
        setCurrentIndex(index);
        qDebug() << "MotifTypeCombo - set index=" << index;
    }
    else
    {
        setCurrentIndex(0);
        qDebug() << "MotifTypeCombo - forcing cindex=0";
    }

    blockSignals(false);
}

void MotifTypeCombo::add(eMotifType type, QString name)
{
    addItem(name,QVariant(type));
}

int MotifTypeCombo::getIndex(eMotifType type)
{
    qDebug().noquote()  << "type is" << Motif::getTypeString(type);

    return findData(QVariant(type));
}

void MotifTypeCombo::slot_motifTypeSelected(int index)
{
    Q_UNUSED(index);

    QVariant qv = currentData();
    eMotifType type = static_cast<eMotifType>(qv.toInt());
    qDebug() << "MotifEditor type="  << sMotifType[type];

    emit sig_motifTypeChanged(type);
}

