#include <QRadioButton>
#include <QMessageBox>

#include "gui/panels/page_crop_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/widgets/crop_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/prototype.h"
#include "sys/geometry/crop.h"

using std::string;
using std::make_shared;

page_crop_maker::page_crop_maker(ControlPanel * apanel)  : panel_page(apanel,PAGE_CROP_MAKER,"Crop Maker")
{
    pageStatusString = "Left-click on corner to Resize - Righ-click inside or on edge to Move";

    QGroupBox * mosaicGroup = new QGroupBox("Moisaic Crop - mosaic crops are applied to prototype maps");

    mosaicWidget = new CropWidget();

    connect(mosaicWidget, &CropWidget::sig_cropModified, this, &page_crop_maker::slot_applyCrop);
    connect(mosaicWidget, &CropWidget::sig_cropChanged,  this, &page_crop_maker::slot_applyCrop );

    auto layout = createMosaicCropControls();

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addWidget(mosaicWidget);
    vb->addLayout(layout);
    mosaicGroup->setLayout(vb);

    QGroupBox * painterGroup = new QGroupBox("Painter Crop - painter crops form a clip region for painting the whole mosaic");

    painterWidget = new CropWidget();

    connect(painterWidget, &CropWidget::sig_cropModified, this, [this]() { emit sig_updateView(); } );
    connect(painterWidget, &CropWidget::sig_cropChanged,  this, [this]() { emit sig_reconstructView(); } );

    layout = createPainterCropControls();

    vb = new QVBoxLayout;
    vb->addWidget(painterWidget);
    vb->addLayout(layout);
    painterGroup->setLayout(vb);

    vbox->addWidget(mosaicGroup);
    vbox->addWidget(painterGroup);
    vbox->addStretch();
    adjustSize();
}

QGridLayout * page_crop_maker::createMosaicCropControls()
{
    // line 1
    QPushButton * pbCreate      = new QPushButton("Create Crop");
    QPushButton * pbRemove      = new QPushButton("Remove Crop");
    QPushButton * pbFetchBorder = new QPushButton("Fetch Border Crop");
    QPushButton * pbApply       = new QPushButton("Apply Crop");
    chkShowMosaic               = new QCheckBox("Show Crop");
    chkEmbed                    = new QCheckBox("Embed Crop");
    chkCropOutside              = new QCheckBox("Crop Outside");

    QGridLayout * gl = new QGridLayout();

    int row = 0;
    gl->addWidget(chkShowMosaic,row,0);
    gl->addWidget(chkEmbed,row,2);
    gl->addWidget(chkCropOutside,row,3);

    row++;
    gl->addWidget(pbCreate,row,0);
    gl->addWidget(pbApply,row,1);
    gl->addWidget(pbFetchBorder,row,2);
    gl->addWidget(pbRemove,row,4);


    connect(chkEmbed,          &QCheckBox::clicked,              this, &page_crop_maker::slot_embedCrop);
    connect(chkCropOutside,    &QCheckBox::clicked,              this, &page_crop_maker::slot_cropOutside);
    connect(chkShowMosaic,     &QCheckBox::toggled,              this, &page_crop_maker::slot_showMosaicCrop);

    connect(pbCreate,          &QPushButton::clicked,            this, &page_crop_maker::slot_createMosaicCrop);
    connect(pbRemove,          &QPushButton::clicked,            this, &page_crop_maker::slot_removeMosaicCrop);
    connect(pbFetchBorder,     &QPushButton::clicked,            this, &page_crop_maker::slot_fetchBorder);
    connect(pbApply,           &QPushButton::clicked,            this, &page_crop_maker::slot_applyCrop);

    return gl;
}

QGridLayout * page_crop_maker::createPainterCropControls()
{
    // line 1
    QPushButton * pbCreate      = new QPushButton("Create Crop");
    QPushButton * pbRemove      = new QPushButton("Remove Crop");
    QPushButton * pbFetchBorder = new QPushButton("Fetch Border Crop");
    QPushButton * pbFetchMosaic = new QPushButton("Fetch Mosaic Crop");
    chkShowPainter              = new QCheckBox("Show Crop");
    chkClip                     = new QCheckBox("Painter Clip");
    QLabel      * dummy         = new QLabel;

    QGridLayout * gl = new QGridLayout();

    int row = 0;
    gl->addWidget(chkShowPainter,row,0);
    gl->addWidget(chkClip,row,2);

    row++;
    gl->addWidget(pbCreate,row,0);
    gl->addWidget(pbFetchMosaic,row,1);
    gl->addWidget(pbFetchBorder,row,2);
    gl->addWidget(dummy,row,3);
    gl->addWidget(pbRemove,row,4);


    connect(chkClip,           &QCheckBox::clicked,              this, &page_crop_maker::slot_clip);
    connect(chkShowPainter,    &QCheckBox::toggled,              this, &page_crop_maker::slot_showPainterCrop);

    connect(pbCreate,          &QPushButton::clicked,            this, &page_crop_maker::slot_createPainterCrop);
    connect(pbRemove,          &QPushButton::clicked,            this, &page_crop_maker::slot_removePainterCrop);
    connect(pbFetchBorder,     &QPushButton::clicked,            this, &page_crop_maker::slot_fetchBorderForPainter);
    connect(pbFetchMosaic,     &QPushButton::clicked,            this, &page_crop_maker::slot_fetchMosaicCrop);

    return gl;
}

void  page_crop_maker::onRefresh()
{
    display();
}

void page_crop_maker::onEnter()
{
    emit sig_reconstructView();
}

void page_crop_maker::display()
{
    // display mosaic crop
    bool embed = false;
    bool apply = false;

    auto crop = mosaicCropMaker.getCrop();
    if (crop)
    {
        if (crop->getEmbed())
            embed = true;
        if (crop->getApply())
            apply = true;
    }

    chkEmbed->blockSignals(true);
    chkEmbed->setChecked(embed);
    chkEmbed->blockSignals(false);

    chkCropOutside->blockSignals(true);
    chkCropOutside->setChecked(apply);
    chkCropOutside->blockSignals(false);

    chkShowMosaic->blockSignals(true);
    chkShowMosaic->setChecked(Sys::cropMakerView->getShowCrop(CM_MOSAIC));
    chkShowMosaic->blockSignals(false);

    mosaicWidget->setCrop(crop);
    mosaicWidget->refresh();

    // display painter crop

    bool clip  = false;
    crop = painterCropMaker.getCrop();
    if (crop)
    {
        if (crop->getClip())
            clip = true;
    }

    chkClip->blockSignals(true);
    chkClip->setChecked(clip);
    chkClip->blockSignals(false);

    chkShowPainter->blockSignals(true);
    chkShowPainter->setChecked(Sys::cropMakerView->getShowCrop(CM_PAINTER));
    chkShowPainter->blockSignals(false);

    painterWidget->setCrop(crop);
    painterWidget->refresh();
}

void page_crop_maker::slot_createMosaicCrop()
{
    auto crop  = mosaicCropMaker.createCrop();
    mosaicCropMaker.setCrop(crop);
    mosaicWidget->setCrop(crop);
    chkShowMosaic->setChecked(true);
    emit sig_reconstructView();
}

void page_crop_maker::slot_createPainterCrop()
{
    auto crop  = painterCropMaker.createCrop();
    painterCropMaker.setCrop(crop);
    painterWidget->setCrop(crop);
    chkShowPainter->setChecked(true);
    emit sig_reconstructView();
}

void page_crop_maker::slot_embedCrop(bool checked)
{
    bool rv = mosaicCropMaker.setEmbed(checked);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not set Embed Crop");
        box.exec();
    }

    slot_applyCrop();
}

void page_crop_maker::slot_cropOutside(bool checked)
{
    bool rv = mosaicCropMaker.setCropOutside(checked);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not set Apply Crop");
        box.exec();
    }

    slot_applyCrop();
}

void page_crop_maker::slot_showMosaicCrop(bool checked)
{
    if (checked)
    {
        Sys::cropMakerView->aquire(&mosaicCropMaker,CM_MOSAIC);
    }
    Sys::cropMakerView->setShowCrop(CM_MOSAIC,checked);
    emit sig_reconstructView();
}

void page_crop_maker::slot_showPainterCrop(bool checked)
{
    if (checked)
    {
        Sys::cropMakerView->aquire(&painterCropMaker,CM_PAINTER);
    }
    Sys::cropMakerView->setShowCrop(CM_PAINTER,checked);
    emit sig_reconstructView();
}

void page_crop_maker::slot_clip(bool checked)
{
    bool rv = painterCropMaker.setClip(checked);
    if (rv)
    {
        emit sig_reconstructView();
    }
}

void page_crop_maker::slot_applyCrop()
{
    auto crop = mosaicCropMaker.getCrop();
    if (crop)
    {
        mosaicMaker->getMosaic()->setCrop(crop);
        emit sig_reconstructView();
    }
    else
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Apply failed: No crop to apply");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
}

void page_crop_maker::slot_removeMosaicCrop()
{
    mosaicCropMaker.removeCrop();
    emit sig_reconstructView();
}

void page_crop_maker::slot_removePainterCrop()
{
    mosaicCropMaker.removePainterCrop();
    emit sig_reconstructView();
}

void page_crop_maker::slot_fetchBorder()
{
    qDebug() << "page_crop_maker::slot_fetchBorder";
    auto mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No mosaic");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    BorderPtr border = mosaic->getBorder();
    if (!border)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No border");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    Crop ocrop   = *(std::dynamic_pointer_cast<Crop>(border).get());
    CropPtr crop = make_shared<Crop>(ocrop);
    mosaicCropMaker.setCrop(crop);
    mosaic->setCrop(crop);

    // map the crop rect
    QRectF r;
    r = crop->getRect();                        // border model units
    r = border->modelToScreen(r);               // screen units
    r = Sys::cropMakerView->screenToModel(r);   // mosaic crop model units
    crop->setRect(r);

    //crop->transform(Sys::cropViewer->getLayerTransform().inverted());
    emit sig_reconstructView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Fetch OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_crop_maker::slot_fetchBorderForPainter()
{
    auto mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No mosaic");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    BorderPtr border = mosaic->getBorder();
    if (!border)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No border");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    Crop ocrop   = *(std::dynamic_pointer_cast<Crop>(border).get());
    CropPtr crop = make_shared<Crop>(ocrop);
    crop->transform(Sys::cropMakerView->getLayerTransform().inverted());
    painterCropMaker.setCrop(crop);
    mosaic->setPainterCrop(crop);

    // map the crop rect
    QRectF r;
    r = crop->getRect();                        // border model units
    r = border->modelToScreen(r);               // screen units
    r = Sys::cropMakerView->screenToModel(r);   // painter crop model units
    crop->setRect(r);
    emit sig_reconstructView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Fetch OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_crop_maker::slot_fetchMosaicCrop()
{
    CropPtr mosCr  = mosaicCropMaker.getCrop();
    if (mosCr)
    {
        CropPtr cp = make_shared<Crop>(*mosCr.get());
        mosaicMaker->getMosaic()->setPainterCrop(cp);
        painterCropMaker.setCrop(cp);
        emit sig_reconstructView();
    }
    else
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No Crop in Mosaic");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
}
