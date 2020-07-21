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

#include "panels/page_prototype_maker.h"
#include "base/tiledpatternmaker.h"
#include "base/canvas.h"
#include "base/utilities.h"
#include "makers/figure_maker/prototype_maker.h"
#include "style/style.h"
#include "panels/panel.h"
#include "viewers/workspace_viewer.h"

Q_DECLARE_METATYPE(PrototypePtr)

page_prototype_maker::page_prototype_maker(ControlPanel * cpanel) : panel_page(cpanel,"Prototype Maker")
{
    // top line
    protoListBox             = new QComboBox();
    protoListBox->setMinimumWidth(131);

    whiteBackground          = new QCheckBox("White background");
    replicateRadial          = new QCheckBox("Replicate Radial");
    hiliteUnit               = new QCheckBox("Highlight Unit");
    QPushButton * pbDup      = new QPushButton("Duplicate Figure");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(protoListBox);
    hbox->addSpacing(31);
    hbox->addWidget(whiteBackground);
    hbox->addWidget(replicateRadial);
    hbox->addWidget(hiliteUnit);
    hbox->addWidget(pbDup);

    // middle
    prototypeMaker = PrototypeMaker::getInstance();
    prototypeMaker->init(maker,this);

    // bottom line
    QPushButton * pbReplaceInProto = new QPushButton("Replace in Proto");
    QPushButton * pbAddToProto     = new QPushButton("Add to Proto");
    QPushButton * pbRender         = new QPushButton("Render");

    QHBoxLayout * hbox2  = new QHBoxLayout;
    hbox2->addWidget(pbReplaceInProto);
    hbox2->addWidget(pbAddToProto);
    hbox2->addWidget(pbRender);

    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addLayout(prototypeMaker);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    connect(pbReplaceInProto,   &QPushButton::clicked,              this,  &page_prototype_maker::slot_replaceInStyle);
    connect(pbAddToProto,       &QPushButton::clicked,              this,  &page_prototype_maker::slot_addToStyle);
    connect(pbRender,           &QPushButton::clicked,              this,  &page_prototype_maker::slot_render);
    connect(pbDup,              &QPushButton::clicked,              this,  &page_prototype_maker::slot_duplicateCurrent);
    connect(whiteBackground,    &QCheckBox::clicked,                this,  &page_prototype_maker::whiteClicked);
    connect(replicateRadial,    &QCheckBox::clicked,                this,  &page_prototype_maker::repRadClicked);
    connect(hiliteUnit,         &QCheckBox::clicked,                this,  &page_prototype_maker::hiliteClicked);

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_prototype_maker::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_prototype_maker::slot_loadedXML);
    connect(canvas, &Canvas::sig_unload,                    this,   &page_prototype_maker::slot_unload);
    connect(protoListBox, SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_prototypeSelected(int)));

    if (config->figureViewBkgdColor == Qt::white)
    {
        whiteBackground->setChecked(true);
    }
}

void page_prototype_maker::setupFigure(bool isRadial)
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


void page_prototype_maker::slot_unload()
{
    if (prototypeMaker)
        prototypeMaker->unload();
}

void page_prototype_maker::slot_reload()
{
    reload();
}

void page_prototype_maker::reload()
{
    TilingPtr tiling = workspace->getTiling();
    if (!tiling)
    {
        prototypeMaker->unload();
        protoListBox->clear();
        return;
    }

    QVector<PrototypePtr> protos = workspace->getPrototypes();
    if (protos.isEmpty())
    {
        // create new prototype
        PrototypePtr pp = make_shared<Prototype>(tiling);
        workspace->addPrototype(pp);
        workspace->setSelectedPrototype(pp);
    }

    protoListBox->blockSignals(true);
    protoListBox->clear();

    for (auto proto : protos)
    {
        QString paddr = Utils::addr(proto.get());
        protoListBox->addItem(paddr,QVariant::fromValue(proto));
    }

    PrototypePtr pp = workspace->getSelectedPrototype();
    QString paddr = Utils::addr(pp.get());
    if (pp)
    {
        pp->setTiling(tiling);
        prototypeMaker->setupFigures(pp);   // sets the prototype maker

        int index = protoListBox->findText(paddr);
        protoListBox->setCurrentIndex(index);
    }
    protoListBox->blockSignals(false);
}

void page_prototype_maker::onEnter()
{
    QString txt("<body style=\"background-color=#000000\"><font color=green>figure</font>  |  <font color=magenta>feature boundary</font>  |  <font color=red>radial figure boundary</font>  |  <font color=yellow>extended boundary</font></body>");
    panel->setStatusStyle("QLabel { background-color : black; }");
    panel->showStatus(txt);

    reload();
}

void page_prototype_maker::onExit()
{
    panel->hideStatus();
}

void page_prototype_maker::refreshPage(void)
{
}

void page_prototype_maker::slot_tilingChanged()
{
    if (prototypeMaker)
    {
        qDebug() << "++tiling changed";
        reload();
    }
}

void page_prototype_maker::slot_replaceInStyle()
{
    qDebug() << "page_figure_maker::slot_replaceInStyle()";

    bool replaced = false;
    PrototypePtr pp       = prototypeMaker->getPrototype();
    MosaicPtr mosaic      = workspace->getMosaic();
    if (mosaic)
    {
        const StyleSet & sset = mosaic->getStyleSet();

        // put new prototype into existing workspace styles
        for (auto it = sset.begin(); it != sset.end(); it++)
        {
            StylePtr sp = *it;
            PrototypePtr existingPP = sp->getPrototype();
            if (existingPP->getTiling()->getName() == pp->getTiling()->getName())
            {
                replaced = true;
                sp->setPrototype(pp);
            }
        }
    }

    if (!replaced)
    {
        slot_addToStyle();
        return;
    }

    emit sig_render();
}

void page_prototype_maker::slot_addToStyle()
{
    qDebug() << "page_figure_maker::slot_addToStyle()";

    StylePtr sp =  prototypeMaker->createDefaultStyleFromPrototype();

    MosaicPtr mosaic = workspace->getMosaic();
    if (!mosaic)
    {
        mosaic = make_shared<Mosaic>();
        workspace->setMosaic(mosaic);
    }
    if (!mosaic->hasContent())
    {
        const CanvasSettings & cs = viewer->getCurrentCanvasSettings();
        mosaic->setCanvasSettings(cs);
    }

    mosaic->addStyle(sp);

    emit sig_render();
}

void page_prototype_maker::slot_render()
{
     emit sig_render();
}

void  page_prototype_maker::whiteClicked(bool state)
{
    if (state)
        config->figureViewBkgdColor = QColor(Qt::white);
    else
        config->figureViewBkgdColor = QColor(Qt::black);
    emit sig_viewWS();
}

void  page_prototype_maker::repRadClicked(bool state)
{
    config->debugReplicate = !state;
    emit canvas->sig_figure_changed();
}

void  page_prototype_maker::hiliteClicked(bool state)
{
    config->highlightUnit = state;
    emit sig_viewWS();
}

void page_prototype_maker::slot_duplicateCurrent()
{
    if (prototypeMaker->duplicateActiveFeature())
    {
        reload();
        emit sig_render();
    }
}

void  page_prototype_maker::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    reload();
}

void page_prototype_maker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name)
    reload();
}


void page_prototype_maker::slot_prototypeSelected(int row)
{
    PrototypePtr pp;
    QVariant var = protoListBox->itemData(row);
    if (var.canConvert<PrototypePtr>())
    {
        pp = var.value<PrototypePtr>();
    }
    if (pp)
    {
        workspace->setSelectedPrototype(pp);
        reload();
    }
}

