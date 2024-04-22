#include <QDebug>
#include <QGroupBox>
#include "makers/motif_maker/motif_maker_widget.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "motifs/motif.h"
#include "panels/page_motif_maker.h"
#include "settings/configuration.h"
#include "widgets/layout_sliderset.h"
#include "panels/panel_misc.h"

Q_DECLARE_METATYPE(NamedMotifEditor *)

/////////////////////////////////////////////////////////
///
/// MotifEditorWidget
///
////////////////////////////////////////////////////////

SpecificEditorWidget::SpecificEditorWidget()
{
    setContentsMargins(0,0,0,0);

}

SpecificEditorWidget::SpecificEditorWidget(NamedMotifEditor * fe)
{
    setContentsMargins(0,0,0,0);
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

    setContentsMargins(0,0,0,0);

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    QLabel * lExt = new QLabel("Extended Boundary:");
    QLabel * lMot = new QLabel("Motif:");

    motifSides     = new SliderSet("Sides", 6, 1, 64);
    motifScale     = new DoubleSliderSet("Scale", 1.0, 0.1, 9.0, 100 );
    motifRotate    = new DoubleSliderSet("Rotate",0.0, -360.0, 360.0, 1);

    boundarySides  = new SliderSet("Sides", 4, 1, 64);
    boundaryScale  = new DoubleSliderSet("Scale", 1.0, 0.1, 9.0, 100 );
    boundaryRotate = new DoubleSliderSet("Rotate",0.0, -360.0, 360.0, 1);

    QVBoxLayout * v1  = new QVBoxLayout();
    v1->addWidget(lExt);
    v1->addLayout(boundarySides);
    v1->addLayout(boundaryRotate);
    v1->addLayout(boundaryScale);

    QVBoxLayout * v2  = new QVBoxLayout();
    v2->addWidget(lMot);
    v2->addLayout(motifSides);
    v2->addLayout(motifRotate);
    v2->addLayout(motifScale);

    QHBoxLayout * box = new QHBoxLayout();
    box->addLayout(v1);
    box->addSpacing(11);
    box->addLayout(v2);

    vbox->addLayout(box);

    connect(this,          &NamedMotifEditor::sig_motif_modified, this, &NamedMotifEditor::slot_motifModified); //, Qt::QueuedConnection);

    connect(boundarySides, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(boundaryRotate,&DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
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
    int    bs  = eb.getSides();
    qreal  bsc = eb.getScale();
    qreal  bro = eb.getRotate();
    qreal  msc = motif->getMotifScale();
    qreal  mro = motif->getMotifRotate();
    int    ms  = motif->getN();

    blockSignals(true);
    boundarySides->setValue(bs);
    boundaryScale->setValue(bsc);
    boundaryRotate->setValue(bro);
    motifSides->setValue(ms);
    motifScale->setValue(msc);
    motifRotate->setValue(mro);
    blockSignals(false);
}

void NamedMotifEditor::editorToMotif(bool doEmit)
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    int   bsides = boundarySides->value();
    qreal bscale = boundaryScale->value();
    qreal brot   = boundaryRotate->value();
    int   msides = motifSides->value();
    qreal mscale = motifScale->value();
    qreal mrot   = motifRotate->value();

    ExtendedBoundary & eb = motif->getRWExtendedBoundary();
    eb.setSides(bsides);
    eb.setScale(bscale);
    eb.setRotate(brot);
    motif->setN(msides);
    motif->setMotifScale(mscale);
    motif->setMotifRotate(mrot);

    if (doEmit)
        emit sig_motif_modified(motif);
}

void NamedMotifEditor::slot_motifModified(MotifPtr motif)
{
    auto protomaker = Sys::prototypeMaker;
    auto data       = protomaker->getProtoMakerData();
    auto del        = data->getSelectedDEL();
    if (!del)
    {
        return;
    }

    del->setMotif(motif);

    // notify motif maker
    bool multi = Sys::config->motifMultiView;
    data->select(MVD_DELEM,del,multi);     // if this is the samne design element this does nothing

    auto tiling = data->getSelectedPrototype()->getTiling();
    protomaker->sm_takeUp(tiling,PROM_MOTIF_CHANGED);
    
    //data->select(MVD_DELEM,del,multi);
    protomaker->getProtoMakerData()->getWidget()->update();

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
        add(MOTIF_TYPE_STAR,                "Star");
        add(MOTIF_TYPE_STAR2,               "Star2 (New)");
        add(MOTIF_TYPE_CONNECT_STAR,        "Star Connect");
        add(MOTIF_TYPE_EXTENDED_STAR,       "Star Extended");
        add(MOTIF_TYPE_EXTENDED_STAR2,      "Star2 Extended (New)");
        add(MOTIF_TYPE_ROSETTE,             "Rosette");
        add(MOTIF_TYPE_ROSETTE2,            "Rosette2 (New)");
        add(MOTIF_TYPE_CONNECT_ROSETTE,     "Rosette Connect");
        add(MOTIF_TYPE_EXTENDED_ROSETTE,    "Rosette Extended");
        add(MOTIF_TYPE_EXTENDED_ROSETTE2,   "Rosette2 Extended (New)");
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
        add(MOTIF_TYPE_IRREGULAR_NO_MAP,    "No Motif (Empty placeholder)");
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
    qDebug() << "MotifTypeCombo type="  << sMotifType[type];

    emit sig_motifTypeChanged(type);
}

