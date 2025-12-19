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
#include "gui/viewers/gui_modes.h"
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

page_backgrounds::page_backgrounds(ControlPanel * apanel) : panel_page(apanel,PAGE_BKGD_MAKER,"Backgrounds"),  bkgdLayout("Bkgd Xform")
{
                bkgdViewGroup  = createBackgroundViewGroup();
    QGroupBox * bkgdImageGroup = createBackgroundImageGroup();
    QGroupBox * bkgdColorGroup = createBackgroundColorGroup();

    vbox->addWidget(bkgdViewGroup);
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
    bkgdViewGroup->setChecked(viewControl->isEnabled(VIEW_BKGD_IMG));

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

QGroupBox * page_backgrounds::createBackgroundViewGroup()
{
    btnNone   = new QRadioButton("None");
    btnMosaic = new QRadioButton("Mosaic");
    btnTiling = new QRadioButton("Tiling");
    btnMaped  = new QRadioButton("Map Editor");
    btnDefine = new QRadioButton("Map Editor Defined");

    smxWidget = new SMXWidget(nullptr,false,true);

    QHBoxLayout * hb = new  QHBoxLayout();
    hb->addWidget(btnNone);
    hb->addWidget(btnMosaic);
    hb->addWidget(btnTiling);
    hb->addWidget(btnMaped);
    hb->addWidget(btnDefine);
    hb->addStretch();
    hb->addWidget(smxWidget);

    QGroupBox * bkgdViewGroup  = new QGroupBox("Background Image Location");
    bkgdViewGroup->setLayout(hb);
    bkgdViewGroup->setCheckable(true);
    bkgdViewGroup->setChecked(viewControl->isEnabled(VIEW_BKGD_IMG));

    connect(bkgdViewGroup, &QGroupBox::clicked,    this, &page_backgrounds::slot_showImageChanged);
    connect(btnNone,       &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_NONE;
                                                                                 displayBackgroundImageStatus(true);  emit sig_updateView();   } );
    connect(btnMosaic,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_MOSAIC;
                                                                                 displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnTiling,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_TILING;
                                                                                 displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnMaped,      &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_MAPED;
                                                                                 displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    connect(btnDefine,     &QRadioButton::toggled, this, [this] (bool checked) { if (checked) Sys::currentBkgImage = BKGD_IMAGE_DEFINED;
                                                                                 displayBackgroundImageStatus(true);  emit sig_updateView();  } );
    return bkgdViewGroup;
}

QGroupBox * page_backgrounds::createBackgroundImageGroup()
{
    QPushButton * loadBkgdBtn        = new QPushButton("Load Background");
                  startAdjustBtn     = new AQPushButton("Start Perspective Adjustment");
    QPushButton * completeAdjustBtn  = new QPushButton("Complete Perspective Adjustment");
    QPushButton * saveAdjustedBtn    = new QPushButton("Save Adjusted");
    QPushButton * clearBtn           = new QPushButton("Clear");
    QPushButton * resetBtn           = new QPushButton("Reset Xform");
    QPushButton * removeBkgdBtn      = new QPushButton("Remove Background");

    chk_useAdjusted    = new QCheckBox("Use Perspective");

    startAdjustBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");

    imageName        = new QLineEdit("Image name");

    QHBoxLayout * box0 = new QHBoxLayout();
    box0->addWidget(removeBkgdBtn);
    box0->addStretch();

    QHBoxLayout * box = new QHBoxLayout();
    box->addWidget(loadBkgdBtn);
    box->addWidget(imageName);
    box->addWidget(clearBtn);

    QHBoxLayout * box2 = new QHBoxLayout();
    box2->addWidget(chk_useAdjusted);
    box2->addWidget(startAdjustBtn);
    box2->addWidget(completeAdjustBtn);
    box2->addWidget(saveAdjustedBtn);
    box2->addStretch();

    QHBoxLayout * box3 = new QHBoxLayout();
    box3->addLayout(&bkgdLayout);
    box3->addWidget(resetBtn);

    QVBoxLayout * bkg = new QVBoxLayout();
    bkg->addLayout(box0);
    bkg->addLayout(box);
    bkg->addLayout(box3);
    bkg->addLayout(box2);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Image");
    bkgdGroup->setLayout(bkg);

    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_backgrounds::slot_loadBackground);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_backgrounds::slot_adjustBackground);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    &page_backgrounds::slot_saveAdjustedBackground);
    connect(clearBtn,          &QPushButton::clicked,         this,    &page_backgrounds::slot_clearBackground);
    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_backgrounds::slot_startSkewAdjustment);
    connect(resetBtn,          &QPushButton::clicked,         this,    &page_backgrounds::slot_resetXform);
    connect(removeBkgdBtn,     &QPushButton::clicked,         this,    &page_backgrounds::slot_removeBackground);
    connect(chk_useAdjusted,   &QCheckBox::clicked,           this,    &page_backgrounds::slot_useAdjustedClicked);
    connect(&bkgdLayout,       &LayoutTransform::xformChanged,this,    &page_backgrounds::slot_setBkgdXform);

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
        bkgdLayout.blockSignals(true);
        bkgdLayout.setTransform(xform);
        bkgdLayout.blockSignals(false);

        chk_useAdjusted->blockSignals(true);
        chk_useAdjusted->setChecked(bip->useAdjusted());
        chk_useAdjusted->blockSignals(false);

        imageName->setText(bip->getTitle());
    }
    else
    {
        imageName->setText("none");
        bkgdLayout.init();
        chk_useAdjusted->blockSignals(true);
        chk_useAdjusted->setChecked(false);
        chk_useAdjusted->blockSignals(false);
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
    if (rv)
    {
        QFileInfo info(filename);
        QString name = info.fileName();
        if (bip->load(name))
        {
            if (Sys::viewController->isEnabled(VIEW_TILING_MAKER) || Sys::viewController->isEnabled(VIEW_TILING))
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
            setupBackground(bkgdLayout.getXform());
            displayBackgroundImageStatus(true);
            panel->delegateView(VIEW_BKGD_IMG,true);     // since we loaded it, might as well see it
        }
    }
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

void page_backgrounds::slot_useAdjustedClicked(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    bip->setUseAdjusted(checked);
    setupBackground(bkgdLayout.getXform());
    emit sig_updateView();
}

void page_backgrounds::setupBackground(Xform xform)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;
    
    bip->setModelXform(xform,false,Sys::nextSigid());
    bip->createPixmap();
}

void page_backgrounds::slot_clearBackground ()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    bip->unload();
    displayBackgroundImageStatus(true);
    emit sig_reconstructView();
}

void page_backgrounds::slot_setBkgdXform()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;
    
    Xform xform = bip->getModelXform();
    xform.setTransform(bkgdLayout.getQTransform());  // TOD -what is  this. is it right?
    bip->setModelXform(xform,false,Sys::nextSigid());
    bip->createPixmap();
    emit sig_updateView();
}

void page_backgrounds::slot_startSkewAdjustment(bool checked)
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    Sys::guiModes->setMouseMode(MOUSE_MODE_NONE,true);    // ensure mouse gets here

    if (checked)
    {
        pageStatusString = "Click to select four points on background image. Then press 'Complete Perspective Adjustement' to fix camera skew.";
        setPageStatus();

        bip->setSkewMode(true);
    }
    else
    {
        clearPageStatus();
        bip->setSkewMode(false);    // also stops mouse interaction
    }
    emit sig_reconstructView();
}

void page_backgrounds::slot_resetXform()
{
    bkgdLayout.init();
    setupBackground(bkgdLayout.getXform());
    emit sig_updateView();
}

void page_backgrounds::slot_adjustBackground()
{
    auto bip = Sys::getBackgroundImageFromSource();
    if (!bip) return;

    if (!bip->getSkewMode())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Bkgd perspective");
        box.exec();
        return;
    }

    EdgeSet & saccum = bip->getAccum();
    if (saccum.size() != 4)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select four points to skew perspective");
        box.exec();
        return;
    }

    bip->createBackgroundAdjustment(saccum[0]->v1->pt, saccum[1]->v1->pt, saccum[2]->v1->pt, saccum[3]->v1->pt);

    bip->createPixmap();

    displayBackgroundImageStatus(true);

    bip->setSkewMode(false);

    clearPageStatus();

    startAdjustBtn->setChecked(false);

    emit sig_reconstructView();
}

void page_backgrounds::slot_saveAdjustedBackground()
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
    bool rv = bip->saveAdjusted(newName);

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
