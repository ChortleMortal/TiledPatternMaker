#include <QDebug>
#include <QGroupBox>

#include "gui/model_editors/motif_edit/extender_editor.h"
#include "gui/model_editors/motif_edit/motif_editor.h"
#include "gui/model_editors/motif_edit/radial_motif_editors.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/panel_misc.h"
#include "model/makers/motif_maker.h"
#include "model/motifs/motif.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"

Q_DECLARE_METATYPE(MotifEditor *)

/////////////////////////////////////////////////////////
///
/// MotifEditor
///
////////////////////////////////////////////////////////

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

MotifEditor::MotifEditor(QString motifName)
{
    name  = motifName;

    setContentsMargins(9,5,9,5);

    parent        = nullptr;
    connectScale  = nullptr;

    motifSides     = new SliderSet("Sides", 6, 1, 64);
    motifScale     = new DoubleSliderSet("Scale", 1.0, 0.1, 9.0, 100 );
    motifRotate    = new DoubleSliderSet("Rotate",0.0, -360.0, 360.0, 1);

    vbox  = new AQVBoxLayout();
    vbox->addLayout(motifSides);
    vbox->addLayout(motifRotate);
    vbox->addLayout(motifScale);

    setLayout(vbox);

    connect(this,       &MotifEditor::sig_updateView,      Sys::systemViewController, &ViewController::slot_updateView);
    connect(motifSides, &SliderSet::valueChanged,               this, [this]() { editorToMotif(true);});
    connect(motifScale, &DoubleSliderSet::valueChanged,         this, [this]() { editorToMotif(true);});
    connect(motifRotate,&DoubleSliderSet::valueChanged,         this, [this]() { editorToMotif(true);});
}

MotifEditor::~MotifEditor()
{}

void MotifEditor::setMotif(MotifPtr motif)
{
    wMotif = motif;
    //qDebug() << "MotifEditor motif set" << this;;
}

QList<QWidget *>  MotifEditor::setupExtenders(MotifPtr motif)
{
    QList<QWidget*> widgets;
    for (const ExtenderPtr &ep : motif->getExtenders())
    {
        QVBoxLayout * vbox = new QVBoxLayout;

        ExtenderEditor * ed = new ExtenderEditor(ep);
        extenderEds.push_back(ed);

        QVBoxLayout * vbox2 = ed->createExtenderLayout(this);
        vbox->addLayout(vbox2);
        vbox->addStretch();

        QWidget * w = new QWidget();
        w->setLayout(vbox);
        widgets.push_back(w);
    }
    return widgets;
}

void MotifEditor::refresh()
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    qreal  msc = motif->getMotifScale();
    qreal  mro = motif->getMotifRotate();
    int    ms  = motif->getN();

    blockSignals(true);

    motifSides->setValue(ms);
    motifScale->setValue(msc);
    motifRotate->setValue(mro);

    blockSignals(false);

    for (auto & ed : extenderEds)
    {
        ed->refresh();
    }
}

void MotifEditor::editorToMotif(bool doEmit)
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    int   msides = motifSides->value();
    qreal mscale = motifScale->value();
    qreal mrot   = motifRotate->value();

    motif->setN(msides);
    motif->setMotifScale(mscale);
    motif->setMotifRotate(mrot);

    for (auto & ed : extenderEds)
    {
        ed->editorToExtender();
    }

    if (doEmit)
        motifModified(motif);
}

void MotifEditor::motifModified(MotifPtr motif)
{
    qInfo() << "MotifEditor::slot_motifModified" << this;

    motif->resetMotifMap();
    motif->build();

    auto timo = Sys::motifMaker->motifData.getDelegatedTileMotif();
    if (!timo)
    {
        return;
    }

    timo->setMotif(motif);

    // notify motif maker
    Sys::motifMaker->motifData.show(timo);     // if this is the samne design element this does nothing

    auto tiling = Sys::motifMaker->motifData.getActivePrototype()->getTiling();
    MotifEvent motifEvent;
    motifEvent.event = MotifMaker::Event::MotifChanged;
    motifEvent.tiling = tiling;
    Sys::motifMaker->sm_takeUp(motifEvent);
    
    //data->select(MVD_DELEM,del,multi);
    qInfo() << "MotifEditor::slot_motifModified calling sig_updateView";
    emit sig_updateView();
}

// called by MotiEditorWidget::slot_addConnector
void MotifEditor::addExtender()
{
    auto motif = wMotif.lock();
    if (motif)
    {
        auto ext = motif->createExtender();
        motif->addExtender(ext);
    }
}

// called by MotiEditorWidget::slot_deleteConnector
void MotifEditor::deleteExtender()
{
    // deletes the last extender
    auto motif = wMotif.lock();
    if (motif)
    {
        auto extenders = motif->getExtenders();
        if (extenders.count())
        {
            ExtenderPtr ep = extenders.last();
            if (ep)
            {
                motif->deleteExtender(ep);
            }
        }
    }
}

