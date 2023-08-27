#include "widgets/crop_widget.h"
#include "viewers/crop_view.h"
#include "widgets/layout_qrectf.h"
#include "widgets/layout_sliderset.h"
#include "enums/eborder.h"
#include "geometry/crop.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

CropWidget::CropWidget()
{
    cropViewer = CropViewer::getInstance();
    blocked    = false;
    setLayout(createLayout());
}

QLayout * CropWidget::createLayout()
{
    QRadioButton * undefinedBtn = new QRadioButton("No Crop");
    QRadioButton * rectBtn      = new QRadioButton("Rectangular Crop");
    rectLayoutW                 = new LayoutQRectF("Model units:", 8, 0.01);
    rectLayoutS                 = new LayoutQRectF("ScreenUnits:", 8,1.0);

    QHBoxLayout * rectW = new QHBoxLayout;
    rectW->addSpacing(21);
    rectW->addLayout(rectLayoutW);

    QHBoxLayout * rectS = new QHBoxLayout;
    rectS->addSpacing(21);
    rectS->addLayout(rectLayoutS);

    QHBoxLayout * rectAsp = new QHBoxLayout;
    rectAsp->addSpacing(21);
    rectAsp->addLayout(createAspectLayout());

    QRadioButton * circBtn = new QRadioButton("Circular Crop");

    radius = new DoubleSpinSet("Radius",0,0.0,20.0);
    radius->setPrecision(16);
    radius->setReadOnly(true);

    centerX = new DoubleSpinSet("Center X",0,-4096.0,4096.0);
    centerX->setPrecision(16);
    centerX->setReadOnly(true);

    centerY = new DoubleSpinSet("Y",0,-2160.0,2160.0);
    centerY->setPrecision(16);
    centerY->setSingleStep(0.05);
    centerY->setReadOnly(true);

    QHBoxLayout * hbCirc = new QHBoxLayout();
    hbCirc->addWidget(circBtn);
    hbCirc->addLayout(radius);
    hbCirc->addLayout(centerX);
    hbCirc->addLayout(centerY);
    hbCirc->addStretch();

    // line 5
    QRadioButton * regBtn  = new QRadioButton("Regular Crop");
    numSides = new SpinSet("Sides",0,0,64);

    QHBoxLayout * hbreg = new QHBoxLayout();
    hbreg->addWidget(regBtn);
    hbreg->addLayout(numSides);

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addWidget(undefinedBtn);
    vb->addWidget(rectBtn);
    vb->addLayout(rectW);
    vb->addLayout(rectS);
    vb->addLayout(rectAsp);
    vb->addLayout(hbCirc);
    vb->addLayout(hbreg);

    // grouping
    cropTypes = new QButtonGroup;
    cropTypes->addButton(undefinedBtn,CROP_UNDEFINED);
    cropTypes->addButton(rectBtn,CROP_RECTANGLE);
    cropTypes->addButton(regBtn,CROP_POLYGON);
    cropTypes->addButton(circBtn,CROP_CIRCLE);

    connect(rectLayoutW,    &LayoutQRectF::rectChanged,       this, &CropWidget::slot_rectChangedW);
    connect(rectLayoutS,    &LayoutQRectF::rectChanged,       this, &CropWidget::slot_rectChangedS);
    connect(radius,         &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(centerX,        &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(centerY,        &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(numSides,       &SpinSet::valueChanged,           this, &CropWidget::slot_sidesChanged);
    connect(cropTypes,      &QButtonGroup::idClicked,         this, &CropWidget::slot_typeSelected);

    return vb;
}

QHBoxLayout * CropWidget::createAspectLayout()
{
    QLabel       *  label     = new QLabel("Aspect Ratio :");
    QRadioButton *  rad_unc   = new QRadioButton("Free");
    QRadioButton *  rad_one   = new QRadioButton(QString("1:1"));
    QString str(3,'1');
    str[1] = MathSymbolSquareRoot;
    str[2] = '2' ;
    QRadioButton *  rad_two   = new QRadioButton(str);
    str[2] = '3' ;
    QRadioButton *  rad_three = new QRadioButton(str);
    str[2] = '4' ;
    QRadioButton *  rad_four  = new QRadioButton(str);
    str[2] = '5' ;
    QRadioButton *  rad_five  = new QRadioButton(str);
    str[2] = '6' ;
    QRadioButton *  rad_six   = new QRadioButton(str);
    str[2] = '7' ;
    QRadioButton *  rad_seven = new QRadioButton(str);
    QRadioButton *  rad_sd    = new QRadioButton(QString("4:3 SD"));
    QRadioButton *  rad_hd    = new QRadioButton(QString("16:9 HD"));
    chkVert                   = new QCheckBox("Vertical");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(label);
    hbox->addWidget(rad_unc);
    hbox->addWidget(rad_one);
    hbox->addWidget(rad_two);
    hbox->addWidget(rad_three);
    hbox->addWidget(rad_four);
    hbox->addWidget(rad_five);
    hbox->addWidget(rad_six);
    hbox->addWidget(rad_seven);
    hbox->addWidget(rad_sd);
    hbox->addWidget(rad_hd);
    hbox->addWidget(chkVert);
    hbox->addStretch();

    aspects = new QButtonGroup;
    aspects->addButton(rad_unc,  ASPECT_UNCONSTRAINED);
    aspects->addButton(rad_one,  ASPECT_SQUARE);
    aspects->addButton(rad_two,  ASPECT_SQRT_2);
    aspects->addButton(rad_three,ASPECT_SQRT_3);
    aspects->addButton(rad_four, ASPECT_SQRT_4);
    aspects->addButton(rad_five, ASPECT_SQRT_5);
    aspects->addButton(rad_six,  ASPECT_SQRT_6);
    aspects->addButton(rad_seven,ASPECT_SQRT_7);
    aspects->addButton(rad_sd,   ASPECT_SD);
    aspects->addButton(rad_hd,   ASPECT_HD);

    connect(aspects, &QButtonGroup::idClicked, this, &CropWidget::slot_cropAspect);
    connect(chkVert, &QCheckBox::toggled,      this, &CropWidget::slot_verticalAspect);

    return hbox;
}

void CropWidget::slot_cropAspect(int id)
{
    if (blocked || !crop) return;

    qDebug() <<  "aspect ratio =" << id;
    crop->setAspect(eAspectRatio(id));
    emit sig_cropModified();
}

void CropWidget::slot_verticalAspect(bool checked)
{
    if (blocked || !crop) return;

    crop->setAspectVertical(checked);
    emit sig_cropModified();
}

void CropWidget::slot_circleChanged(qreal r)
{
    Q_UNUSED(r);

    if (blocked || !crop) return;

    if (crop->getCropType() != CROP_CIRCLE)
    {
        return;
    }

    QPointF center(centerX->value(),centerY->value());

    Circle & c = crop->getCircle();
    c.setRadius(radius->value());
    c.setCenter(center);
    emit sig_cropModified();
}

void CropWidget::slot_sidesChanged(int n)
{
    Q_UNUSED(n);

    if (blocked || !crop) return;

    if (crop->getCropType() != CROP_POLYGON)
    {
        return;
    }
    crop->setPolygon(numSides->value(),1.0,0);  // TODO crop scale/rpt
    emit sig_cropModified();
}

void CropWidget::slot_rectChangedW()
{
    if (blocked || !crop) return;

    if (crop && crop->getCropType() == CROP_RECTANGLE )
    {
        QRectF rect = rectLayoutW->get();
        crop->setRect(rect);
    }
    emit sig_cropModified();
}

void CropWidget::slot_rectChangedS()
{
    if (blocked || !crop) return;

    if (crop && crop->getCropType() == CROP_RECTANGLE )
    {
        QRectF rect = rectLayoutS->get();
        rect = cropViewer->screenToWorld(rect);
        crop->setRect(rect);
    }
    emit sig_cropModified();
}

void CropWidget::slot_typeSelected(int id)
{
    if (blocked || !crop) return;

    eCropType type = static_cast<eCropType>(id);
    switch(type)
    {
    case CROP_RECTANGLE:
    {
        crop->setAspect(eAspectRatio(aspects->checkedId()));
        crop->setAspectVertical(chkVert->isChecked());
        QRectF rect = rectLayoutW->get();
        crop->setRect(rect);
     }
        break;

    case CROP_POLYGON:
        crop->setPolygon(numSides->value(),1.0,0); // TODO scale/rot
        break;

    case CROP_CIRCLE:
    {
        Circle ac(QPointF(centerX->value(),centerY->value()), radius->value());
        crop->setCircle(ac);
    }   break;

    case CROP_UNDEFINED:
        break;
    }
    emit sig_cropChanged();
}

void CropWidget::refresh()
{
    blocked = true;

    if (crop)
    {
        cropTypes->button(crop->getCropType())->setChecked(true);
        rectLayoutW->set(crop->getRect());
        rectLayoutS->set(cropViewer->worldToScreen(crop->getRect()));
        aspects->button(crop->getAspect())->setChecked(true);
        chkVert->setChecked(crop->getAspectVertical());
        Circle & circle = crop->getCircle();
        radius->setValue(circle.radius);
        centerX->setValue(circle.centre.x());
        centerY->setValue(circle.centre.y());
        numSides->setValue(crop->getPolygon().size());
    }
    else
    {
        cropTypes->button(CROP_UNDEFINED)->setChecked(true);
        rectLayoutW->set(QRectF());
        rectLayoutS->set(QRectF());
        aspects->button(ASPECT_UNCONSTRAINED)->setChecked(true);
        chkVert->setChecked(false);
        radius->setValue(5);
        centerX->setValue(0);
        centerX->setValue(0);
        numSides->setValue(6);
    }
    blocked = false;
}

////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////

CropDlg::CropDlg(CropPtr crop)
{
    Q_ASSERT(crop);

    this->crop = crop;

    setAttribute(Qt::WA_DeleteOnClose);

    cw = new CropWidget();
    cw->setCrop(crop);
    cw->refresh();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(cw);

    setLayout(vbox);

    setWindowTitle("Crop Geometry");
}

void CropDlg::setCrop(CropPtr crop)
{
    this->crop = crop;
    cw->setCrop(crop);
    cw->refresh();
}

void CropDlg::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    emit sig_dlg_done();
}
