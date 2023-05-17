#include "widgets/crop_widget.h"
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
    blocked = false;
    setLayout(createLayout());
}

QLayout * CropWidget::createLayout()
{
    QVBoxLayout * vb = new QVBoxLayout;

    // line 0
    QRadioButton * undefinedBtn = new QRadioButton("No Crop");
    vb->addWidget(undefinedBtn);

    // line 1
    QRadioButton * rectBtn = new QRadioButton("Rectangular Crop");
    vb->addWidget(rectBtn);

    // line 2
    cropRectLayout = new LayoutQRectF("Rectangle (model units) :", 8, 0.01);

    QHBoxLayout * rectL = new QHBoxLayout;
    rectL->addSpacing(21);
    rectL->addLayout(cropRectLayout);
    vb->addLayout(rectL);

    // line3
    QHBoxLayout * rectAsp = new QHBoxLayout;
    rectAsp->addSpacing(21);
    rectAsp->addLayout(createAspectLayout());
    vb->addLayout(rectAsp);

    // line4
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
    vb->addLayout(hbCirc);

    // line 5
    QRadioButton * regBtn  = new QRadioButton("Regular Crop");
    numSides = new SpinSet("Sides",0,0,64);

    QHBoxLayout * hbreg = new QHBoxLayout();
    hbreg->addWidget(regBtn);
    hbreg->addLayout(numSides);
    vb->addLayout(hbreg);

    // grouping
    cropTypes = new QButtonGroup;
    cropTypes->addButton(undefinedBtn,CROP_UNDEFINED);
    cropTypes->addButton(rectBtn,CROP_RECTANGLE);
    cropTypes->addButton(regBtn,CROP_POLYGON);
    cropTypes->addButton(circBtn,CROP_CIRCLE);

    connect(cropRectLayout, &LayoutQRectF::rectChanged,       this, &CropWidget::slot_rectChanged);
    connect(radius,         &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(centerX,        &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(centerY,        &DoubleSpinSet::valueChanged,     this, &CropWidget::slot_circleChanged);
    connect(numSides,       &SpinSet::valueChanged,           this, &CropWidget::slot_sidesChanged);
    connect(cropTypes,      &QButtonGroup::idClicked,         this, &CropWidget::slot_typeSelected);

    return vb;
}

QHBoxLayout * CropWidget::createAspectLayout()
{
    QLabel       *  label     = new QLabel("Rectangle Aspect Ratio :");
    QRadioButton *  rad_unc   = new QRadioButton("Unconstrained");
    QRadioButton *  rad_two   = new QRadioButton(QString("1 : %1 2").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_three = new QRadioButton(QString("1 : %1 3").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_four  = new QRadioButton(QString("1 : %1 4").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_five  = new QRadioButton(QString("1 : %1 5").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_six   = new QRadioButton(QString("1 : %1 6").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_seven = new QRadioButton(QString("1 : %1 7").arg(MathSymbolSquareRoot));
    chkVert                   = new QCheckBox("Vertical");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(label);
    hbox->addWidget(rad_unc);
    hbox->addWidget(rad_two);
    hbox->addWidget(rad_three);
    hbox->addWidget(rad_four);
    hbox->addWidget(rad_five);
    hbox->addWidget(rad_six);
    hbox->addWidget(rad_seven);
    hbox->addWidget(chkVert);
    hbox->addStretch();

    aspects = new QButtonGroup;
    aspects->addButton(rad_unc,  ASPECT_UNCONSTRAINED);
    aspects->addButton(rad_two,  ASPECT_SQRT_2);
    aspects->addButton(rad_three,ASPECT_SQRT_3);
    aspects->addButton(rad_four, ASPECT_SQRT_4);
    aspects->addButton(rad_five, ASPECT_SQRT_5);
    aspects->addButton(rad_six,  ASPECT_SQRT_6);
    aspects->addButton(rad_seven,ASPECT_SQRT_7);

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
    if (blocked || !crop) return;

    if (crop->getCropType() != CROP_CIRCLE)
    {
        return;
    }

    QPointF center(centerX->value(),centerY->value());

    auto c = crop->getCircle();
    c->setRadius(radius->value());
    c->setCenter(center);
    emit sig_cropModified();
}

void CropWidget::slot_sidesChanged(int n)
{
    if (blocked || !crop) return;

    if (crop->getCropType() != CROP_POLYGON)
    {
        return;
    }
    crop->setPolygon(numSides->value(),1.0,0);  // TODO crop scale/rpt
    emit sig_cropModified();
}

void CropWidget::slot_rectChanged()
{
    if (blocked || !crop) return;

    if (crop && crop->getCropType() == CROP_RECTANGLE )
    {
        QRectF rect = cropRectLayout->get();
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
        QRectF rect = cropRectLayout->get();
        crop->setRect(rect);
     }
        break;

    case CROP_POLYGON:
        crop->setPolygon(numSides->value(),1.0,0); // TODO scale/rot
        break;

    case CROP_CIRCLE:
        crop->setCircle(std::make_shared<Circle>(QPointF(centerX->value(),centerY->value()), radius->value()));
        break;

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
        cropRectLayout->set(crop->getRect());
        aspects->button(crop->getAspect())->setChecked(true);
        chkVert->setChecked(crop->getAspectVertical());
        auto circle = crop->getCircle();
        if (circle)
        {
            radius->setValue(crop->getCircle()->radius);
            centerX->setValue(crop->getCircle()->centre.x());
            centerY->setValue(crop->getCircle()->centre.y());
        }
        else
        {
            radius->setValue(5);
            centerX->setValue(0);
            centerX->setValue(0);
        }
        numSides->setValue(crop->getPolygon().size());
    }
    else
    {
        cropTypes->button(CROP_UNDEFINED)->setChecked(true);
        cropRectLayout->set(QRectF());
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

    QPushButton * pb = new QPushButton("Finish Editing");
    QHBoxLayout * hb = new QHBoxLayout;
    hb->addStretch();
    hb->addWidget(pb);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(cw);
    vbox->addLayout(hb);

    setLayout(vbox);

    setWindowTitle("Edit Crop");

    connect(pb,  &QPushButton::clicked, this, [this]() { emit sig_dlg_done(); done(QDialog::Accepted); } );
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