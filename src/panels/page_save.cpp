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

#include "panels/page_save.h"
#include "base/tiledpatternmaker.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "style/style.h"
#include <QSvgGenerator>

page_save::page_save(ControlPanel * cpanel)  : panel_page(cpanel, "Save")
{
    createDesignSave();
    createTilingSave();

    QPushButton * pbSaveImage = new QPushButton("Save BMP");
    QPushButton * pbSaveSVG   = new QPushButton("Save SVG");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(pbSaveImage);
    hbox->addSpacing(17);
    hbox->addWidget(pbSaveSVG);
    hbox->addStretch();
    vbox->addLayout(hbox);

    connect(tpm,  &TiledPatternMaker::sig_loadedXML,    this,   &page_save::slot_loadedXML);
    connect(tpm,  &TiledPatternMaker::sig_loadedTiling, this,   &page_save::slot_loadedTiling);
    connect(pbSaveImage,  &QPushButton::clicked,        this,   &page_save::slot_saveImage);
    connect(view,         &View::sig_saveImage,         this,   &page_save::slot_saveImage);
    connect(pbSaveSVG,    &QPushButton::clicked,        this,   &page_save::slot_saveSvg);
    connect(view,         &View::sig_saveSVG,           this,   &page_save::slot_saveSvg);
}

void page_save::createDesignSave()
{
    leSaveXmlName   = new QLineEdit();
    saveXml         = new QPushButton("Save Mosaic");
    designNotes     = new QTextEdit("Design Notes");
    designNotes->setFixedSize(601,101);
    QLabel * label  = new QLabel("Name");

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label);
    hbox2->addWidget(leSaveXmlName);
    hbox2->addWidget(saveXml);
    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addLayout(hbox2);
    vlayout->addWidget(designNotes);

    QGroupBox * saveBox = new QGroupBox("Mosaic");
    saveBox->setLayout(vlayout);
    vbox->addWidget(saveBox);

    connect(saveXml,   SIGNAL(clicked()),       this,   SLOT(slot_saveAsXML()));
    connect(this,      &page_save::sig_saveXML, tpm,  &TiledPatternMaker::slot_saveXML);
}

void page_save::createTilingSave()
{
    requiresSave = new QLabel;
    requiresSave->setStyleSheet("color: red");
    requiresSave->setFixedWidth(91);
    requiresSave->setAlignment(Qt::AlignRight);

    QHBoxLayout * hbox = new QHBoxLayout;
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

    connect(pbSave, &QPushButton::clicked, this, &page_save::slot_saveTiling);
}

void page_save::onEnter()
{
    MosaicPtr mosaic = workspace->getMosaic();
    leSaveXmlName->setText(mosaic->getName());
    designNotes->setText(mosaic->getNotes());

    TilingPtr tiling =  workspace->getCurrentTiling();
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
    TilingPtr tiling =  workspace->getCurrentTiling();

    if (tiling && (tiling->getState() == TILING_MODIFED))
        requiresSave->setText("HAS CHANGED");
    else
        requiresSave->setText("");
}

void page_save::slot_saveAsXML()
{
    MosaicPtr mosaic = workspace->getMosaic();
    mosaic->setNotes(designNotes->toPlainText());
    QString name = leSaveXmlName->text();
    Q_ASSERT(!name.contains(".xml"));
    emit sig_saveXML(name);
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

void page_save::slot_designSourceChanged()
{
    onEnter();
}

void page_save::slot_tilingSourceChanged()
{
    onEnter();
}

void page_save::slot_saveTiling()
{
    QString name = tile_name->text();

    TilingPtr tiling = workspace->getCurrentTiling();
    if (!tiling || tiling->getState() == TILING_EMPTY)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling");
        box.setText("There is no tiling to save. Please add some features");
        box.exec();
        return;
    }

    if (tiling->countPlacedFeatures() == 0)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling");
        box.setText("There are no placed features.  Please add som features.");
        box.exec();
        return;
    }

    int count = tiling->getUniqueFeatures().count();
    if (count >= MAX_UNIQUE_FEATURES)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle("Save Tiling");
        box.setText(QString("There are too many unique features (count=%1).\nSomething is wrong with this tiling.").arg(count));
        box.exec();
        return;
    }

    if (name.isEmpty() || name == "The Unnamed")
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling");
        box.setText("There is no tiling name. A tiling name is required.");
        box.exec();
        return;
    }

    tiling->setName(name);
    tiling->setAuthor(tile_author->text());
    tiling->setDescription(tile_desc->toPlainText());

    TilingManager tm;
    tm.saveTiling(name,tiling); // Tiling Manager haandles OK/FAIL status
}

void page_save::slot_saveImage()
{
    QPixmap pixmap = View::getInstance()->grab();

    QString name = config->lastLoadedXML;
    Q_ASSERT(!name.contains(".xml"));

    QSettings s;
    QString path = config->rootMediaDir;
    qDebug() << "path=" << path;

    QString nameList;
    if (path.contains(".png"))
    {
        nameList = "PNG (*.png);;BMP Files (*.bmp);;JPG (*.jpg)";
    }
    else if (path.contains(".jpg"))
    {
        nameList = "JPG (*.jpg);;PNG (*.png);;BMP Files (*.bmp)";
    }
    else
    {
        nameList = "BMP Files (*.bmp);;JPG (*.jpg);;PNG (*.png)";
    }

    QFileDialog dlg(View::getInstance(), "Save image", path, nameList);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.selectFile(name);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    QStringList fileList = dlg.selectedFiles();
    if (fileList.isEmpty())
    {
        qDebug() << "No file selected";
        return;
    }
    QString file = fileList.at(0);
    QString flt  = dlg.selectedNameFilter();
    QString ext;
    if (flt.contains(".png"))
    {
        ext = ".png";
    }
    else if (flt.contains(".jpg"))
    {
        ext = ".jpg";
    }
    else
    {
        ext = ".bmp";
    }
    if (!file.contains(ext))
    {
        file = file + ext;
    }

    qDebug() << "saving:" << file;
    bool rv = pixmap.save(file);

    if (rv)
    {
        qDebug() << file << "saved OK";
        QFileInfo fileInfo(file);
        path = fileInfo.path();
        s.setValue("picPath2",path);

        QMessageBox box;
        box.setIcon(QMessageBox::Information);
        box.setText(QString("File %1 saved: OK").arg(file));
        box.exec();
    }
    else
    {
        qDebug() << file << "save ERROR";
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("ERROR:  File %1 not saved").arg(file));
        box.exec();
    }
}

void page_save::slot_saveSvg()
{
    if (config->viewerType != VIEW_MOSAIC)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Design View");
        box.exec();
        return;
    }

    MosaicPtr mosaic = workspace->getMosaic();
    StylePtr sp = mosaic->getFirstStyle();
    if (!sp)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Styled Image not found");
        box.exec();
        return;
    }

    QString path = config->rootMediaDir;
    QString name = mosaic->getName();
    QString pathplus = path + "/" + name + ".svg";

    QString newPath = QFileDialog::getSaveFileName(View::getInstance(), "Save SVG", pathplus, "SVG files (*.svg)");
    if (newPath.isEmpty())
        return;

    path = newPath;

    View * view = View::getInstance();

    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(view->size());
    generator.setViewBox(view->rect());
    generator.setTitle(QString("SVG Image: %1").arg(name));
    generator.setDescription("Created using Tiled Pattern Maker (David Casper)");

    sp->triggerPaintSVG(&generator);
    view->update();

    QCoreApplication::processEvents();

    QMessageBox box;
    box.setIcon(QMessageBox::Information);
    box.setText(QString("File %1 saved: OK").arg(path));
    box.exec();
}

