#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include "panels/page_backgrounds.h"
#include "widgets/dlg_name.h"
#include "settings/configuration.h"
#include "panels/controlpanel.h"
#include "panels/panel_misc.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "viewers/backgroundimageview.h"
#include "viewers/viewcontrol.h"

page_backgrounds::page_backgrounds(ControlPanel * apanel) : panel_page(apanel,"Backgrounds"),  bkgdLayout("Bkgd Xform")
{
    bip = BackgroundImageView::getInstance();

    bkgdImageGroup = createBackgroundImageGroup();
    bkgdColorGroup = createBackgroundColorGroup();

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
    bkgdImageGroup->setChecked(config->showBackgroundImage);
    displayBackgroundStatus(false);

    for (int i = 0; i < viewTable->rowCount(); i++)
    {
        QTableWidgetItem * item = viewTable->item(i,1);
        QColor color = view->getViewSettings().getBkgdColor(static_cast<eViewType>(i));
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

    startAdjustBtn->setStyleSheet("QPushButton::checked { background-color: yellow; color: red;}");

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

    QGroupBox * bkgdGroup  = new QGroupBox("Show Background Image");
    bkgdGroup->setCheckable(true);
    bkgdGroup->setLayout(bkg);
    bkgdGroup->setChecked(config->showBackgroundImage);

    connect(loadBkgdBtn,       &QPushButton::clicked,         this,    &page_backgrounds::slot_loadBackground);
    connect(completeAdjustBtn, &QPushButton::clicked,         this,    &page_backgrounds::slot_adjustBackground);
    connect(saveAdjustedBtn,   &QPushButton::clicked,         this,    &page_backgrounds::slot_saveAdjustedBackground);
    connect(clearBtn,          &QPushButton::clicked,         this,    &page_backgrounds::slot_clearBackground);
    connect(startAdjustBtn,    &QPushButton::clicked,         this,    &page_backgrounds::slot_startSkewAdjustment);
    connect(resetBtn,          &QPushButton::clicked,         this,    &page_backgrounds::slot_resetXform);
    connect(chk_useAdjusted,   &QCheckBox::clicked,           this,    &page_backgrounds::slot_useAdjustedClicked);
    connect(&bkgdLayout,       &LayoutTransform::xformChanged,this,    &page_backgrounds::slot_setBkgdXform);
    connect(bkgdGroup,         &QGroupBox::clicked,           this,    &page_backgrounds::slot_showImageChanged);


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
    const QMap<eViewType,ViewData*> & fset = view->getViewSettings().getSettingsMap();

    viewTable->setRowCount(fset.size());

    QMap<eViewType,ViewData*>::const_iterator i = fset.constBegin();
    while (i != fset.constEnd())
    {
        eViewType type = i.key();
        const ViewData * s = i.value();
        ++i;

        QTableWidgetItem * item =  new QTableWidgetItem(s2ViewerType[type]);
        viewTable->setItem(row,0,item);

        QColor color = s->getBkgdColor();
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

    connect(pbResetB, &QPushButton::clicked, this, [this] { view->getViewSettings().reInitBkgdColors(QColor(Qt::black)); emit sig_refreshView(); } );
    connect(pbResetW, &QPushButton::clicked, this, [this] { view->getViewSettings().reInitBkgdColors(QColor(Qt::white)); emit sig_refreshView(); } );

    return bkgdGroup;
}

void page_backgrounds::displayBackgroundStatus(bool force)
{
    if (!refresh && !force)
    {
        return;
    }

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

void page_backgrounds::slot_useAdjustedClicked(bool checked)
{
    bip->setUseAdjusted(checked);
    setupBackground(bkgdLayout.getXform());
}

void page_backgrounds::setupBackground(Xform xform)
{
    bip->setCanvasXform(xform);
    bip->showPixmap();
}

void page_backgrounds::slot_clearBackground ()
{
    bip->unload();
    displayBackgroundStatus(true);
    emit sig_refreshView();
}

void page_backgrounds::slot_setBkgdXform()
{
    Xform xform = bip->getCanvasXform();
    xform.setTransform(bkgdLayout.getQTransform());
    bip->setCanvasXform(xform);
    bip->showPixmap();
    emit sig_refreshView();
}

void page_backgrounds::slot_startSkewAdjustment(bool checked)
{
    view->setMouseMode(MOUSE_MODE_NONE,true);    // ensure mouse gets here

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

void page_backgrounds::slot_saveAdjustedBackground()
{
    Xform xf = bip->getCanvasXform();

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
            bip->setCanvasXform(xf);
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

    QColor color = view->getViewSettings().getBkgdColor(vtype);

    AQColorDialog dlg(color,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        view->getViewSettings().setBkgdColor(vtype,color);
        emit sig_refreshView();
    }
}
