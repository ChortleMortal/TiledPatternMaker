#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QHeaderView>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/panels/page_backgrounds.h"
#include "gui/top/system_view.h"
#include "gui/widgets/memory_combo.h"
#include "gui/widgets/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/dlg_name.h"
#include "gui/widgets/floatable_tab.h"
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
    imageTab = createImageTab();
    colorTab = createColorTab();

    tabWidget = new QTabWidget;
    tabWidget->addTab(imageTab, "Background Image");
    tabWidget->addTab(colorTab, "Background Colors");

    vbox->addWidget(tabWidget);
    vbox->addStretch();

    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &page_backgrounds::slot_detach);

    QSettings s = Sys::settings();

    // detach in descending tab-index order so removing one tab doesn't shift the index of another not-yet-processed tab
    QString name = QString("panel2/%1/float_color_tab").arg(pageName);
    if (s.value(name,false).toBool())
        slot_detach(1);

    name = QString("panel2/%1/float_image_tab").arg(pageName);
    if (s.value(name,false).toBool())
        slot_detach(0);
}

page_backgrounds::~page_backgrounds()
{
    QSettings s = Sys::settings();

    QString name = QString("panel2/%1/float_image_tab").arg(pageName);
    s.setValue(name,imageTab->floating);
    if (imageTab->floating)
        imageTab->savePosition();
    imageTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_color_tab").arg(pageName);
    s.setValue(name,colorTab->floating);
    if (colorTab->floating)
        colorTab->savePosition();
    colorTab->floating = false;   // so it closes correctly
}

void page_backgrounds::slot_detach(int index)
{
    QWidget * widget = tabWidget->widget(index);
    QString title    = tabWidget->tabText(index);

    tabWidget->removeTab(index);

    FloatableTab * floater = dynamic_cast<FloatableTab*>(widget);
    Q_ASSERT(floater);
    floater->detach(tabWidget,title);
}

FloatableTab * page_backgrounds::createImageTab()
{
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(createBackgroundImageLayout());
    layout->addStretch();

    FloatableTab * tab = new FloatableTab(this);
    tab->setLayout(layout);
    return tab;
}

FloatableTab * page_backgrounds::createColorTab()
{
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(createBackgroundColorGroup());
    layout->addStretch();

    FloatableTab * tab = new FloatableTab(this);
    tab->setLayout(layout);
    return tab;
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
    chkView->setChecked(viewControl->isEnabled(Layer::Type::BkgdImage));

    displayBackgroundImageStatus(false);

    auto bip = Sys::backgroundImage;
    smxWidget->setLayer(bip.get());
    smxWidget->refresh();

    for (int i = 0; i < viewTable->rowCount(); i++)
    {
        Layer::Type vtype = static_cast<Layer::Type>(viewTable->item(i,0)->data(Qt::UserRole).toInt());
        QTableWidgetItem * item = viewTable->item(i,1);
        QColor color = config->getBkgdColor(vtype);
        item->setText(color.name());

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        viewTable->setCellWidget(i,2,label);
    }
}

QVBoxLayout * page_backgrounds::createBackgroundImageLayout()
{
    chkView         = new QCheckBox("View Background Image");
    chkConvertToPng = new QCheckBox("Convert to PNG");
    imageName       = new MemoryCombo("bkgdImageName");
    chkShowAdjusted = new QCheckBox("Show Adjusted");
    chkShowCropped  = new QCheckBox("Show Cropped");
    smxWidget       = new SMXWidget(nullptr,false,true);

    QPushButton * resetBtn      = new QPushButton("Reset Xform");
    QPushButton * selectBkgdBtn = new QPushButton("Select");
    QPushButton * loadBkgdBtn   = new QPushButton("Load");
    QPushButton * loadReplBtn   = new QPushButton("Load Replace");
    QPushButton * unloadBkgdBtn = new QPushButton("Unload");

    startAdjustBtn                      = new AQPushButton("Start Keystone Adjustment");
    QPushButton  * completeAdjustBtn    = new QPushButton("Complete Keystone Adustment");
    QPushButton  * saveAdjustedBtn      = new QPushButton("Save Adjusted");

    startCropBtn                        = new AQPushButton("Start Image Crop");
    QPushButton  * endCropBtn           = new QPushButton("Complete Image Crop");
    QPushButton  * saveCroppedBtn       = new QPushButton("Save Image Cropped");

    startAdjustBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");
    startCropBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");

    QHBoxLayout * hb1 = new QHBoxLayout();
    hb1->addWidget(selectBkgdBtn);
    hb1->addWidget(imageName);
    hb1->addWidget(loadBkgdBtn);
    hb1->addWidget(loadReplBtn);
    hb1->addWidget(chkConvertToPng);
    hb1->addWidget(unloadBkgdBtn);

    QHBoxLayout * hb0 = new QHBoxLayout();
    hb0->addWidget(chkView);
    hb0->addStretch();
    hb0->addWidget(smxWidget);

    QHBoxLayout * hb3 = new QHBoxLayout();
    hb3->addLayout(&tformLayout);
    hb3->addWidget(resetBtn);

    QHBoxLayout * hbAdjust = new QHBoxLayout();
    hbAdjust->addWidget(startAdjustBtn);
    hbAdjust->addWidget(completeAdjustBtn);
    hbAdjust->addWidget(saveAdjustedBtn);
    hbAdjust->addWidget(chkShowAdjusted);
    hbAdjust->addStretch();

    QGroupBox * adjustGroup = new QGroupBox("Keystone Adjustment");
    adjustGroup->setLayout(hbAdjust);

    QHBoxLayout * hbCrop = new QHBoxLayout();
    hbCrop->addWidget(startCropBtn);
    hbCrop->addWidget(endCropBtn);
    hbCrop->addWidget(saveCroppedBtn);
    hbCrop->addWidget(chkShowCropped);
    hbCrop->addStretch();

    QGroupBox * cropGroup = new QGroupBox("Image Cropping");
    cropGroup->setLayout(hbCrop);

    QVBoxLayout * vb = new QVBoxLayout();
    vb->addLayout(hb0);
    vb->addSpacing(10);
    vb->addLayout(hb1);
    vb->addLayout(hb3);
    vb->addSpacing(16);
    vb->addWidget(adjustGroup);
    vb->addSpacing(10);
    vb->addWidget(cropGroup);

    connect(chkView,       &QCheckBox::clicked,           this, &page_backgrounds::slot_showImageChanged);
    connect(resetBtn,      &QPushButton::clicked,         this, &page_backgrounds::slot_resetXform);
    connect(&tformLayout,  &LayoutTransform::xformChanged,this, &page_backgrounds::slot_setBkgdXform);

    connect(chkShowCropped,   &QCheckBox::clicked, this, [this] (bool checked) { useCropped(checked); emit sig_updateView(); });
    connect(chkShowAdjusted,  &QCheckBox::clicked, this, [this] (bool checked) { useAdjusted(checked);emit sig_updateView(); });

    connect(selectBkgdBtn,     &QPushButton::clicked,         this,    &page_backgrounds::slot_selectBackground);
    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_backgrounds::slot_loadBackground);
    connect(loadReplBtn,       &QPushButton::clicked,         this,    &page_backgrounds::slot_loadBackgroundReplace);
    connect(imageName,         &QComboBox::currentTextChanged,this,    &page_backgrounds::updateImageNameTally);
    connect(unloadBkgdBtn,     &QPushButton::clicked,         this,    &page_backgrounds::slot_unloadBackground);

    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_backgrounds::slot_startKeystoneAdjustment);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_backgrounds::slot_endKeystoneAdjust);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    [this] { saveModified(true); });

    connect(startCropBtn,      &QPushButton::clicked,         this,    &page_backgrounds::slot_startCrop);
    connect(endCropBtn,        &QPushButton::clicked,         this,    &page_backgrounds::slot_endCrop);
    connect(saveCroppedBtn,    &QPushButton::clicked,         this,    [this] { saveModified(false); });

    return vb;
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
        Layer::Type vtype = static_cast<Layer::Type>(i);
        if (!hasBkgdColor(vtype))
            continue;

        viewTable->setRowCount(row +1);

        QTableWidgetItem * item =  new QTableWidgetItem(sLayerType2[(int)vtype]);
        item->setData(Qt::UserRole,i);
        viewTable->setItem(row,0,item);

        QColor color = config->getBkgdColor(vtype);
        item = new QTableWidgetItem(color.name());
        viewTable->setItem(row,1,item);

        QLabel * label = new QLabel;
        QVariant variant= color;
        QString colcode = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        viewTable->setCellWidget(row,2,label);

        QPushButton * btn = new QPushButton("Select color");
        viewTable->setCellWidget(row,3,btn);
        connect(btn, &QPushButton::clicked, this, [this,vtype] { selectColor(vtype); });

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

    auto bip   = Sys::backgroundImage;
    bool loaded = bip && bip->isLoaded();
    if (loaded)
    {
        const Xform & xform = bip->getModelXform();
        tformLayout.blockSignals(true);
        tformLayout.setTransform(xform);
        tformLayout.blockSignals(false);

        // combo text is only synced on an explicit (force) refresh, never on a poll tick,
        // so a pending Select'd-but-not-yet-loaded name isn't clobbered out from under the user
        if (force && imageName->currentText() != bip->getTitle())
            imageName->setCurrentText(bip->getTitle());
        chkShowAdjusted->setChecked(bip->useAdjusted());
        chkShowCropped->setChecked(bip->useCropped());
    }
    else
    {
        // leave the combo showing whatever it already has (last-selected on startup, or a
        // pending Select) rather than stomping it with "None"
        tformLayout.init();
        chkShowAdjusted->setChecked(false);
        chkShowCropped->setChecked(false);
    }

    updateImageNameTally();
}

// green (with explicit readable text) when the shown name is actually loaded, otherwise the
// combo's normal theme style (readable in light and dark mode). Wired directly to the combo's
// own text-change signal (not just called from displayBackgroundImageStatus) so it updates the
// instant the user picks a different entry, rather than waiting on the hover-gated poll refresh.
void page_backgrounds::updateImageNameTally()
{
    auto bip    = Sys::backgroundImage;
    bool loaded = bip && bip->isLoaded();
    bool isCurrent = loaded && (imageName->currentText() == bip->getTitle());
    if (isCurrent)
        imageName->setStyleSheet("QComboBox, QComboBox QLineEdit { background-color: #66bb6a; color: black; }");
    else
        imageName->setStyleSheet(QString());
}

// picks a file, imports/converts it into the background photos dir, and records the name
// in the combo - does NOT load it as the active background image (see slot_loadBackground)
void page_backgrounds::slot_selectBackground()
{
    QString bkgdDir  = Sys::bkgdPhotosDir;
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

    if (chkConvertToPng->isChecked() &&
        (filename.endsWith(".jpg",Qt::CaseInsensitive) || filename.endsWith(".jpeg",Qt::CaseInsensitive)))
    {
        QString pngFilename = convertJpgToPng(filename);
        if (pngFilename.isEmpty())
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText(QString("Failed to convert image to PNG: %1").arg(filename));
            box.exec();
            return;
        }

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Loaded and re-saved as: %1").arg(pngFilename));
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();

        filename = pngFilename;
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
    imageName->setCurrentText(name);   // pending selection - press Load to actually activate it
}

void page_backgrounds::slot_loadBackground()
{
    loadBackground(false);
}

void page_backgrounds::slot_loadBackgroundReplace()
{
    loadBackground(true);
}

// loads the name currently shown in the combo (set by Select, or picked from its history)
// as the background image - replace keeps the current placement, otherwise it starts at unity
void page_backgrounds::loadBackground(bool replace)
{
    QString name = imageName->currentText();
    if (name.isEmpty() || name == "None")
        return;

    if (!QImageReader(Sys::bkgdPhotosDir + name).canRead())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Failed to load image: %1").arg(name));
        box.exec();
        return;
    }

    Sys::backgroundImage->load(name);

    if (!replace)
        tformLayout.init();

    setupBackgroundPixmap();
    panel->delegateView(Layer::Type::BkgdImage,true);     // since we loaded it, might as well see it
    displayBackgroundImageStatus(true);
}

QString page_backgrounds::convertJpgToPng(const QString & jpgPath)
{
    QImageReader reader(jpgPath);
    reader.setAutoTransform(true);     // applies EXIF orientation (e.g. camera-rotated JPEGs)

    QImage image = reader.read();
    if (image.isNull())
    {
        qWarning() << "convertJpgToPng: failed to read" << jpgPath << reader.errorString();
        return QString();
    }

    QFileInfo info(jpgPath);
    QString pngPath = info.absolutePath() + "/" + info.completeBaseName() + ".png";

    if (!image.save(pngPath,"PNG"))
    {
        qWarning() << "convertJpgToPng: failed to save" << pngPath;
        return QString();
    }

    qDebug() << "convertJpgToPng: converted" << jpgPath << "->" << pngPath;
    return pngPath;
}

void page_backgrounds::slot_unloadBackground()
{
    Sys::backgroundImage->unload();

    emit sig_reconstructView();

    displayBackgroundImageStatus(true);
}

void page_backgrounds::setupBackgroundPixmap()
{
    auto bip = Sys::backgroundImage;
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
    auto bip = Sys::backgroundImage;
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

        bip->adjuster.activate(true, bip.get());
        Sys::systemViewController->getView()->setCursor(Qt::CrossCursor);

        // the view is a separate top-level window from this panel, so a plain setFocus()
        // isn't enough - the window itself has to become the OS-active one first
        Sys::systemViewController->raiseView();        // for MacOS
        Sys::systemViewController->activateWindow();   // for Windows
        Sys::systemViewController->getView()->setFocus();
    }
    else
    {
        clearPageStatus();
        bip->adjuster.activate(false);    // also stops mouse interaction
        Sys::systemViewController->getView()->unsetCursor();
    }

    emit sig_updateView();
}

void page_backgrounds::slot_endKeystoneAdjust()
{
    auto bip = Sys::backgroundImage;
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

    bip->correctPerspective(saccum[0]->v1->pt, saccum[1]->v1->pt, saccum[2]->v1->pt, saccum[3]->v1->pt);

    bip->adjuster.activate(false);     // pops the model xform back to where it was on Start
    Sys::systemViewController->getView()->unsetCursor();

    // useAdjusted() -> setupBackgroundPixmap() re-applies tformLayout's xform to the layer,
    // so resync tformLayout to the just-restored xform first or it would clobber the restore
    // with whatever stale value tformLayout last cached
    tformLayout.blockSignals(true);
    tformLayout.setTransform(bip->getModelXform());
    tformLayout.blockSignals(false);

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
    auto bip = Sys::backgroundImage;
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
    {
        rv = bip->saveCropped(newName);
        xf.applyTranslate(bip->getCroppedOffset());     // reloaded image is centered on the origin
    }

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Image %1 saved").arg(newName));
        box.setInformativeText("Save the tiling or mosaic to keep it as the background.");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Image %1 save FAILED").arg(newName));
        box.setInformativeText("Try again with a new name");
    }
    box.exec();

    if (rv)
    {
        if (adjusted)
        {
            bip->adjuster.clear();     // the keystone rectangle stayed visible until now; the saved image supersedes it
        }

        // re-load
        bip->load(newName);
        if (bip->isLoaded())
        {
            bip->setModelXform(xf,false,Sys::nextSigid());
            bip->createPixmap();
            displayBackgroundImageStatus(true);
            panel->delegateView(Layer::Type::BkgdImage,true);
        }
    }
}

void page_backgrounds::useAdjusted(bool checked)
{
    auto bip = Sys::backgroundImage;
    if (!bip) return;

    if (checked)
    {
        startCropBtn->setChecked(false);
        chkShowCropped->setChecked(false);
        bip->setUseCropped(false);
    }

    chkShowAdjusted->setChecked(checked);
    bip->setUseAdjusted(checked);
    setupBackgroundPixmap();
}


void page_backgrounds::slot_startCrop(bool checked)
{
    auto bip = Sys::backgroundImage;
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
            Sys::systemViewController->getView()->unsetCursor();
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
    auto bip = Sys::backgroundImage;
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
    auto bip = Sys::backgroundImage;
    if (!bip) return;

    if (checked)
    {
        startAdjustBtn->setChecked(false);
        chkShowAdjusted->setChecked(false);
        bip->setUseAdjusted(false);
    }

    chkShowCropped->setChecked(checked);
    bip->setUseCropped(checked);
    setupBackgroundPixmap();
}

void page_backgrounds::slot_showImageChanged(bool checked)
{
    panel->userDelegateView(Layer::Type::BkgdImage,checked);
}

bool page_backgrounds::hasBkgdColor(Layer::Type vtype)
{
    // overlay layers do not set the view background
    return !(   vtype == Layer::Type::Grid
             || vtype == Layer::Type::Crop
             || vtype == Layer::Type::ConstructionLines);
}

void page_backgrounds::selectColor(Layer::Type vtype)
{
    QColor color = config->getBkgdColor(vtype);

    AQColorDialog dlg(color,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        if (vtype == Layer::Type::Mosaic)
            mosaicMaker->setBkgdColor(color);
        else
            config->setBkgdColor(vtype,color);
        emit sig_reconstructView();
    }
}

void page_backgrounds::reInitBkgdColors(QColor bcolor)
{
    for (int i = 0; i < NUM_VIEW_TYPES; i++)
    {
        Layer::Type vtype = static_cast<Layer::Type>(i);
        if (!hasBkgdColor(vtype))
            continue;
        if (vtype == Layer::Type::Mosaic)
            mosaicMaker->setBkgdColor(bcolor);
        else
            config->setBkgdColor(vtype,bcolor);
    }
    emit sig_reconstructView();
}
