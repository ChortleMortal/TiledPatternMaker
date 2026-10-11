#include <QDebug>
#include <QGroupBox>

#include "gui/model_editors/motif_edit/lower_third_editor.h"
#include "gui/model_editors/motif_edit/extender_editor.h"
#include "gui/model_editors/motif_edit/radial_motif_editors.h"
#include "model/motifs/motif.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"

/////////////////////////////////////////////////////////
///
/// LowerThirdWidget
///
////////////////////////////////////////////////////////

LowerThirdWidget::LowerThirdWidget()
{
    motifEditor = nullptr;
}

void LowerThirdWidget::set(MotifEditor * editor, MotifPtr motif)
{
    int ctab = currentIndex();

    for (int i = 0; i < tabs.size(); i++)
    {
        QWidget * tab = tabs[i];
        removeTab(i);
        tab->deleteLater();
    }
    tabs.clear();

    // set tab1 - the motif editor
    int i=0;
    insertTab(i++,editor,"Motif Editor");
    motifEditor = editor;
    tabs.push_back(editor);

    // add extenders and connectors
    const QList<QWidget *> widgets = editor->setupExtenders(motif);
    for (QWidget * widget : widgets)
    if (widget)
    {
        insertTab(i++,widget,"Extender");
        tabs.push_back(widget);
    }

    if (motif == weakMotif.lock())
    {
        qDebug() << "restoring tab" << ctab;
        setCurrentIndex(ctab);
    }

    weakMotif = motif;
}

void LowerThirdWidget::refresh()
{
    // refresh the lower third
    if (motifEditor)
    {
        motifEditor->refresh();
    }
}

