#include "motif_display_widget.h"
#include "makers/motif_maker/feature_launcher.h"
#include "makers/motif_maker/master_figure_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "viewers/view.h"

MotifDisplayWidget::MotifDisplayWidget(page_motif_maker * menu)
{
    this->menu = menu;
    motifMaker = MotifMaker::getInstance();

    // Feature buttons
    launcher    = new FeatureLauncher();
    viewerBtn   = make_shared<FeatureButton>(-1);

    QHBoxLayout * btnBox = new QHBoxLayout();
    btnBox->addLayout(launcher);
    btnBox->addWidget(viewerBtn.get());

    // Editors
    masterEdit  = new MasterFigureEditor(menu);

    AQVBoxLayout * motifBox = new AQVBoxLayout;
    motifBox->addLayout(btnBox);
    motifBox->addWidget(masterEdit);

    setLayout(motifBox);

    setMinimumWidth(610);

    connect(launcher,  &FeatureLauncher::sig_launcherButton, this, &MotifDisplayWidget::slot_launcherButton);
    View * view = View::getInstance();
    connect(view,      &View::sig_figure_changed,            this, &MotifDisplayWidget::slot_launcherButton);
}

void MotifDisplayWidget::prototypeChanged()
{
    launcher->launch(motifMaker->getSelectedPrototype());
    update();
}

void MotifDisplayWidget::figureChanged()
{
    viewerBtn->designElementChanged();
    FeatureBtnPtr btn = launcher->getCurrentButton();
    if (btn)
        btn->designElementChanged();
    update();
}

void MotifDisplayWidget::slot_launcherButton()
{
    qDebug() << "MotifMaker::slot_launcherButton";
    setActiveFeature(launcher->getCurrentButton());
}

void MotifDisplayWidget::setActiveFeature(FeatureBtnPtr fb)
{
    if (!fb) return;

    qDebug() << "MotifMaker::setActiveFeature btn=" << fb->getIndex() << fb.get();

    DesignElementPtr designElement = fb->getDesignElement(); // DAC taprats cloned here
    Q_ASSERT(designElement);
    FigurePtr figure   = designElement->getFigure();
    Q_ASSERT(figure);
    qDebug() << "Active feature: index=" << fb->getIndex() << "del=" << designElement.get()  << "fig=" << figure.get() << " " << figure->getFigureDesc();

    motifMaker->setSelectedDesignElement(designElement);
    motifMaker->setActiveFeature(designElement->getFeature());

    viewerBtn->setDesignElement(designElement);

    masterEdit->masterResetWithFigure(figure);

    menu->setupFigure(figure->isRadial());

    View * view = View::getInstance();
    view->update();
    menu->update();
}



