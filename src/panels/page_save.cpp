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

#include "page_save.h"
#include "base/tiledpatternmaker.h"
#include "base/canvas.h"
#include "tile/Tiling.h"

page_save::page_save(ControlPanel * cpanel)  : panel_page(cpanel, "Save")
{
    createDesignSave();
    createTilingSave();

    QPushButton * pbSaveImage = new QPushButton("Save Pixmap");
    QPushButton * pbSaveSVG   = new QPushButton("Save as SVG");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(pbSaveImage);
    hbox->addSpacing(17);
    hbox->addWidget(pbSaveSVG);
    hbox->addStretch();
    vbox->addLayout(hbox);

    connect(maker,  &TiledPatternMaker::sig_loadedXML,    this,   &page_save::slot_loadedXML);
    connect(maker,  &TiledPatternMaker::sig_loadedTiling, this,   &page_save::slot_loadedTiling);
    connect(pbSaveImage,  &QPushButton::clicked,          canvas, &Canvas::saveImage);
    connect(pbSaveSVG,    &QPushButton::clicked,          canvas, &Canvas::saveSvg);
}

void page_save::createDesignSave()
{
    designStyle = new QRadioButton("Loaded Styles");
    designWS    = new QRadioButton("Workspace");

    QButtonGroup * savegroup = new QButtonGroup();
    savegroup->addButton(designStyle,WS_LOADED);
    savegroup->addButton(designWS,WS_TILING);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(designStyle);
    hbox->addWidget(designWS);
    hbox->addStretch();

    leSaveXmlName   = new QLineEdit();
    saveXml         = new QPushButton("Save Design");
    designNotes     = new QTextEdit("Design Notes");
    designNotes->setFixedSize(601,101);
    QLabel * label  = new QLabel("Name");

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label);
    hbox2->addWidget(leSaveXmlName);
    hbox2->addWidget(saveXml);
    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addLayout(hbox);
    vlayout->addLayout(hbox2);
    vlayout->addWidget(designNotes);

    QGroupBox * saveBox = new QGroupBox("Design");
    saveBox->setLayout(vlayout);
    vbox->addWidget(saveBox);

    designStyle->setChecked(true);

    connect(savegroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this , &page_save::slot_designSourceChanged);
    connect(saveXml,   SIGNAL(clicked()),       this,   SLOT(slot_saveAsXML()));
    connect(this,      &page_save::sig_saveXML, maker,  &TiledPatternMaker::slot_saveXML);
}

void page_save::createTilingSave()
{
    tilingStyle  = new QRadioButton("Loaded Styles");
    tilingStyle->setChecked(true);
    tilingWS     = new QRadioButton("Workspace");
    requiresSave = new QLabel;
    requiresSave->setStyleSheet("color: red");
    requiresSave->setFixedWidth(91);
    requiresSave->setAlignment(Qt::AlignRight);

    QButtonGroup * savegroup = new QButtonGroup();
    savegroup->addButton(tilingStyle,WS_LOADED);
    savegroup->addButton(tilingWS,WS_TILING);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(tilingStyle);
    hbox->addWidget(tilingWS);
    hbox->addStretch();
    hbox->addWidget(requiresSave);

    QPushButton * pbSave  = new QPushButton("Save Tiling");
    QLabel * label        = new QLabel("Name");
    QLabel * label2       = new QLabel("Author");
    tile_name             = new QLineEdit();
    tile_author           = new QLineEdit();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label);
    hbox2->addWidget(tile_name);
    hbox2->addWidget(label2);
    hbox2->addWidget(tile_author);
    hbox2->addWidget(pbSave);

    tile_desc = new QTextEdit();
    tile_desc->setFixedHeight(101);

    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addLayout(hbox);
    vlayout->addSpacing(3);
    vlayout->addLayout(hbox2);
    vlayout->addSpacing(3);
    vlayout->addWidget(tile_desc);
    vlayout->addSpacing(3);

    QGroupBox * saveBox = new QGroupBox("Tiling");
    saveBox->setLayout(vlayout);
    vbox->addWidget(saveBox);

    connect(savegroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this , &page_save::slot_tilingSourceChanged);
    connect(pbSave, &QPushButton::clicked, this, &page_save::slot_saveTiling);
}

TilingPtr page_save::getTiling()
{
    eWsData wsdata   = (tilingStyle->isChecked()) ? WS_LOADED : WS_TILING;
    TilingPtr tiling = workspace->getTiling(wsdata);
    return tiling;
}

void page_save::onEnter()
{
    eWsData wsdata    = (designStyle->isChecked()) ? WS_LOADED : WS_TILING;
    StyledDesign & sd = workspace->getStyledDesign(wsdata);

    leSaveXmlName->setText(sd.getName());
    designNotes->setText(sd.getNotes());

    TilingPtr tiling =  getTiling();

    blockSignals(true);
    if (tiling)
    {
        tile_desc->setText(tiling->getDescription());
        tile_name->setText(tiling->getName());
        tile_author->setText(tiling->getAuthor());
    }
    else
    {
        tile_desc->setText("");
        tile_name->setText("");
        tile_author->setText("");
    }
    blockSignals(false);

    refreshPage();;
}

void page_save::refreshPage()
{
    TilingPtr tiling = getTiling();

    if (tiling && tiling->isDirty())
        requiresSave->setText("HAS CHANGED");
    else
        requiresSave->setText("");
}

void page_save::slot_saveAsXML()
{
    eWsData wsdata    = (designStyle->isChecked()) ? WS_LOADED : WS_TILING;
    StyledDesign & sd = workspace->getStyledDesign(wsdata);
    sd.setNotes(designNotes->toPlainText());

    QString name = leSaveXmlName->text();
    Q_ASSERT(!name.contains(".xml"));
    emit sig_saveXML(wsdata,name);
}


void page_save::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    onEnter();
}

void page_save::slot_loadedTiling(QString name)
{
    Q_UNUSED(name)
    onEnter();
}

void page_save::slot_designSourceChanged(int wsdata)
{
    Q_UNUSED(wsdata)
    onEnter();
}

void page_save::slot_tilingSourceChanged(int wsdata)
{
    Q_UNUSED(wsdata)
    onEnter();
}

void page_save::slot_saveTiling()
{
    TilingPtr tiling =  getTiling();
    if (!tiling)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Save Tiling: there is no tiling");
        box.exec();
        return;
    }

    if (tile_name->text().isEmpty() || tile_name->text() == "The Unnamed")
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Save Tiling: requires tiling name");
        box.exec();
        return;
    }

    tiling->setName(tile_name->text());
    tiling->setAuthor(tile_author->text());
    tiling->setDescription(tile_desc->toPlainText());

    if (workspace->saveTiling(tile_name->text(),tiling))
    {
        tiling->setDirty(false);
    }
}


