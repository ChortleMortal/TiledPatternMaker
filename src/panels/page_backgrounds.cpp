#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include "panels/page_backgrounds.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/sys.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "panels/panel_misc.h"
#include "tile/backgroundimage.h"
#include "settings/configuration.h"
#include "viewers/backgroundimageview.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_name.h"

page_backgrounds::page_backgrounds(ControlPanel * apanel) : panel_page(apanel,PAGE_BKGD_MAKER,"Backgrounds"),  bkgdLayout("Bkgd Xform")
{
    bview = Sys::backgroundImageView;

    QGroupBox * bkgdImageGroup = createBackgroundImageGroup();
    QGroupBox * bkgdColorGroup = createBackgroundColorGroup();

    vbox->addWidget(bkgdImageGroup);
    vbox->addSpacing(13);
    vbox->addWidget(bkgdColorGroup);
    vbox->addStretch();
}

void page_backgrounds::onEnter()
{
    displayBackgroundStatus(true);
}

void page_backgrounds::onRefresh()
{
    chkShowBkgd->setChecked(config->showBackgroundImage);
    displayBackgroundStatus(false);

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
    QPushButton * loadBkgdBtn        = new QPushButton("Load Background");
                  startAdjustBtn     = new AQPushButton("Start Perspective Adjustment");
    QPushButton * completeAdjustBtn  = new QPushButton("Complete Perspective Adjustment");
    QPushButton * saveAdjustedBtn    = new QPushButton("Save Adjusted");
    QPushButton * clearBtn           = new QPushButton("Clear");
    QPushButton * resetBtn           = new QPushButton("Reset Xform");
    QPushButton * removeBkgdBtn      = new QPushButton("Remove Background");
                  chkShowBkgd        = new QCheckBox("Show Background Image");
                  chk_useAdjusted    = new QCheckBox("Use Perspective");

    chkShowBkgd->setChecked(config->showBackgroundImage);

    startAdjustBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");

    imageName        = new QLineEdit("Image name");

    QHBoxLayout * box0 = new QHBoxLayout();
    box0->addWidget(chkShowBkgd);
    box0->addSpacing(13);
    box0->addWidget(chk_useAdjusted);
    box0->addStretch();

    QHBoxLayout * box = new QHBoxLayout();
    box->addWidget(loadBkgdBtn);
    box->addWidget(imageName);
    box->addWidget(clearBtn);

    QHBoxLayout * box2 = new QHBoxLayout();
    box2->addWidget(startAdjustBtn);
    box2->addWidget(completeAdjustBtn);
    box2->addWidget(saveAdjustedBtn);
    box2->addStretch();
    box2->addWidget(removeBkgdBtn);

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
    connect(chkShowBkgd,       &QCheckBox::clicked,           this,    &page_backgrounds::slot_showImageChanged);

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

void page_backgrounds::displayBackgroundStatus(bool force)
{
    if (!refresh && !force)
    {
        return;
    }

    auto bip   = bview->getImage();
    if (bip && bip->isLoaded())
    {
        const Xform & xform = bview->getModelXform();
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
            // The owner of the image is the mosaic
            auto mosaic = mosaicMaker->getMosaic();
            Q_ASSERT(mosaic);
            mosaic->setBkgdImage(bip);

            // the view has a weak_ptr
            bview->setImage(bip);
            config->showBackgroundImage = true;     // since we loaded it, might as well see it
            setupBackground(bkgdLayout.getXform());
            displayBackgroundStatus(true);
            emit sig_refreshView();
        }
    }

}

void page_backgrounds::slot_removeBackground()
{
    auto mosaic = mosaicMaker->getMosaic();
    mosaic->removeBkgdImage();

    emit sig_refreshView();

    displayBackgroundStatus(true);
}

void page_backgrounds::slot_useAdjustedClicked(bool checked)
{
    auto bip = bview->getImage();
    if (!bip) return;

    bip->setUseAdjusted(checked);
    setupBackground(bkgdLayout.getXform());
    view->update();
}

void page_backgrounds::setupBackground(Xform xform)
{
    auto bip = bview->getImage();
    if (!bip) return;
    
    bview->setModelXform(xform,false);
    bip->createPixmap();
}

void page_backgrounds::slot_clearBackground ()
{
    auto bip = bview->getImage();
    if (!bip) return;

    bip->unload();
    displayBackgroundStatus(true);
    emit sig_refreshView();
}

void page_backgrounds::slot_setBkgdXform()
{
    auto bip = bview->getImage();
    if (!bip) return;
    
    Xform xform = bview->getModelXform();
    xform.setTransform(bkgdLayout.getQTransform());
    bview->setModelXform(xform,false);
    bip->createPixmap();
    view->update();
}

void page_backgrounds::slot_startSkewAdjustment(bool checked)
{
    view->setMouseMode(MOUSE_MODE_NONE,true);    // ensure mouse gets here

    if (checked)
    {
        QString txt = "Click to select four points on background image. Then press 'Complete Perspective Adjustement' to fix camera skew.";
        panel->setStatus(txt);

        bview->setSkewMode(true);
    }
    else
    {
        panel->clearStatus();
        bview->setSkewMode(false);    // also stops mouse interaction
        emit sig_refreshView();
    }
}

void page_backgrounds::slot_resetXform()
{
    bkgdLayout.init();
    setupBackground(bkgdLayout.getXform());
}

void page_backgrounds::slot_adjustBackground()
{
    auto bip = bview->getImage();
    if (!bip) return;

    if (!bview->getSkewMode())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Bkgd perspective");
        box.exec();
        return;
    }

    EdgePoly & saccum = bview->getAccum();
    if (saccum.size() != 4)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select four points to skew perspective");
        box.exec();
        return;
    }

    bview->createBackgroundAdjustment(bip, saccum[0]->v1->pt, saccum[1]->v1->pt, saccum[2]->v1->pt, saccum[3]->v1->pt);

    bip->createPixmap();

    displayBackgroundStatus(true);

    bview->setSkewMode(false);

    panel->clearStatus();

    startAdjustBtn->setChecked(false);

    emit sig_refreshView();
}

void page_backgrounds::slot_saveAdjustedBackground()
{
    auto bip = bview->getImage();
    if (!bip) return;
    
    Xform xf = bview->getModelXform();

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
            bview->setModelXform(xf,false);
            bip->setUseAdjusted(false);

            config->showBackgroundImage = true;     // since we loaded it, might as well see it

            displayBackgroundStatus(true);
            emit sig_refreshView();
        }
    }
}

void page_backgrounds::slot_showImageChanged(bool checked)
{
    config->showBackgroundImage = checked;
    emit sig_refreshView();
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
        emit sig_refreshView();
    }
}

void page_backgrounds::reInitBkgdColors(QColor bcolor)
{
    //setBkgdColor(VIEW_DESIGN,bcolor);
    viewControl->setBackgroundColor(VIEW_MOSAIC,bcolor);
    viewControl->setBackgroundColor(VIEW_PROTOTYPE,bcolor);
    viewControl->setBackgroundColor(VIEW_MOTIF_MAKER,bcolor);
    viewControl->setBackgroundColor(VIEW_TILING,bcolor);
    viewControl->setBackgroundColor(VIEW_TILING_MAKER,bcolor);
    viewControl->setBackgroundColor(VIEW_MAP_EDITOR,bcolor);
    viewControl->setBackgroundColor(VIEW_BKGD_IMG,bcolor);
    viewControl->setBackgroundColor(VIEW_GRID,bcolor);
    viewControl->setBackgroundColor(VIEW_BORDER,bcolor);
    viewControl->setBackgroundColor(VIEW_CROP,bcolor);
    //setBkgdColor(VIEW_MEASURE,bcolor);
    //setBkgdColor(VIEW_CENTER,bcolor);
    //setBkgdColor(VIEW_IMAGE,bcolor);
    emit sig_refreshView();
}
