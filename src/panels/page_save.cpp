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

using std::string;

page_save::page_save(ControlPanel * cpanel)  : panel_page(cpanel, "Save")
{
    createConfigGrid();
    vbox->addLayout(configGrid);
    vbox->addStretch();

    refreshPage();
}

void page_save::createConfigGrid()
{
    leSaveXmlName   = new QLineEdit();
    saveXml         = new QPushButton("Save Design");
    designNotes     = new QTextEdit("Design Notes");
    designNotes->setFixedSize(601,201);
    QLabel * label  = new QLabel("Design");

    configGrid = new QGridLayout();
    configGrid->addWidget(label,0,0);
    configGrid->addWidget(leSaveXmlName,0,1);
    configGrid->addWidget(saveXml,0,2);

    configGrid->addWidget(designNotes,1,0,1,3);

    connect(saveXml,        SIGNAL(clicked()),                  this,   SLOT(slot_saveAsXML()));
    connect(designNotes,    &QTextEdit::textChanged,            this,   &page_save::designNotesChanged);
    connect(this,           &page_save::sig_saveXML,            maker,  &TiledPatternMaker::slot_saveXML);
    connect(maker,          &TiledPatternMaker::sig_loadedXML,  this,   &page_save::slot_loadedXML);
}

void page_save::onEnter()
{
    leSaveXmlName->setText(config->currentlyLoadedXML);
    designNotes->setText(workspace->getLoadedStyles().getNotes());
}

void page_save::refreshPage()
{
}

void page_save::slot_saveAsXML()
{
    QString name = leSaveXmlName->text();
    Q_ASSERT(!name.contains(".xml"));
    emit sig_saveXML(name);
}

void page_save::slot_showXMLName(QString name)
{
    leSaveXmlName->setText(name);
}

void page_save::designNotesChanged()
{
    workspace->getLoadedStyles().setNotes(designNotes->toPlainText());
    workspace->getWsStyles().setNotes(designNotes->toPlainText());
}

void page_save::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    onEnter();
}
