﻿#include <QGroupBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QFileDialog>

#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "gui/panels/page_save.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/tiling.h"
#include "gui/top/view_controller.h"
#include <QSvgGenerator>

page_save::page_save(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_SAVE, "Save")
{
    createMosaicSave();
    createTilingSave();

    QPushButton * pbSaveMenu = new QPushButton("Save Menu BMP (J)");
    pbSaveMenu->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QPushButton * pbSaveImage = new QPushButton("Save Image BMP (P)");
    pbSaveImage->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QPushButton * pbSaveSVG   = new QPushButton("Save Image SVG (Y)");
    pbSaveSVG->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(pbSaveMenu);
    hbox->addSpacing(17);
    hbox->addWidget(pbSaveImage);
    hbox->addSpacing(17);
    hbox->addWidget(pbSaveSVG);
    hbox->addStretch();

    QGroupBox * gbox = new QGroupBox("Image");
    gbox->setLayout(hbox);

    vbox->addWidget(gbox);
    vbox->addStretch();

    connect(mosaicMaker,    &MosaicMaker::sig_mosaicLoaded,     this,   &page_save::setup);
    connect(tilingMaker,    &TilingMaker::sig_tilingLoaded,     this,   &page_save::setup);
    connect(pbSaveMenu,     &QPushButton::clicked,              this,   &page_save::slot_saveMenu);
    connect(Sys::view,      &View::sig_saveMenu,                this,   &page_save::slot_saveMenu);
    connect(Sys::view,      &View::sig_saveImage,               this,   &page_save::slot_saveImage);
    connect(Sys::view,      &View::sig_saveSVG,                 this,   &page_save::slot_saveSvg);
    connect(pbSaveImage,    &QPushButton::clicked,              this,   &page_save::slot_saveImage);
    connect(pbSaveSVG,      &QPushButton::clicked,              this,   &page_save::slot_saveSvg);
}

page_save::~page_save()
{
}

void page_save::createMosaicSave()
{
    leSaveXmlName   = new QLineEdit();

    QPushButton * saveMosaic  = new QPushButton("Save Mosaic");
    saveMosaic->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    designNotes     = new QTextEdit("Design Notes");
    designNotes->setFixedSize(601,101);
    QLabel * label  = new QLabel("Name");

    QCheckBox * chkSaveTest = new QCheckBox("Test");
    chkSaveTest->setChecked(config->saveMosaicTest);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label);
    hbox2->addWidget(leSaveXmlName);
    hbox2->addWidget(chkSaveTest);
    hbox2->addWidget(saveMosaic);

    QLabel * label2 = new QLabel("Description/Notes");

    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addLayout(hbox2);
    vlayout->addWidget(label2);
    vlayout->addWidget(designNotes);

    QGroupBox * saveBox = new QGroupBox("Mosaic");
    saveBox->setLayout(vlayout);
    vbox->addWidget(saveBox);

    connect(saveMosaic, &QPushButton::clicked,      this, &page_save::slot_saveMosaic);
    connect(chkSaveTest,&QCheckBox::clicked,        this, [this](bool checked) {config->saveMosaicTest = checked; });
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
    pbSave->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QLabel * label        = new QLabel("Name");
    QLabel * label2       = new QLabel("Author");
    QLabel * label3       = new QLabel("Description");
    tile_name             = new QLineEdit();
    tile_author           = new QLineEdit();

    QCheckBox * chkSaveTest = new QCheckBox("Test");
    chkSaveTest->setChecked(config->saveTilingTest);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label);
    hbox2->addWidget(tile_name);
    hbox2->addWidget(label2);
    hbox2->addWidget(tile_author);
    hbox2->addWidget(chkSaveTest);
    hbox2->addWidget(pbSave);

    tile_desc = new QTextEdit();
    tile_desc->setFixedHeight(101);

    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addLayout(hbox);
    vlayout->addSpacing(3);
    vlayout->addLayout(hbox2);
    vlayout->addSpacing(3);
    vlayout->addWidget(label3);
    vlayout->addWidget(tile_desc);
    vlayout->addSpacing(3);

    QGroupBox * saveBox = new QGroupBox("Tiling");
    saveBox->setLayout(vlayout);
    vbox->addWidget(saveBox);

    connect(pbSave,     &QPushButton::clicked,      this,   &page_save::slot_saveTiling);
    connect(chkSaveTest,&QCheckBox::clicked,        this,   [this](bool checked) {config->saveTilingTest = checked; });
}

void page_save::onEnter()
{
    setup();
}

void page_save::setup()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        leSaveXmlName->setText(mosaic->getName().get());
        designNotes->setText(mosaic->getNotes());
    }
    else
    {
        leSaveXmlName->setText("");
        designNotes->setText("");
    }

    TilingPtr tiling =  tilingMaker->getSelected();
    blockSignals(true);
    if (tiling)
    {
        tile_desc->setText(tiling->getDescription());
        tile_name->setText(tiling->getName().get());
        tile_author->setText(tiling->getAuthor());
    }
    else
    {
        tile_desc->setText("");
        tile_name->setText("");
        tile_author->setText("");
    }
    blockSignals(false);

    onRefresh();;
}

void page_save::onRefresh()
{
    if (tilingMaker->tilingChanged())
        requiresSave->setText("HAS CHANGED");
    else
        requiresSave->setText("");
}

void page_save::slot_saveMosaic()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Mosaic");
        box.setText("There is no Mosaic to save");
        box.setInformativeText("Please load a tiling and then use the Motdif Maker and the Mosaic Maker");
        box.exec();
        return;
    }

    mosaic->setNotes(designNotes->toPlainText());

    if (!mosaic->hasContent())
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Mosaic");
        box.setText("Mosaic has no content");
        box.setInformativeText("Please load a tiling and then use the Motif Maker and the Mosaic Maker");
        box.exec();
        return;
    }

    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (const auto & tiling : std::as_const(tilings))
    {
        if (tilingMaker->tilingChanged() || (tiling->getName().get()  == Sys::defaultTilingName))
        {
            QMessageBox box(panel);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle("Save Mosaic");
            box.setText("Tiling requires saving");
            box.setInformativeText("Please save tiling first");
            box.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Cancel);
            int rv = box.exec();
            if (rv == QMessageBox::Cancel)
            {
                return;
            }
        }
    }

    VersionedName vname(leSaveXmlName->text());

    if (vname.get() == Sys::defaultMosaicName || vname.isEmpty())
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Mosaic");
        box.setText("Mosaic requires an identifying name");
        box.setInformativeText("Please set a name first");
        box.exec();
        return;
    }

    mosaic->setName(vname);
    mosaicMaker->saveMosaic(mosaic,false);
}


void page_save::slot_designSourceChanged()
{
    setup();
}

void page_save::slot_tilingSourceChanged()
{
    setup();
}

void page_save::slot_saveTiling()
{
    TilingPtr tiling = tilingMaker->getSelected();

    QString name = tile_name->text();

    VersionedName vname(name);

    if (tiling->numIncluded() == 0)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling");
        box.setText("There are no placed tiles.  Please add some tiles.");
        box.exec();
        return;
    }

    int count = tiling->getUniqueTiles().count();
    if (count >= MAX_UNIQUE_TILE_INDEX)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle("Save Tiling");
        box.setText("There could be something wrong with this tiling");
        box.setInformativeText(QString("There are many unique tiles (count=%1)").arg(count));
        box.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Cancel);
        int rv = box.exec();
        if (rv == QMessageBox::Cancel)
        {
            return;
        }
    }

    if (vname.isEmpty() || vname.get().contains(Sys::defaultTilingName))
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling");
        box.setText("There is no tiling name. A tiling name is required.");
        box.exec();
        return;
    }
    
    tiling->setName(vname);
    tiling->setAuthor(tile_author->text());
    tiling->setDescription(tile_desc->toPlainText());

    tilingMaker->saveTiling(tiling);
}

void page_save::slot_saveImage()
{
    VersionedName name = config->lastLoadedMosaic;

    QPixmap pixmap = Sys::view->grab();

    savePixmap(pixmap, name.get());
}

void page_save::slot_saveMenu()
{
    auto page      = panel->getCurrentPage();
    QString name   = page->getName();

    QPixmap pixmap = panel->grab();

    savePixmap(pixmap,name);
}

void page_save::savePixmap(QPixmap & pixmap, QString name)
{
    static bool firstTime = true;
    static QString path;

    QSettings s;
    if (firstTime)
    {
        path = config->rootMediaDir;
        firstTime = false;
    }
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

    QFileDialog dlg(panel, "Save image", path, nameList);
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

    QFileInfo afile(file);
    path = afile.absolutePath();    //saved for next time

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

        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText(QString("File %1 saved: OK").arg(file));
        box.exec();
    }
    else
    {
        qDebug() << file << "save ERROR";
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("ERROR:  File %1 not saved").arg(file));
        box.exec();
    }
}

void page_save::slot_saveSvg()
{
    if (!viewControl->isEnabled(VIEW_MOSAIC))
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Mosaic View");
        box.exec();
        return;
    }

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("There is no Mosaic to save");
        box.exec();
        return;
    }

    StylePtr sp = mosaic->getFirstStyle();
    if (!sp)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Styled Image not found");
        box.exec();
        return;
    }

    QString path = config->rootMediaDir;
    QString name = mosaic->getName().get();
    QString pathplus = path + "/" + name + ".svg";

    QString newPath = QFileDialog::getSaveFileName(panel, "Save SVG", pathplus, "SVG files (*.svg)");
    if (newPath.isEmpty())
        return;

    path = newPath;

    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(Sys::view->size());
    generator.setViewBox(Sys::view->rect());
    generator.setTitle(QString("SVG Image: %1").arg(name));
    generator.setDescription("Created using Tiled Pattern Maker (David Casper)");

    sp->triggerPaintSVG(&generator);
    emit sig_updateView();

    QCoreApplication::processEvents();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText(QString("File %1 saved: OK").arg(path));
    box.exec();
}

