#include <QPalette>
#include "gui/widgets/layout_sliderset.h"
#include "sys/sys.h"

//
// AQSpinBox
//

AQSpinBox::AQSpinBox() : QSpinBox()
{
    blocked = false;
    setAlignment(Qt::AlignRight);
    setKeyboardTracking(false);

    QPalette palette             = this->palette();
    QPalette::ColorRole role    = backgroundRole();
    QColor backgroundColor      = palette.color(role);
    QString backgroundColorName = backgroundColor.name(QColor::HexRgb);
}

void AQSpinBox::leaveEvent(QEvent *event)
{
    if (isReadOnly()) return;
    blocked = false;

    QString styleSheet = QString("background-color: %1;").arg(backgroundColorName);
    setStyleSheet(styleSheet);

    QSpinBox::leaveEvent(event);
}

void  AQSpinBox::enterEvent(QEnterEvent *event)
{
    if (isReadOnly()) return;
    blocked = true;

    if (Sys::isDarkTheme)
        setStyleSheet("background-color: #22b895");
    else
        setStyleSheet("background-color:yellow");

    QSpinBox::enterEvent(event);
}

void AQSpinBox::setValue(int val)
{
    if  (blocked) return;

    blockSignals(true);
    QSpinBox::setValue(val);
    blockSignals(false);
}

//
// AQDoubleSpinBox
//

AQDoubleSpinBox::AQDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    blocked = false;
    setDecimals(8);
    setAlignment(Qt::AlignRight);
    setKeyboardTracking(false);

    QPalette palette             = this->palette();
    QPalette::ColorRole role    = backgroundRole();
    QColor backgroundColor      = palette.color(role);
    QString backgroundColorName = backgroundColor.name(QColor::HexRgb);
}

void  AQDoubleSpinBox::leaveEvent(QEvent *event)
{
    if (isReadOnly()) return;
    blocked = false;

    QString styleSheet = QString("background-color: %1;").arg(backgroundColorName);
    setStyleSheet(styleSheet);

    QDoubleSpinBox::leaveEvent(event);
}

void AQDoubleSpinBox::enterEvent(QEnterEvent *event)
{
    if (isReadOnly()) return;
    blocked = true;

    if (Sys::isDarkTheme)
        setStyleSheet("background-color: #22b895");
    else
        setStyleSheet("background-color:yellow");

    QDoubleSpinBox::enterEvent(event);
}

void AQDoubleSpinBox::setValue(double val)
{
    if  (blocked) return;

    blockSignals(true);
    QDoubleSpinBox::setValue(val);
    blockSignals(false);
}

//
// SpinSet
//

SpinSet::SpinSet(QString txt, int val, int min, int max, bool nostretch)
{
    label = new QLabel(txt);

    spin = new AQSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);
    if (nostretch == false)
        addStretch();

    connect(spin,   SIGNAL(valueChanged(int)), this,   SIGNAL(valueChanged(int)));
}

void SpinSet::setValue(int val)
{
    if  (spin->blocked) return;

    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);
}

//
// DoubleSpinSet
//

DoubleSpinSet::DoubleSpinSet(QString txt, qreal val, qreal min, qreal max, bool nostretch)
{
    label = new QLabel(txt);

    spin = new AQDoubleSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);
    if (nostretch == false)
        addStretch();

    connect(spin,   SIGNAL(valueChanged(qreal)), this,   SLOT(slot_valueChanged(qreal)));
}

void DoubleSpinSet::setValue(qreal val)
{
    if  (spin->blocked) return;

    //qDebug() << "value set" << val;
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);
}

void  DoubleSpinSet::slot_valueChanged(qreal val)
{
    //qDebug() << "value changed" << val;
    emit valueChanged(val);
}

//
// Slider Set
//

SliderSet::SliderSet(QString txt, int val, int min, int max)
{
    defaultVal = val;

    label = new QLabel(txt);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(min,max);
    slider->setValue(val);

    spin = new AQSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);
    spin->setKeyboardTracking(false);

    addWidget(label);
    addSpacing(7);
    addWidget(slider);
    addSpacing(7);
    addWidget(spin);

    connect(slider, SIGNAL(valueChanged(int)), this,  SLOT(sliderChanged(int)));
    connect(spin,   SIGNAL(valueChanged(int)), this,  SLOT(spinChanged(int)));
}

void SliderSet::setValue(int val)
{
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);

    slider->blockSignals(true);
    slider->setValue(val);
    slider->blockSignals(false);
}

void SliderSet::spinChanged(int val)
{
    slider->blockSignals(true);
    slider->setValue(val);
    slider->blockSignals(false);

    emit valueChanged(val);
}

void SliderSet::sliderChanged(int val)
{
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);

    emit valueChanged(val);
}

void SliderSet::setRange(int min, int max)
{
    spin->setRange(min,max);
    slider->setRange(min,max);
}

void SliderSet::setValues(int val, int min, int max)
{
    blockSignals(true);
    setRange(min,max);
    setValue(val);
    blockSignals(false);
}

//
// Double slider set
//

DoubleSliderSet::DoubleSliderSet(QString txt, qreal val, qreal min, qreal max, int mult)
{
    scale      = mult;
    defaultVal = val;

    label = new QLabel(txt);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(min * scale, max * scale);
    slider->setValue(val * scale);

    spin = new AQDoubleSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);
    spin->setSingleStep(1.0/mult);
    spin->setKeyboardTracking(false);

    addWidget(label);
    addSpacing(7);
    addWidget(slider);
    addSpacing(7);
    addWidget(spin);

    connect(slider, SIGNAL(valueChanged(int)),   this,  SLOT(sliderChanged(int)));
    connect(spin,   SIGNAL(valueChanged(qreal)), this,  SLOT(spinChanged(qreal)));
}

void DoubleSliderSet::setValues(qreal val, qreal min, qreal max)
{
    blockSignals(true);
    setRange(min,max);
    setValue(val);
    blockSignals(false);
}

void DoubleSliderSet::spinChanged(qreal val)
{
    slider->blockSignals(true);
    slider->setValue(val * scale);
    slider->blockSignals(false);

    //qDebug() << "spin - emit" << val;
    emit valueChanged(val);
}

void DoubleSliderSet::sliderChanged(int val)
{
    qreal val2 = (qreal)val / (qreal)scale;

    spin->blockSignals(true);
    spin->setValue(val2);
    spin->blockSignals(false);

    //qDebug() << "slider - emit" << val2;
    emit valueChanged(val2);
}

void DoubleSliderSet::setValue(qreal val)
{
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);

    slider->blockSignals(true);
    slider->setValue(val * scale);
    slider->blockSignals(false);
}

void DoubleSliderSet::setRange(qreal min, qreal max)
{
    spin->setRange(min,max);
    slider->setRange(min * scale, max * scale);
}

void DoubleSliderSet::setPrecision(int val)
{
    spin->setDecimals(val);
}





