#include <qglobal.h>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QDebug>
#endif
#include "makers/motif_maker/motif_maker_widget.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/design_element_selector.h"
#include "makers/prototype_maker//prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "misc/sys.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "viewers/view.h"

MotifMakerWidget::MotifMakerWidget() : QWidget()
{
    setContentsMargins(0,0,0,0);

    protoMakerData = Sys::prototypeMaker->getProtoMakerData();
    config         = Sys::config;
    view           = Sys::view;

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
}

void MotifMakerWidget::selectPrototype()
{
    static WeakProtoPtr _prototype;
    static int          _numButtons = 0;

    auto proto = protoMakerData->getSelectedPrototype();

    if (proto && proto->numDesignElements() && ((_prototype.lock() != proto) || (proto->numDesignElements() != _numButtons)))
    {
        _prototype  = proto;
        _numButtons = proto->numDesignElements();

        delSelector->setup(proto);

        auto del = proto->getDesignElements().last();
        auto btn = delSelector->getButton(del);
        delegate(btn,false,true);   // start off with a single selection
    }
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

    DesignElementPtr designElement = btn->getDesignElement(); // DAC taprats cloned here

    if (set)
    {
        if (!designElement) return;

        // Deaign Element Viewer button - do this first
        viewerBtn->setDesignElement(designElement);
        viewerBtn->setViewTransform();

        // Prototype Maker data
        protoMakerData->select(  MVD_DELEM,designElement,add);

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
        DesignElementPtr np;
        viewerBtn->setDesignElement(np);

        // Prototype Maker data
        protoMakerData->deselect(MVD_DELEM,designElement,add);

        // DEL selector
        delSelector->delegate(btn,add,set);
        delSelector->tallyButtons();

        // Motif Editor
        motifEditor->delegate(np);
    }

    // view
    view->update();
}

void MotifMakerWidget::update()
{
    delSelector->getDelegated()->update();
    viewerBtn->update();

}
