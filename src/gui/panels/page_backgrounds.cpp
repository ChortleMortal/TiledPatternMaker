#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/panels/page_backgrounds.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/dlg_name.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "sys/enums/emousemode.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"

page_backgrounds::page_backgrounds(ControlPanel * apanel) : panel_page(apanel,PAGE_BKGD_MAKER,"Backgrounds"),  tformLayout("Bkgd Xform")
{
    QGroupBox * bkgdImageGroup = createBackgroundImageGroup();
    QGroupBox * bkgdColorGroup = createBackgroundColorGroup();

    vbox->addWidget(bkgdImageGroup);
    vbox->addWidget(bkgdColorGroup);
    vbox->addStretch();
}

void page_backgrounds::onEnter()
{
    displayBackgroundImageStatus(true);
}

void page_backgrounds::onExit()
{
    clearPageStatus();
}

void page_backgrounds::onRefresh()
{
    chkView->setChecked(viewControl->isEnabled(VIEW_BKGD_IMG));

    displayBackgroundImageStatus(false);

    auto bip = Sys::getBackgroundImageFromSource();
    smxWidget->setLayer(bip.get());
    smxWidget->refresh();

    for (int i = 0; i < viewTable->rowCount(); i++)
    {
        QTableWidgetItem * item = viewTable->item(i,1);
        QColor color = viewControl->getBackgroundColor(static_cast<eViewType>(i));
        item->setText(color.name());

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        viewTable->setCellWidget(i,2,label);
    }
}

QGroupBox * page_backgrounds::createBackgroundImageGroup()
{
    chkView         = new QCheckBox("View");
    imageName       = new QLineEdit("Image name");
    chkShowAdjusted = new QCheckBox("Show Adjusted");
    chkShowCropped  = new QCheckBox("Show Cropped");
    btnNone         = new QRadioButton("None");
    btnMosaic       = new QRadioButton("Mosaic");
    btnTiling       = new QRadioButton("Tiling");
    btnMaped        = new QRadioButton("Map Editor");
    btnDefine       = new QRadioButton("Engine");
    smxWidget       = new SMXWidget(nullptr,false,true);

    QPushButton * resetBtn      = new QPushButton("Reset Xform");
    QPushButton * loadBkgdBtn   = new QPushButton("Load");
    QPushButton * removeBkgdBtn = new QPushButton("Remove");
    QPushButton * clearBtn      = new QPushButton("Clear Content");

    startAdjustBtn                      = new AQPushButton("Start Keystone Adjustment");
    QPushButton  * completeAdjustBtn    = new QPushButton("Complete Keystone Adustment");
    QPushButton  * saveAdjustedBtn      = new QPushButton("Save Adjusted");

    startCropBtn                        = new AQPushButton("Start Image Crop");
    QPushButton  * endCropBtn           = new QPushButton("Complete Image Crop");
    QPushButton  * saveCroppedBtn       = new QPushButton("Save Image Cropped");

    startAdjustBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");
    startCropBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");

    QHBoxLayout * hb1 = new QHBoxLayout();
    hb1->addWidget(loadBkgdBtn);
    hb1->addWidget(imageName);
    hb1->addWidget(removeBkgdBtn);
    hb1->addWidget(clearBtn);

    QHBoxLayout * hb2 = new  QHBoxLayout();
    hb2->addWidget(chkView);
    hb2->addStretch();
    hb2->addWidget(btnNone);
    hb2->addWidget(btnMosaic);
    hb2->addWidget(btnTiling);
    hb2->addWidget(btnMaped);
    hb2->addWidget(btnDefine);
    hb2->addStretch();
    hb2->addWidget(smxWidget);

    QHBoxLayout * hb3 = new QHBoxLayout();
    hb3->addLayout(&tformLayout);
    hb3->addWidget(resetBtn);

    int row = 0;
    QGridLayout * grid = new QGridLayout();
    grid->addWidget(startAdjustBtn,row,0);
    grid->addWidget(completeAdjustBtn,row,1);
    grid->addWidget(saveAdjustedBtn,row,2);
    grid->addWidget(chkShowAdjusted,row,3);

    row++;
    grid->addWidget(startCropBtn,row,0);
    grid->addWidget(endCropBtn,row,1);
    grid->addWidget(saveCroppedBtn,row,2);
    grid->addWidget(chkShowCropped,row,3);

    QVBoxLayout * vb = new QVBoxLayout();
    vb->addLayout(hb1);
    vb->addLayout(hb2);
    vb->addLayout(hb3);
    vb->addSpacing(5);
    vb->addLayout(grid);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Image");
    bkgdGroup->setLayout(vb);

    connect(chkView,       &QCheckBox::clicked,           this, &page_backgrounds::slot_showImageChanged);
    connect(resetBtn,      &QPushButton::clicked,         this, &page_backgrounds::slot_resetXform);
    connect(&tformLayout,  &LayoutTransform::xformChanged,this, &page_backgrounds::slot_setBkgdXform);

    connect(btnNone,       &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_NONE;   displayBackgroundImageStatus(true);  emit sig_updateView();   } );
    connect(btnMosaic,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_MOSAIC; displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnTiling,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_TILING; displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnMaped,      &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_MAPED;  displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnDefine,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_DEFINED;displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(chkShowCropped,   &QCheckBox::clicked, this, [this] (bool checked) { useCropped(checked); emit sig_updateView(); });
    connect(chkShowAdjusted,  &QCheckBox::clicked, this, [this] (bool checked) { useAdjusted(checked);emit sig_updateView(); });

    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_backgrounds::slot_loadBackground);
    connect(removeBkgdBtn,     &QPushButton::clicked,         this,    &page_backgrounds::slot_removeBackground);
    connect(clearBtn,          &QPushButton::clicked,         this,    &page_backgrounds::slot_clearBackground);

    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_backgrounds::slot_startKeystoneAdjustment);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_backgrounds::slot_endKeystoneAdjust);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    [this] { saveModified(true); });

    connect(startCropBtn,      &QPushButton::clicked,         this,    &page_backgrounds::slot_startCrop);
    connect(endCropBtn,        &QPushButton::clicked,         this,    &page_backgrounds::slot_endCrop);
    connect(saveCroppedBtn,    &QPushButton::clicked,         this,    [this] { saveModified(false); });

    return bkgdGroup;
}

QGroupBox * page_backgrounds::createBackgroundColorGroup()
{
    viewTable              = new AQTableWidget();
    QPushButton * pbResetB = new QPushButton("Reset to black");
    QPushButton * pbResetW = new QPushButton("Reset to white");
    QHBoxLayout * hbox     = new QHBoxLayout();
    hbox->addWidget(pbResetB);
    hbox->addSpacing(13);
    hbox->addWidget(pbResetW);
    hbox->addStretch();

    QVBoxLayout * vbox  = new QVBoxLayout();
    vbox->addWidget(viewTable);
    vbox->addLayout(hbox);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Colors");
    bkgdGroup->setLayout(vbox);

    QStringList qslH;
    qslH << "View" << "BkgdColor" << "Color" << "Edit";
    viewTable->setColumnCount(4);
    viewTable->setHorizontalHeaderLabels(qslH);
    viewTable->horizontalHeader()->setVisible(true);
    viewTable->verticalHeader()->setVisible(false);

    int row = 0;
    for (int i= 0; i < NUM_VIEW_TYPES; i++)
    {
        eViewType vtype = static_cast<eViewType>(i);

        viewTable->setRowCount(row +1);

        QTableWidgetItem * item =  new QTableWidgetItem(s2ViewerType[vtype]);
        viewTable->setItem(row,0,item);

        QColor color = viewControl->getBackgroundColor(vtype);
        item = new QTableWidgetItem(color.name());
        viewTable->setItem(row,1,item);

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        viewTable->setCellWidget(row,2,label);

        QPushButton * btn = new QPushButton("Select color");
        viewTable->setCellWidget(row,3,btn);
        connect(btn, &QPushButton::clicked, this, [this,row] { selectColor(row); });

        row++;
    }

    viewTable->resizeColumnsToContents();
    viewTable->adjustTableSize();
    
    connect(pbResetB, &QPushButton::clicked, this, [this] { reInitBkgdColors(QColor(Qt::black)); });
    connect(pbResetW, &QPushButton::clicked, this, [this] { reInitBkgdColors(QColor(Qt::white)); });

    return bkgdGroup;
}

void page_backgrounds::displayBackgroundImageStatus(bool force)
{
    if (!refresh && !force)
    {
        return;
    }

    switch (Sys::currentBkgImage)
    {
    case BKGD_IMAGE_NONE:
        if (!btnNone->isChecked())
            btnNone->setChecked(true);
        break;
    case BKGD_IMAGE_MOSAIC:
        if (!btnMosaic->isChecked())
            btnMosaic->setChecked(true);
        break;
    case BKGD_IMAGE_TILING:
        if (!btnTiling->isChecked())
            btnTiling->setChecked(true);
        break;
    case BKGD_IMAGE_MAPED:
        if (!btnMaped->isChecked())
            btnMaped->setChecked(true);
        break;
    case BKGD_IMAGE_DEFINED:
        if (!btnDefine->isChecked())
            btnDefine->setChecked(true);
    }

    auto bip   = Sys::getBackgroundImageFromSource();
    if (bip && bip->isLoaded())
    {
        const Xform & xform = bip->getModelXform();
        tformLayout.blockSignals(true);
        tformLayout.setTransform(xform);
        tformLayout.blockSignals(false);

        imageName->setText(bip->getTitle());
#if 0
        chkShowAdjusted->blockSignals(true);
        chkShowAdjusted->setChecked(bip->useAdjusted());
        chkShowAdjusted->blockSignals(false);
#endif
    }
    else
    {
        imageName->setText("None");
        tformLayout.init();
#if 0
        chkShowAdjusted->blockSignals(true);
        chkShowAdjusted->setChecked(false);
        chkShowAdjusted->blockSignals(false);
#endif
    }
}

void page_backgrounds::slot_loadBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.jpeg)");
    if (filename.isEmpty()) return;

    if (filename.contains(".heic"))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("HEIC files not supported: %1").arg(filename));
        box.exec();
        return;
    }
    
    auto bip = std::make_shared<BackgroundImage>();
    bool rv  = bip->importIfNeeded(filename);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Failed to import image: %1").arg(filename));
        box.exec();
        return;
    }

    QFileInfo info(filename);
    QString name = info.fileName();
    rv = bip->load(name);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Failed to load image: %1").arg(filename));
        box.exec();
        return;
    }

    // if already loaded, replace, else load
    switch (Sys::currentBkgImage)
    {
    case BKGD_IMAGE_NONE:
    {
        auto mosaic = mosaicMaker->getMosaic();
        if (Sys::viewController->isEnabled(VIEW_TILING_MAKER) || Sys::viewController->isEnabled(VIEW_TILING) || mosaic->getName() == Sys::defaultMosaicName)
        {
            // The owner of the image is the mosaic
            auto tiling = tilingMaker->getSelected();
            Q_ASSERT(tiling);
            tiling->setBkgdImage(bip);
            Sys::currentBkgImage = BKGD_IMAGE_TILING;
        }
        else
        {
            // The owner of the image is the mosaic
            auto mosaic = mosaicMaker->getMosaic();
            Q_ASSERT(mosaic);
            mosaic->setBkgdImage(bip);
            Sys::currentBkgImage = BKGD_IMAGE_MOSAIC;
        }
    }   break;

    case BKGD_IMAGE_MOSAIC:
    {
        auto mosaic = mosaicMaker->getMosaic();
        mosaic->setBkgdImage(bip);
    }   break;

    case BKGD_IMAGE_TILING:
    {
        auto tiling = tilingMaker->getSelected();
        tiling->setBkgdImage(bip);
    }   break;

    case BKGD_IMAGE_MAPED:
        Sys::mapEditor->getDb()->setBackgroundImage(bip);
        break;

    case BKGD_IMAGE_DEFINED:
        break;
    }

    setupBackgroundPixmap();
    panel->delegateView(VIEW_BKGD_IMG,true);     // since we loaded it, might as well see it
    displayBackgroundImageStatus(true);
}

void page_backgrounds::slot_removeBackground()
{
    switch (Sys::currentBkgImage)
    {
    case BKGD_IMAGE_MOSAIC:
    {
        auto mosaic = mosaicMaker->getMosaic();
        mosaic->removeBkgdImage();
    }   break;

    case BKGD_IMAGE_TILING:
    {
        auto tiling = tilingMaker->getSelected();
        tiling->removeBkgdImage();
    }   break;

    case BKGD_IMAGE_MAPED:
        Sys::mapEditor->getDb()->removeBackgroundImage();
        break;

    case BKGD_IMAGE_NONE:
    case BKGD_IMAGE_DEFINED:
        break;
    }

    emit sig_reconstructView();

    displayBackgroundImageStatus(true);
}

void page_backgrounds::slot_clearBackground ()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    bip->unload();

    emit sig_reconstructView();

    displayBackgroundImageStatus(true);
}

void page_backgrounds::setupBackgroundPixmap()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;
    
    const Xform & xf = tformLayout.getXform();
    bip->setModelXform(xf,false,Sys::nextSigid());
    bip->createPixmap();
}

void page_backgrounds::slot_setBkgdXform()
{
    setupBackgroundPixmap();
    emit sig_updateView();
}

void page_backgrounds::slot_resetXform()
{
    tformLayout.init();
    setupBackgroundPixmap();
    emit sig_updateView();
}

void page_backgrounds::slot_startKeystoneAdjustment(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip || !bip->isLoaded())
    {
        startAdjustBtn->setChecked(false);
        return;
    }

    Sys::setSysMouseMode(MOUSE_MODE_NONE,true);    // ensure mouse gets here

    if (checked)
    {
        if (bip->cropper.isActive())
        {
            // turn off cropper
            bip->cropper.activate(false);
            startCropBtn->setChecked(false);
        }

        pageStatusString = "Click to select four points on background image. Then press 'Complete Perspective Adjustement' to fix camera skew.";
        setPageStatus();

        bip->adjuster.activate(true);
    }
    else
    {
        clearPageStatus();
        bip->adjuster.activate(false);    // also stops mouse interaction
    }

    emit sig_updateView();
}

void page_backgrounds::slot_endKeystoneAdjust()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    if (!bip->adjuster.isActive())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please press: Start Keystone Adjustment");
        box.exec();
        return;
    }

    EdgeSet & saccum = bip->adjuster.getAccum();
    if (saccum.size() != 4)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select four points to adjust keystone perspective");
        box.exec();
        return;
    }

    bip->createBackgroundAdjustment(saccum[0]->v1->pt, saccum[1]->v1->pt, saccum[2]->v1->pt, saccum[3]->v1->pt);

    bip->adjuster.activate(false);

    bip->createAdjustedImage();
    useAdjusted(true);
    bip->createPixmap();

    clearPageStatus();
    displayBackgroundImageStatus(true);
    startAdjustBtn->setChecked(false);

    emit sig_updateView();
}

void page_backgrounds::saveModified(bool adjusted)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;
    
    Xform xf = bip->getModelXform();

    QString oldname = bip->getTitle();

    DlgName dlg;
    dlg.newEdit->setText(oldname);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }

    Q_ASSERT(retval == QDialog::Accepted);
    QString newName = dlg.newEdit->text();

    // save
    bool rv;
    if (adjusted)
        rv = bip->saveAdjusted(newName);
    else
        rv = bip->saveCropped(newName);

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Image Save - OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("Image - Save - FAILED");
        box.setInformativeText("Try again with a new name");
    }
    box.exec();

    if (rv)
    {
        // re-load
        bip->load(newName);
        if (bip->isLoaded())
        {
            bip->setModelXform(xf,false,Sys::nextSigid());
            bip->setUseAdjusted(false);
            displayBackgroundImageStatus(true);
            panel->delegateView(VIEW_BKGD_IMG,true);
        }
    }
}

void page_backgrounds::useAdjusted(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    if (checked)
    {
        startCropBtn->setChecked(false);
        chkShowCropped->setChecked(false);
    }

    chkShowAdjusted->setChecked(checked);
    bip->setUseAdjusted(checked);
    setupBackgroundPixmap();
}


void page_backgrounds::slot_startCrop(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip || !bip->isLoaded())
    {
        startCropBtn->setChecked(false);
        return;
    }

    Sys::setSysMouseMode(MOUSE_MODE_NONE,true);    // ensure mouse gets here

    if (checked)
    {
        if (bip->adjuster.isActive())
        {
            // turn off adjuseter
            bip->adjuster.activate(false);
            startAdjustBtn->setChecked(false);
        }

        pageStatusString = "Left-click on corner to Resize - Righ-click inside or on edge to Move";
        setPageStatus();

        bip->cropper.activate(true, bip.get(), &bip->crop);
    }
    else
    {
        clearPageStatus();
        bip->cropper.activate(false);    // also stops mouse interaction
    }

    emit sig_updateView();
}

void page_backgrounds::slot_endCrop()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    if (!bip->cropper.isActive())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please press: Start Image Crop");
        box.exec();
        return;
    }

    bip->cropper.activate(false);

    bip->createCroppedImage();
    useCropped(true);
    bip->createPixmap();

    clearPageStatus();
    displayBackgroundImageStatus(true);
    startCropBtn->setChecked(false);

    emit sig_updateView();
}

void page_backgrounds::useCropped(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    if (checked)
    {
        startAdjustBtn->setChecked(false);
        chkShowAdjusted->setChecked(false);
    }

    chkShowCropped->setChecked(checked);
    bip->setUseCropped(checked);
    setupBackgroundPixmap();
}

void page_backgrounds::slot_showImageChanged(bool checked)
{
    panel->delegateView(VIEW_BKGD_IMG,checked);
}

void page_backgrounds::selectColor(int row)
{
    eViewType vtype = static_cast<eViewType>(row);
    
    QColor color = viewControl->getBackgroundColor(vtype);

    AQColorDialog dlg(color,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        viewControl->setBackgroundColor(vtype,color);
        emit sig_reconstructView();
    }
}

void page_backgrounds::reInitBkgdColors(QColor bcolor)
{
    viewControl->setBackgroundColor(VIEW_LEGACY,bcolor);
    viewControl->setBackgroundColor(VIEW_MOSAIC,bcolor);
    viewControl->setBackgroundColor(VIEW_PROTOTYPE,bcolor);
    viewControl->setBackgroundColor(VIEW_MOTIF_MAKER,bcolor);
    viewControl->setBackgroundColor(VIEW_TILING,bcolor);
    viewControl->setBackgroundColor(VIEW_TILING_MAKER,bcolor);
    viewControl->setBackgroundColor(VIEW_MAP_EDITOR,bcolor);
    viewControl->setBackgroundColor(VIEW_BKGD_IMG,bcolor);
    viewControl->setBackgroundColor(VIEW_GRID,bcolor);
    viewControl->setBackgroundColor(VIEW_CROP,bcolor);
    viewControl->setBackgroundColor(VIEW_DEBUG,bcolor);
    viewControl->setBackgroundColor(VIEW_BMP_IMAGE,bcolor);
    emit sig_reconstructView();
}
