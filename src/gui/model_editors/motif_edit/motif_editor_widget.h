#pragma once

#include <QComboBox>
#include <QStackedWidget>

#include "gui/model_editors/motif_edit/lower_third_editor.h"
#include "gui/model_editors/motif_edit/motif_editor.h"
#include "gui/model_editors/motif_edit/motif_type_selector.h"

class MotifEditor;
class MotifMakerWidget;
class QCheckBox;

typedef std::shared_ptr<class Motif>       MotifPtr;
typedef std::shared_ptr<class TileMotif>   TileMotifPtr;

typedef std::weak_ptr<class TileMotif>     WeakTileMotifPtr;

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the TileMotif being edited is changed.

class MotifEditorWidget : public QWidget
{
    Q_OBJECT

public:
    MotifEditorWidget(MotifMakerWidget * parent);
    
    void  delegateComboAndLowerThird(TileMotifPtr tileMotif);

    void  refresh();

public slots:
    void slot_motifTypeChanged(Motif::Type type);
    void slot_motifVersionChanged(int version);

protected slots:
    void slot_addExtender();
    void slot_deleteExtender();
    void slot_cleanse();
    void slot_swapTileRegularity();
    void slot_lockBtnView(bool checked);
    void slot_cropToTile(bool checked);

protected:
    void createLayout();

private:
    void delegateLowerThird(TileMotifPtr tileMotif, bool force = false);
    void delegateChangedMotifType(TileMotifPtr tileMotif, Motif::Type type);
    void updateCropCheckbox(MotifPtr motif);

    MotifMakerWidget      * parent;
    WeakTileMotifPtr        delegatedTileMotif;

    // the new editor is always built into whichever of these is currently hidden, and
    // only swapped to visible once fully built and refreshed - so a freshly-created
    // editor's controls are never painted mid-layout (see delegateLowerThird()). This also
    // means thirdStack's sizeHint (the max over both pages) stops growing/shrinking once
    // both slots have each held the largest editor at least once, instead of resizing on
    // every switch between differently-sized editors (e.g. Rosette vs Rosette2).
    QStackedWidget          thirdStack;
    LowerThirdWidget      * lowerThirds[2];
    LowerThirdWidget      * frontThirdWidget;   // currently visible
    LowerThirdWidget      * backThirdWidget;    // currently hidden staging area
    QWidget               * cornerWidget;

    MotifTypeSelector       typeSelector;
    QPushButton           * cleanBtn;
    QCheckBox             * pbLock;
    QCheckBox             * cropChk;
};

