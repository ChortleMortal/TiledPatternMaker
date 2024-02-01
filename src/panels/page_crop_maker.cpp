#include <QRadioButton>
#include <QMessageBox>

#include "geometry/crop.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype.h"
#include "misc/border.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "panels/page_crop_maker.h"
#include "viewers/crop_view.h"
#include "viewers/view_controller.h"
#include "widgets/crop_widget.h"

using std::string;
using std::make_shared;

page_crop_maker::page_crop_maker(ControlPanel * apanel)  : panel_page(apanel,PAGE_CROP_MAKER,"Crop Maker")
{
    cropViewer = CropViewer::getInstance();
    lockView   = false;

    cropWidget = new CropWidget();
    connect(cropWidget, &CropWidget::sig_cropModified, this, [this]() { view->update(); } );
    connect(cropWidget, &CropWidget::sig_cropChanged,  this, [this]() { emit sig_refreshView(); } );

    QHBoxLayout * layout2 = createCropControls();

    vbox->addWidget(cropWidget);
    vbox->addLayout(layout2);
    vbox->addStretch();
    adjustSize();
}

QHBoxLayout * page_crop_maker::createCropControls()
{
    // line 1
    QPushButton * pbCreate       = new QPushButton("Create Crop");
    QPushButton * pbRemove       = new QPushButton("Remove Crop");
    QPushButton * pbFetchBorder  = new QPushButton("Fetch Border Crop");
    QPushButton * pbReApply      = new QPushButton("Re-apply Crop");
    QCheckBox   * chkLock        = new QCheckBox("Lock View");
                  chkEmbed       = new QCheckBox("Embed Crop");
                  chkApply       = new QCheckBox("Crop Outside");

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(pbCreate);
    hbox1->addWidget(chkEmbed);
    hbox1->addWidget(chkApply);
    hbox1->addWidget(chkLock);
    hbox1->addWidget(pbReApply);
    hbox1->addStretch();
    hbox1->addWidget(pbFetchBorder);
    hbox1->addStretch();
    hbox1->addWidget(pbRemove);

    connect(pbCreate,          &QPushButton::clicked,            this, &page_crop_maker::slot_createCrop);
    connect(chkEmbed,          &QCheckBox::clicked,              this, &page_crop_maker::slot_embedCrop);
    connect(chkApply,          &QCheckBox::clicked,              this, &page_crop_maker::slot_applyCrop);
    connect(chkLock,           &QCheckBox::clicked,              this, [this](bool checked) { lockView = checked; } );
    connect(pbRemove,          &QPushButton::clicked,            this, &page_crop_maker::slot_removeCrop);
    connect(pbFetchBorder,     &QPushButton::clicked,            this, &page_crop_maker::slot_fetchBorder);
    connect(pbReApply,         &QPushButton::clicked,            this, &page_crop_maker::slot_reApplyCrop);

    return hbox1;
}

void  page_crop_maker::onRefresh()
{
    display();
}

void page_crop_maker::onEnter()
{
    panel->pushPanelStatus("Left-click on corner to Resize - Left-click inside or on edge to Move");

    cropViewer->init(&cropMaker);
    if (lockView)
    {
        cropViewer->setShowCrop(true);
    }
    emit sig_refreshView();
}

void page_crop_maker::onExit()
{
    panel->popPanelStatus();

    if (!lockView)
    {
        cropViewer->setShowCrop(false);
        emit sig_refreshView();
    }
}

void page_crop_maker::display()
{
    bool embed = false;
    bool apply = false;
    bool cropState;

    auto crop = cropMaker.getCrop();
    if (crop)
    {
        if (crop->getEmbed())
            embed = true;
        if (crop->getApply())
            apply = true;
        cropState = true;
    }
    else
    {
        cropState = false;
    }

    if (cropViewer->getShowCrop() != cropState)
    {
        cropViewer->setShowCrop(cropState);
        emit sig_refreshView();
    }

    chkEmbed->blockSignals(true);
    chkEmbed->setChecked(embed);
    chkEmbed->blockSignals(false);
    chkApply->blockSignals(true);
    chkApply->setChecked(apply);
    chkApply->blockSignals(false);

    cropWidget->setCrop(crop);
    cropWidget->refresh();
}

void page_crop_maker::slot_createCrop()
{
    auto crop  = cropMaker.createCrop();
    cropMaker.setCrop(crop);
    emit sig_refreshView();
}

void page_crop_maker::slot_embedCrop(bool checked)
{
    bool rv = cropMaker.setEmbed(checked);
    if (rv)
    {
        emit sig_refreshView();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not set Embed Crop");
        box.exec();
    }
}

void page_crop_maker::slot_applyCrop(bool checked)
{
    bool rv = cropMaker.setApply(checked);
    if (rv)
    {
        emit sig_refreshView();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not set Apply Crop");
        box.exec();
    }
}

void page_crop_maker::slot_reApplyCrop()
{
    auto crop = cropMaker.getCrop();
    mosaicMaker->getMosaic()->setCrop(crop);
    emit sig_refreshView();
}


void page_crop_maker::slot_removeCrop()
{
    cropMaker.removeCrop();
    emit sig_refreshView();
}

void page_crop_maker::slot_fetchBorder()
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

    CropPtr ocrop = std::dynamic_pointer_cast<Crop>(border);
    CropPtr crop = make_shared<Crop>(ocrop);
    crop->transform(cropViewer->getLayerTransform().inverted());
    cropMaker.setCrop(crop);
    mosaic->setCrop(crop);

    emit sig_refreshView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Fetch OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}
