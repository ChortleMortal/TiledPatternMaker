#include <QRadioButton>
#include <QMessageBox>

#include "panels/page_crop.h"
#include "geometry/crop.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/crop_maker/crop_maker.h"
#include "misc/border.h"
#include "mosaic/mosaic.h"
#include "mosaic/prototype.h"
#include "panels/panel.h"
#include "viewers/crop_view.h"
#include "viewers/viewcontrol.h"
#include "widgets/crop_widget.h"

using std::string;
using std::make_shared;

page_crop::page_crop(ControlPanel * apanel)  : panel_page(apanel,"Crop Maker")
{
    cropMaker = CropMaker::getInstance();
    crview = CropView::getSharedInstance();

    cw = new CropWidget();
    connect(cw, &CropWidget::sig_cropModified, this, [this]() { view->update(); } );
    connect(cw, &CropWidget::sig_cropChanged,  this, [this]() { emit sig_refreshView(); } );

    QHBoxLayout * layout1 = createCropControls();

    vbox->addWidget(cw);
    vbox->addLayout(layout1);
    adjustSize();
}

QHBoxLayout * page_crop::createCropControls()
{
    // line 1
    QPushButton * pbCreate       = new QPushButton("Create Crop");
    QPushButton * pbEmbed        = new QPushButton("Embed Crop");
    QPushButton * pbApply        = new QPushButton("Crop Outside");
    QPushButton * pbRemove       = new QPushButton("Remove Crop");
    QPushButton * pbFetchBorder  = new QPushButton("Fetch Border Settings");
                  pbEditing      = new QPushButton("Finish Editing");

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(pbCreate);
    hbox1->addWidget(pbEmbed);
    hbox1->addWidget(pbApply);
    hbox1->addWidget(pbEditing);
    hbox1->addStretch();
    hbox1->addWidget(pbFetchBorder);
    hbox1->addStretch();
    hbox1->addWidget(pbRemove);

    connect(pbCreate,          &QPushButton::clicked,            this, &page_crop::slot_createCrop);
    connect(pbEmbed,           &QPushButton::clicked,            this, &page_crop::slot_embedCrop);
    connect(pbApply,           &QPushButton::clicked,            this, &page_crop::slot_applyCrop);
    connect(pbRemove,          &QPushButton::clicked,            this, &page_crop::slot_removeCrop);
    connect(pbFetchBorder,     &QPushButton::clicked,            this, &page_crop::slot_fetchBorder);
    connect(pbEditing,         &QPushButton::clicked,            this, &page_crop::slot_editCrop);

    return hbox1;
}

void  page_crop::onRefresh()
{
    display();
}

void page_crop::onEnter()
{

    display();
}

void page_crop::display()
{
    static CropPtr oldCrop;
    auto mosaic =  mosaicMaker->getMosaic();
    if (mosaic)
    {
        auto crop = mosaic->getCrop();
        if (crop != oldCrop)
        {
            crop = cropMaker->loadCrop();
            oldCrop = crop;
            cw->setCrop(crop);
            view->update();
        }
    }
    else if (oldCrop)
    {
        oldCrop.reset();
        cw->setCrop(oldCrop);
        view->update();
    }

    cw->refresh();

    switch (cropMaker->getState())
    {
    case CROPMAKER_STATE_INACTIVE:
        pbEditing->setText("");
        break;
    case CROPMAKER_STATE_ACTIVE:
        pbEditing->setText("Finish Editing");
        break;
    case CROPMAKER_STATE_COMPLETE:
        pbEditing->setText("Restart Editing");
        break;
    }
}

void page_crop::slot_createCrop()
{
    auto crop  = cropMaker->createCrop();

    cw->setCrop(crop);

    emit sig_refreshView();
}

void page_crop::slot_editCrop()
{
    if (!cropMaker->getCrop())
    {
        return;
    }

    switch (cropMaker->getState())
    {
    case CROPMAKER_STATE_ACTIVE:
        cropMaker->setState(CROPMAKER_STATE_COMPLETE);
        break;
    case CROPMAKER_STATE_INACTIVE:
    case CROPMAKER_STATE_COMPLETE:
        cropMaker->setState(CROPMAKER_STATE_ACTIVE);
        break;
    }
    view->update();
}


#if 0
void page_crop::slot_createCropOld()
{
    auto crop = maped->createCrop();    // need to do this here

    panel->setCurrentPage("Map Editor");
    panel_page      * pp  = panel->getCurrentPage();
    page_map_editor * pme = dynamic_cast<page_map_editor *>(pp);
    if (pme)
    {
        pme->slot_setMouseMode(MAPED_MOUSE_EDIT_CROP,true);
        emit sig_refreshView();
    }

    if (crop->getCropType() == CROP_RECTANGLE)
    {
        QString msg =  QString("You can now resize the rectangle by clicking and dragging.\n")
                     + QString("Then:\n")
                     + QString("    Press 'Embed Crop' to embed the crop in the map\n")
                     + QString("    Press 'Apply Crop' to remove everything outside of the crop");
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText(msg);
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
}
#endif

void page_crop::slot_embedCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        auto crop = mosaic->getCrop();
        if (crop)
        {
            crop->embed();
            if (crop->isUsed())
            {
                CropMaker::getInstance()->setState(CROPMAKER_STATE_COMPLETE);
            }

            mosaic->resetProtoMaps();
            emit sig_refreshView();

            QMessageBox box(this);
            box.setIcon(QMessageBox::Information);
            box.setText("Embed Crop : OK");
            box.exec();
            return;
        }
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setText("Embed Crop : FAILED");
    box.exec();
}

void page_crop::slot_applyCrop()
{
    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        auto crop = mosaic->getCrop();
        if (crop)
        {
            crop->apply();
            if (crop->isUsed())
            {
                CropMaker::getInstance()->setState(CROPMAKER_STATE_COMPLETE);
            }

            mosaic->resetProtoMaps();
            emit sig_refreshView();

            QMessageBox box(this);
            box.setIcon(QMessageBox::Information);
            box.setText("Apply Crop : OK");
            box.exec();
            return;
        }
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setText("Apply Crop : FAILED");
    box.exec();
}

void page_crop::slot_removeCrop()
{
    CropMaker::getInstance()->removeCrop();
    emit sig_refreshView();
}

void page_crop::slot_fetchBorder()
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

    CropPtr bordercrop = border;
    Q_ASSERT(bordercrop);

    CropPtr crop = make_shared<Crop>(bordercrop);
    crop->transform(crview->getLayerTransform().inverted());
    mosaic->resetCrop(crop);

    cw->setCrop(crop);
    display();
    emit sig_refreshView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Fetch OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_crop::slot_centerCrop()
{
#if 0
    const Xform & xf = view->getCurrentXform2();
    QPointF center = xf.getModelCenter();
    centerX[0]->setValue(center.x());
    centerY[0]->setValue(center.y());
    slot_circleChanged(0);
#endif
}


CropPtr page_crop::getMosaicCrop()
{
    CropPtr crop;

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        crop = mosaic->getCrop();
    }
    return crop;
}
