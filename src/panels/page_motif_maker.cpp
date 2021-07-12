/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/page_motif_maker.h"
#include "panels/panel.h"
#include "base/tiledpatternmaker.h"
#include "base/shared.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "style/style.h"
#include "panels/motif_display_widget.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"
#include "tapp/design_element.h"
#include "tapp/figure.h"

typedef std::weak_ptr<Prototype>       WeakPrototypePtr;

Q_DECLARE_METATYPE(WeakPrototypePtr)

page_motif_maker::page_motif_maker(ControlPanel * cpanel) : panel_page(cpanel,"Motif Maker")
{
    motifMaker->setCallback(this);

    // top line
    QLabel  * tilingLabel    = new QLabel("Tiling:");
    tilingListBox            = new QComboBox();
    tilingListBox->setMinimumWidth(131);

    whiteBackground          = new QCheckBox("White background");
    replicateRadial          = new QCheckBox("Replicate Radial");
    hiliteUnit               = new QCheckBox("Highlight Unit");
    QPushButton * pbDup      = new QPushButton("Duplicate Selected Figure");
    QPushButton * pbDel      = new QPushButton("Delete Selected Figure");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(tilingLabel);
    hbox->addWidget(tilingListBox);
    hbox->addSpacing(31);
    hbox->addWidget(whiteBackground);
    hbox->addWidget(replicateRadial);
    hbox->addWidget(hiliteUnit);
    hbox->addWidget(pbDup);
    hbox->addWidget(pbDel);

    // Feature Buttons
    motifWidget = new MotifDisplayWidget(this);
    motifWidget->setMinimumWidth(610);

    // bottom line
    QPushButton * pbRender = new QPushButton("Render");
    pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addStretch();
    hbox2->addWidget(pbRender);
    hbox2->addStretch();

    // putting it together
    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addWidget(motifWidget);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    connect(pbRender,           &QPushButton::clicked,                  this,   &panel_page::sig_render);
    connect(pbDup,              &QPushButton::clicked,                  this,   &page_motif_maker::slot_duplicateCurrent);
    connect(pbDel,              &QPushButton::clicked,                  this,   &page_motif_maker::slot_deleteCurrent);
    connect(whiteBackground,    &QCheckBox::clicked,                    this,   &page_motif_maker::whiteClicked);
    connect(replicateRadial,    &QCheckBox::clicked,                    this,   &page_motif_maker::replicateRadialClicked);
    connect(hiliteUnit,         &QCheckBox::clicked,                    this,   &page_motif_maker::hiliteClicked);
    connect(tilingListBox,      SIGNAL(currentIndexChanged(int)),       this,   SLOT(slot_prototypeSelected(int)));


    if (config->figureViewBkgdColor == Qt::white)
    {
        whiteBackground->setChecked(true);
    }
}

void page_motif_maker::onEnter()
{
}

void page_motif_maker::onExit()
{
}

void page_motif_maker::refreshPage(void)
{
    static WeakPrototypePtr wpp;

    if (wpp.lock() != motifMaker->getSelectedPrototype())
    {
        wpp = motifMaker->getSelectedPrototype();
        tilingChoicesChanged();
        motifWidget->prototypeChanged();
    }
}

void page_motif_maker::featureChanged()
{
    motifWidget->figureChanged();
}

void page_motif_maker::tilingChanged()
{
    motifWidget->prototypeChanged();
}

void page_motif_maker::setupFigure(bool isRadial)
{
    if (isRadial)
    {
        replicateRadial->show();
        replicateRadial->blockSignals(true);
        replicateRadial->setChecked(!config->debugReplicate);
        replicateRadial->blockSignals(false);
        hiliteUnit->show();
        hiliteUnit->blockSignals(true);
        hiliteUnit->setChecked(config->highlightUnit);
        hiliteUnit->blockSignals(false);
    }
    else
    {
        replicateRadial->hide();
        hiliteUnit->hide();
    }
}

void page_motif_maker::tilingChoicesChanged()
{
    tilingListBox->blockSignals(true);

    tilingListBox->clear();

    const QVector<PrototypePtr> & protos = motifMaker->getPrototypes();
    for (auto proto : protos)
    {
        tilingListBox->addItem(proto->getTiling()->getName(),QVariant::fromValue(WeakPrototypePtr(proto)));
    }

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    if (pp)
    {
        QString name = pp->getTiling()->getName();
        qDebug() << "page_prototype_maker::reload() selected tiling:" << name;
        int index = tilingListBox->findText(name);
        tilingListBox->setCurrentIndex(index);
    }

    tilingListBox->blockSignals(false);
}

void  page_motif_maker::whiteClicked(bool state)
{
    if (state)
        config->figureViewBkgdColor = QColor(Qt::white);
    else
        config->figureViewBkgdColor = QColor(Qt::black);
    emit sig_refreshView();
}

void  page_motif_maker::replicateRadialClicked(bool state)
{
    config->debugReplicate = !state;
    motifWidget->slot_launcherButton();
}

void  page_motif_maker::hiliteClicked(bool state)
{
    config->highlightUnit = state;
    emit sig_refreshView();
}

void page_motif_maker::slot_duplicateCurrent()
{
    motifMaker->duplicateActiveFeature();
    motifWidget->prototypeChanged();
}

void page_motif_maker::slot_deleteCurrent()
{
    motifMaker->deleteActiveFeature();
    motifWidget->prototypeChanged();
}

void page_motif_maker::slot_prototypeSelected(int row)
{
    QVariant var = tilingListBox->itemData(row);
    if (var.canConvert<WeakPrototypePtr>())
    {
        WeakPrototypePtr wpp = var.value<WeakPrototypePtr>();
        PrototypePtr pp      = wpp.lock();
        select(pp);
    }
}

FeaturePtr page_motif_maker::getActiveFeature()
{
    return motifMaker->getActiveFeature();
}

void page_motif_maker::select(PrototypePtr prototype)
{
    qDebug() << "MotifMaker::select  prototype="  << prototype.get();

    motifMaker->setSelectedPrototype(prototype);

    tilingMaker->select(prototype->getTiling());
}

void page_motif_maker::slot_figureChanged(FigurePtr fp)
{
    setupFigure(fp->isRadial());

    DesignElementPtr dep = motifMaker->getSelectedDesignElement();
    dep->setFigure(fp);

    motifWidget->figureChanged();

    motifMaker->setSelectedDesignElement(dep);

    motifMaker->sm_take(motifMaker->getSelectedPrototype()->getTiling(),SM_FIGURE_CHANGED);

    emit sig_refreshView();
}

void page_motif_maker::slot_figureTypeChanged(eFigType type)
{
    // placeholder if anything else needs to be  done here
    Q_UNUSED(type);

    emit sig_refreshView();
}
