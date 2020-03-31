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

#include "panels/page_figure_maker.h"
#include "base/tiledpatternmaker.h"
#include "base/canvas.h"
#include "makers/figure_maker.h"
#include "style/Style.h"
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"

page_figure_maker::page_figure_maker(ControlPanel * cpanel) : panel_page(cpanel,"Figure Maker")
{
    // top line
    radioLoadedStyleTileView = new QRadioButton("Style Tiling");
    radioWSTileView          = new QRadioButton("Workspace Tiling");
    whiteBackground          = new QCheckBox("White background");
    QPushButton * pbDup      = new QPushButton("Duplicate Current");

    tilingGroup.addButton(radioLoadedStyleTileView,FV_STYLE);
    tilingGroup.addButton(radioWSTileView,FV_WS);
    tilingGroup.button(config->figureViewer)->setChecked(true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(radioLoadedStyleTileView);
    hbox->addWidget(radioWSTileView);
    hbox->addStretch();
    hbox->addWidget(whiteBackground);
    hbox->addStretch();
    hbox->addWidget(pbDup);

    // middle
    figureMaker = new FigureMaker(maker);

    // bottom line
    QPushButton * pbReplaceInStyle = new QPushButton("Replace Proto");
    QPushButton * pbAddToStyle     = new QPushButton("Add Proto");
    QPushButton * pbRender         = new QPushButton("Render");

    targetLoadedStyle = new QRadioButton("To Loaded Styles");
    targetWS          = new QRadioButton("To WS Styles");

    targetGroup.addButton(targetLoadedStyle,TARGET_LOADED_STYLES);
    targetGroup.addButton(targetWS, TARGET_WS_STYLES);
    targetGroup.button(config->pushTarget)->setChecked(true);

    QHBoxLayout * hbox2  = new QHBoxLayout;
    hbox2->addWidget(targetLoadedStyle);
    hbox2->addWidget(targetWS);
    hbox2->addWidget(pbReplaceInStyle);
    hbox2->addWidget(pbAddToStyle);
    hbox2->addWidget(pbRender);
    hbox2->addStretch();

    vbox->addLayout(hbox);
    vbox->addSpacing(7);
    vbox->addLayout(figureMaker);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    connect(&tilingGroup,       SIGNAL(buttonClicked(int)),         this,  SLOT(slot_source_selected(int)));
    connect(&targetGroup,       SIGNAL(buttonClicked(int)),         this,  SLOT(slot_target_selected(int)));
    connect(pbReplaceInStyle,   &QPushButton::clicked,              this,  &page_figure_maker::slot_replaceInStyle);
    connect(pbAddToStyle,       &QPushButton::clicked,              this,  &page_figure_maker::slot_addToStyle);
    connect(pbRender,           &QPushButton::clicked,              this,  &page_figure_maker::slot_render);
    connect(pbDup,              &QPushButton::clicked,              this,  &page_figure_maker::slot_duplicateCurrent);
    connect(whiteBackground,    &QCheckBox::clicked,                this,  &page_figure_maker::whiteClicked);

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_figure_maker::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_figure_maker::slot_loadedXML);
    connect(canvas, &Canvas::sig_unload,                    this,   &page_figure_maker::slot_unload);

    if (config->figureViewBkgdColor == Qt::white)
    {
        whiteBackground->setChecked(true);
    }
}

void page_figure_maker::slot_unload()
{
    if (figureMaker)
        figureMaker->unload();
    loadedStyle.reset();
    loadedTiling.reset();
}

void page_figure_maker::reload(bool force)
{
    if (config->figureViewer == FV_STYLE)
    {
        // prototype from style
        StyledDesign & sd = workspace->getLoadedStyles();
        StylePtr sp = sd.getFirstStyle();
        if (sp)
        {
            if (sp != loadedStyle || force)
            {

                loadedStyle = sp;
                figureMaker->setNewStyle(loadedStyle);
            }
        }
    }
    else
    {
        Q_ASSERT(config->figureViewer ==  FV_WS);
        TilingPtr tp = workspace->getTiling();

        if (tp && tp->countPlacedFeatures())
        {
                loadedTiling = tp;
                figureMaker->setNewTiling(loadedTiling);
        }
    }

    targetGroup.button(config->pushTarget)->setChecked(true);
}

void page_figure_maker::onEnter()
{
    QString txt("<body style=\"background-color=#000000\"><font color=green>figure</font>  |  <font color=magenta>feature boundary</font>  |  <font color=red>radial figure boundary</font>  |  <font color=yellow>extended boundary</font></body>");
    QLabel * l = panel->getStatus();
    l->setStyleSheet("QLabel { background-color : black; }");
    l->setMinimumHeight(21);
    l->setText(txt);
    reload();
}

void page_figure_maker::onExit()
{
    panel->getStatus()->setText("");
}

void page_figure_maker::refreshPage(void)
{
    radioLoadedStyleTileView->setText("Style Tiling " + addr(workspace->getLoadedStyles().getTiling().get()));
    radioWSTileView->setText("Workspace Tiling " + addr(workspace->getTiling().get()));

    tilingGroup.blockSignals(true);
    tilingGroup.button(config->figureViewer)->setChecked(true);
    tilingGroup.blockSignals(false);
}

void page_figure_maker::slot_tilingChanged()
{
    if (figureMaker)
    {
        qDebug() << "++tiling changed";
        figureMaker->setTilingChanged();
    }
}

void page_figure_maker::slot_replaceInStyle()
{
    qDebug() << "page_figure_maker::slot_replaceInStyle()";

    PrototypePtr pp = figureMaker->getPrototype();
    StyledDesign & sd = (config->pushTarget == TARGET_WS_STYLES) ? workspace->getWsStyles() : workspace->getLoadedStyles();
    const StyleSet & sset = sd.getStyleSet();

    // put new prototype into existing workspace styles
    bool replaced = false;
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

    if (!replaced)
    {
        slot_addToStyle();
        return;
    }

    if (config->pushTarget == TARGET_WS_STYLES)
        emit sig_render(RENDER_WS);
    else
        emit sig_render(RENDER_LOADED);
}

void page_figure_maker::slot_addToStyle()
{
    qDebug() << "page_figure_maker::slot_addToStyle()";

    StylePtr sp =  figureMaker->createDefaultStyleFromPrototype();

    StyledDesign & sd = (config->pushTarget == TARGET_WS_STYLES) ? workspace->getWsStyles() : workspace->getLoadedStyles();

    if (!sd.hasContent())
    {
        ViewDefinition * viewDef = &WorkspaceViewer::viewDimensions[VIEW_DESIGN];
        CanvasSettings cs;
        cs.set(viewDef);
        sd.setCanvasSettings(cs);
    }

    sd.addStyle(sp);

    if (config->pushTarget == TARGET_WS_STYLES)
        emit sig_render(RENDER_WS);
    else
        emit sig_render(RENDER_LOADED);
}

void page_figure_maker::slot_render()
{
    if (config->pushTarget == TARGET_WS_STYLES)
        emit sig_render(RENDER_WS);
    else
        emit sig_render(RENDER_LOADED);
}

void page_figure_maker::slot_source_selected(int id)
{
    config->figureViewer = eFigureViewer(id);
    onEnter();
    emit sig_viewWS();
}

void page_figure_maker::slot_target_selected(int id)
{
    config->pushTarget = ePushTarget(id);
}

void  page_figure_maker::whiteClicked(bool state)
{
    if (state)
        config->figureViewBkgdColor = QColor(Qt::white);
    else
        config->figureViewBkgdColor = QColor(Qt::black);
    emit sig_viewWS();
}

void page_figure_maker::slot_duplicateCurrent()
{
    if (figureMaker->duplicateActiveFeature())
    {
    reload(true);
        if (config->pushTarget == TARGET_WS_STYLES)
            emit sig_render(RENDER_WS);
        else
            emit sig_render(RENDER_LOADED);
    }
}

void  page_figure_maker::slot_loadedXML(QString name)
{
    Q_UNUSED(name)

    reload();
}

void page_figure_maker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name)

    reload();
}
