#pragma once
#ifndef MOTIFMAKERWIDGET_H
#define MOTIFMAKERWIDGET_H

#include <QWidget>
#include "gui/model_editors/motif_edit/design_element_selector.h"

typedef std::shared_ptr<class DesignElement>       DELPtr;
typedef std::shared_ptr<class DesignElementButton> DELBtnPtr;
typedef std::shared_ptr<class Motif>               MotifPtr;
typedef std::shared_ptr<class Prototype>           ProtoPtr;
typedef std::weak_ptr<class Prototype>          WeakProtoPtr;

// This widget is housed by page_motif_maker.
// It provides selection and display of DesignElements (DELs)
// and its maker (controller) functions both select motifs and modify them

// This widget coordinates delegation/selection of
//  1. Motif View Button
//  2. motifViewData (ProtoData)
//  3. DEL selector
//  4. Motif Editor

// However DELs (particularly their tiles) can
// be changed externally, so the particular DEL must be
// refreshed here and the only way to do this is to select
// the DEL and rebuild the motif

class MotifMakerWidget : public QWidget
{
    Q_OBJECT

public:
    MotifMakerWidget();

    void refreshMotifMakerWidget();

    void delegate(DELBtnPtr btn, bool add, bool set);

    DELBtnPtr getButton(DELPtr del) { return delSelector->getButton(del); }
    DELBtnPtr getDelegatedButton()            { return delSelector->getDelegated(); }

    void   update();

    class MotifEditorWidget * getMotifEditor() { return motifEditor; }

signals:
    void sig_updateView();

private:
    class PrototypeMaker      * protoMaker;

    DELSelectorWidget         * delSelector;        // LHS top
    class DesignElementButton * viewerBtn;          // rhs top
    class MotifEditorWidget   * motifEditor;        // bottom
};

#endif // MOTIFMAKERWIDGET_H
