#include "panels/page_background_image.h"
#include "panels/dlg_name.h"
#include "settings/configuration.h"
#include "tile/backgroundimage.h"
#include "panels/panel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

page_background_image::page_background_image(ControlPanel * apanel) : panel_page(apanel,"Background Image"),  bkgdLayout("Bkgd Xform")
{
    bip = BackgroundImage::getSharedInstance();

    bkgdGroup = createBackgroundGroup();
    vbox->addWidget(bkgdGroup);
}

void page_background_image::onEnter()
{
    displayBackgroundStatus(true);
}

void page_background_image::refreshPage()
{
    displayBackgroundStatus(false);
}

QGroupBox * page_background_image::createBackgroundGroup()
{
    QPushButton * loadBkgdBtn        = new QPushButton("Load Background");
                   startAdjustBtn    = new AQPushButton("Start Perspective Adjustment");
    QPushButton * completeAdjustBtn  = new QPushButton("Complete Perspective Adjustment");
    QPushButton * saveAdjustedBtn    = new QPushButton("Save Adjusted");
    QPushButton * clearBtn           = new QPushButton("Clear");

    chk_useAdjusted  = new QCheckBox("Use Perspective");
    imageName        = new QLineEdit("Image name");

    QHBoxLayout * box = new QHBoxLayout();
    box->addWidget(loadBkgdBtn);
    box->addWidget(imageName);
    box->addWidget(clearBtn);

    QHBoxLayout * box2 = new QHBoxLayout();
    box2->addWidget(startAdjustBtn);
    box2->addWidget(completeAdjustBtn);
    box2->addWidget(saveAdjustedBtn);
    box2->addStretch();
    box2->addWidget(chk_useAdjusted);

    QVBoxLayout * bkg = new QVBoxLayout();
    bkg->addLayout(box);
    bkg->addLayout(&bkgdLayout);
    bkg->addLayout(box2);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Image");
    bkgdGroup->setLayout(bkg);

    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_background_image::slot_loadBackground);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_background_image::slot_adjustBackground);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    &page_background_image::slot_saveAdjustedBackground);
    connect(clearBtn,          &QPushButton::clicked,         this,    &page_background_image::slot_clearBackground);
    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_background_image::slot_startSkewAdjustment);

    connect(&bkgdLayout,    &LayoutTransform::xformChanged,   this,    &page_background_image::slot_setBkgdXform);
    connect(chk_useAdjusted,&QAbstractButton::clicked,        this,    &page_background_image::slot_useAdjustedClicked);

    return bkgdGroup;
}

void page_background_image::displayBackgroundStatus(bool force)
{
    if (!refresh && !force)
    {
        return;
    }

    if (bip->isLoaded())
    {
        const Xform & xform = bip->getImageXform();
        bkgdLayout.blockSignals(true);
        bkgdLayout.setTransform(xform);
        bkgdLayout.blockSignals(false);

        chk_useAdjusted->blockSignals(true);
        chk_useAdjusted->setChecked(bip->useAdjusted());
        chk_useAdjusted->blockSignals(false);

        imageName->setText(bip->getName());
    }
    else
    {
        imageName->setText("none");
        bkgdLayout.init();
    }
}

void page_background_image::slot_loadBackground()
{
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    bool rv = bip->import(filename);
    if (rv)
    {
        QFileInfo info(filename);
        QString name = info.fileName();
        bip->load(name);
        if (bip->isLoaded())
        {
            config->showBackgroundImage = true;     // since we loaded it, might as well see it

            setupBackground();

            displayBackgroundStatus(true);

            emit sig_refreshView();
        }
    }
}

void page_background_image::slot_useAdjustedClicked(bool checked)
{
    bip->setUseAdjusted(checked);
    setupBackground();
}

void page_background_image::setupBackground()
{
    Xform xform = bip->getImageXform();
    xform.setTransform(bkgdLayout.getQTransform());
    bip->updateImageXform(xform);

    bip->createPixmap();
}

void page_background_image::slot_clearBackground ()
{
    bip->unload();

    emit sig_refreshView();
}

void page_background_image::slot_setBkgdXform()
{
    Xform xform = bip->getImageXform();
    xform.setTransform(bkgdLayout.getQTransform());
    bip->updateImageXform(xform);
    bip->createPixmap();
    emit sig_refreshView();
}

void page_background_image::slot_startSkewAdjustment(bool checked)
{
    if (checked)
    {
        QString txt = "Click to select four points on background image. Then press 'Adjust Perspective' to fix camera skew.";
        panel->showPanelStatus(txt);

        bip->setSkewMode(true);
    }
    else
    {
        panel->hidePanelStatus();
        bip->setSkewMode(false);    // also stops mouse interaction
    }
}

void page_background_image::slot_adjustBackground()
{
    if (!bip->getSkewMode())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Bkgd perspective");
        box.exec();
        return;
    }

    EdgePoly & saccum = bip->sAccum;
    if (saccum.size() != 4)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select four points to skew perspective");
        box.exec();
        return;
    }

    bip->createBackgroundAdjustment(
        saccum[0]->v1->pt,
        saccum[1]->v1->pt,
        saccum[2]->v1->pt,
        saccum[3]->v1->pt);

    bip->createPixmap();

    displayBackgroundStatus(true);

    bip->setSkewMode(false);

    panel->hidePanelStatus();

    startAdjustBtn->setChecked(false);

    emit sig_refreshView();
}

void page_background_image::slot_saveAdjustedBackground()
{
    QString oldname = bip->getName();

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
        box.setText("OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("FAILED");
    }
    box.exec();

    if (rv)
    {
        // re-load
        bip->load(newName);
        if (bip->isLoaded())
        {
            config->showBackgroundImage = true;     // since we loaded it, might as well see it

            displayBackgroundStatus(true);

            setupBackground();

            emit sig_refreshView();
        }
    }
}
