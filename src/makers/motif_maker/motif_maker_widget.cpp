#include <qglobal.h>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QDebug>
#endif
#include "makers/motif_maker/motif_maker_widget.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/motif_maker/motif_editor_widget.h"
#include "makers/motif_maker/motif_selector.h"
#include "makers/prototype_maker//prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"

MotifMakerWidget::MotifMakerWidget() : AQWidget()
{
    motifViewData = PrototypeMaker::getInstance()->getProtoMakerData();
    config        = Configuration::getInstance();

    // Motif buttons
    delSelector = new DELSelectorWidget();

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

    connect(delSelector, &DELSelectorWidget::sig_launcherButton,    this, &MotifMakerWidget::slot_selectMotifButton);
}

void MotifMakerWidget::setCurrentButtonViewTransform()
{
    viewerBtn->setViewTransform();
    DELBtnPtr btn = delSelector->getCurrentButton();
    if (btn)
    {
        btn->setViewTransform();
    }
}

// when a motif button is selected, this sets the motif in both the motif maker and the motif editor
void MotifMakerWidget::slot_selectMotifButton(DELBtnPtr btn)
{
    if (!btn) return;
    qDebug() << "MotifWidget::slot_selectMotifButton btn=" << btn->getIndex() << btn.get();

    DesignElementPtr designElement = btn->getDesignElement(); // DAC taprats cloned here
    if (!designElement) return;

    motifViewData->select(MVD_DELEM,designElement,config->motifMultiView);
}

void MotifMakerWidget::selectPrototype(ProtoPtr proto)
{
    delSelector->setup(proto);
}

void MotifMakerWidget::selectDEL(DesignElementPtr designElement)
{
    viewerBtn->setDesignElement(designElement);

    motifEditor->selectMotifEditor(designElement);

    delSelector->tallyButtons();

    setCurrentButtonViewTransform();

    update();
}

