#include <qglobal.h>
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "gui/model_editors/motif_edit/design_element_selector.h"
#include "model/prototypes//prototype.h"
#include "model/makers/prototype_maker.h"
#include "sys/sys.h"
#include "model/prototypes/design_element.h"
#include "gui/top/system_view.h"

MotifMakerWidget::MotifMakerWidget() : QWidget()
{
    setContentsMargins(0,0,0,0);

    protoMaker = Sys::prototypeMaker;

    // Motif buttons
    delSelector = new DELSelectorWidget(this);

    // larger slected feature button
    viewerBtn  = new DesignElementButton(-1);

    // top row
    QHBoxLayout * btnBox = new QHBoxLayout();
    btnBox->addWidget(delSelector);
    btnBox->addSpacing(25);
    btnBox->addWidget(viewerBtn);
    btnBox->addStretch();

    // Editors
    motifEditor  = new MotifEditorWidget();

    AQVBoxLayout * motifBox = new AQVBoxLayout;
    motifBox->addLayout(btnBox);
    motifBox->addWidget(motifEditor);

    setLayout(motifBox);
    setMinimumWidth(610);

    connect(this, &MotifMakerWidget::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);
}

void MotifMakerWidget::refreshMotifMakerWidget()
{
    static WeakProtoPtr _prototype;
    static int          _numButtons = 0;

    auto proto = protoMaker->getSelectedPrototype();

    if (proto && ((_prototype.lock() != proto) || (proto->numDesignElements() != _numButtons) || protoMaker->forceWidgetRefresh()))
    {
        protoMaker->setForceWidgetRefresh(false);

        _prototype  = proto;
        _numButtons = proto->numDesignElements();

        delSelector->setup(proto);

        if (_numButtons)
        {
            auto del = proto->getDesignElements().last();
            auto btn = delSelector->getButton(del);
            delegate(btn,false,true);   // start off with a single selection
        }
    }
    update();
}

// when a DEL button is selected, this delgates the motif in both the motif maker and the motif editor
void MotifMakerWidget::delegate(DELBtnPtr btn, bool add, bool set)
{
    if (!btn)
    {
        qWarning("No DEL button to delgate");
        return;
    }

    qDebug() << "MotifMakerWidget::delegate btn=" << btn->getIndex() << "multi" << add << "set" << set;

    DELPtr designElement = btn->getDesignElement(); // DAC taprats cloned here

    if (set)
    {
        if (!designElement) return;

        // Deaign Element Viewer button - do this first
        viewerBtn->setDesignElement(designElement);
        viewerBtn->setViewTransform();

        // Prototype Maker data
        protoMaker->select(  MVD_DELEM,designElement,add);

        // DEL selector
        btn->setViewTransform();
        delSelector->delegate(btn,add,set);
        delSelector->tallyButtons();

        // Motif Editor
        motifEditor->delegate(designElement);
    }
    else
    {
        // Design Element Viewer button - do this first
        DELPtr np;
        viewerBtn->setDesignElement(np);

        // Prototype Maker data
        protoMaker->deselect(MVD_DELEM,designElement,add);

        // DEL selector
        delSelector->delegate(btn,add,set);
        delSelector->tallyButtons();

        // Motif Editor
        motifEditor->delegate(np);
    }

    // view
    emit sig_updateView();
}


void MotifMakerWidget::update()
{
    delSelector->update();
    viewerBtn->update();
    QWidget::update();
}
