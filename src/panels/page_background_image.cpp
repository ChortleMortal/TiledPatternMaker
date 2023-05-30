#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include "panels/page_background_image.h"
#include "widgets/dlg_name.h"
#include "settings/configuration.h"
#include "panels/panel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "viewers/backgroundimageview.h"

page_background_image::page_background_image(ControlPanel * apanel) : panel_page(apanel,"Background Image"),  bkgdLayout("Bkgd Xform")
{
    bkgdGroup = createBackgroundGroup();
    vbox->addWidget(bkgdGroup);
}

void page_background_image::onEnter()
{
    displayBackgroundStatus(true);
}

void page_background_image::onRefresh()
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
    QPushButton * resetBtn           = new QPushButton("Reset Xform");

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

    QHBoxLayout * box3 = new QHBoxLayout();
    box3->addLayout(&bkgdLayout);
    box3->addWidget(resetBtn);

    QVBoxLayout * bkg = new QVBoxLayout();
    bkg->addLayout(box);
    bkg->addLayout(box3);
    bkg->addLayout(box2);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Image");
    bkgdGroup->setLayout(bkg);

    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_background_image::slot_loadBackground);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_background_image::slot_adjustBackground);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    &page_background_image::slot_saveAdjustedBackground);
    connect(clearBtn,          &QPushButton::clicked,         this,    &page_background_image::slot_clearBackground);
    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_background_image::slot_startSkewAdjustment);
    connect(resetBtn,          &QPushButton::clicked,         this,    &page_background_image::slot_resetXform);
    connect(chk_useAdjusted,   &QCheckBox::clicked,           this,    &page_background_image::slot_useAdjustedClicked);
    connect(&bkgdLayout,       &LayoutTransform::xformChanged,this,    &page_background_image::slot_setBkgdXform);

    return bkgdGroup;
}

void page_background_image::displayBackgroundStatus(bool force)
{
    if (!refresh && !force)
    {
        return;
    }
    
    auto bip = BackgroundImageView::getInstance();
    if (bip->isLoaded())
    {
        const Xform & xform = bip->getCanvasXform();
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
    
    auto bip = BackgroundImageView::getInstance();
    bool rv = bip->import(filename);
    if (rv)
    {
        QFileInfo info(filename);
        QString name = info.fileName();
        bip->load(name);
        if (bip->isLoaded())
        {
            config->showBackgroundImage = true;     // since we loaded it, might as well see it

            setupBackground(bkgdLayout.getXform());

            displayBackgroundStatus(true);

            emit sig_refreshView();
        }
    }
}

void page_background_image::slot_useAdjustedClicked(bool checked)
{
    auto bip = BackgroundImageView::getInstance();
    bip->setUseAdjusted(checked);
    setupBackground(bkgdLayout.getXform());
}

void page_background_image::setupBackground(Xform xform)
{
    auto bip = BackgroundImageView::getInstance();
    bip->setCanvasXform(xform);
    bip->showPixmap();
}

void page_background_image::slot_clearBackground ()
{
    auto bip = BackgroundImageView::getInstance();
    bip->unload();

    emit sig_refreshView();
}

void page_background_image::slot_setBkgdXform()
{
    auto bip = BackgroundImageView::getInstance();
    Xform xform = bip->getCanvasXform();
    xform.setTransform(bkgdLayout.getQTransform());
    bip->setCanvasXform(xform);
    bip->showPixmap();
    emit sig_refreshView();
}

void page_background_image::slot_startSkewAdjustment(bool checked)
{
    auto bip = BackgroundImageView::getInstance();

    if (checked)
    {
        QString txt = "Click to select four points on background image. Then press 'Complete Perspective Adjustement' to fix camera skew.";
        panel->pushPanelStatus(txt);

        bip->setSkewMode(true);
    }
    else
    {
        panel->popPanelStatus();
        bip->setSkewMode(false);    // also stops mouse interaction
    }
}

void page_background_image::slot_resetXform()
{
    bkgdLayout.init();
    setupBackground(bkgdLayout.getXform());
}

void page_background_image::slot_adjustBackground()
{
    auto bip = BackgroundImageView::getInstance();

    if (!bip->getSkewMode())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select Bkgd perspective");
        box.exec();
        return;
    }

    EdgePoly & saccum = bip->getAccum();
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

    bip->showPixmap();

    displayBackgroundStatus(true);

    bip->setSkewMode(false);

    panel->popPanelStatus();

    startAdjustBtn->setChecked(false);

    emit sig_refreshView();
}

void page_background_image::slot_saveAdjustedBackground()
{
    auto bip = BackgroundImageView::getInstance();

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
            config->showBackgroundImage = true;     // since we loaded it, might as well see it

            displayBackgroundStatus(true);

            setupBackground(bkgdLayout.getXform());

            emit sig_refreshView();
        }
    }
}
