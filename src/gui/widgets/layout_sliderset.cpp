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
    setFocusPolicy(Qt::StrongFocus);
}

void AQSpinBox::focusInEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = true;

        if (Sys::isDarkTheme)
            setStyleSheet("background-color: #22b895");
        else
            setStyleSheet("background-color:yellow");
    }

    QSpinBox::focusInEvent(event);
}

void AQSpinBox::focusOutEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = false;
        setStyleSheet("");
    }

    QSpinBox::focusOutEvent(event);
}

void AQSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
    {
        event->ignore();
        return;
    }

    QSpinBox::wheelEvent(event);
}

void AQSpinBox::setValue(int val)
{
    if  (blocked) return;

    blockSignals(true);
    QSpinBox::setValue(val);
    blockSignals(false);
}

//
// FullPrecisionDoubleSpinBox
//

FullPrecisionDoubleSpinBox::FullPrecisionDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    displayDecimals = 8;
    setDecimals(16);    // stored precision; display uses displayDecimals
}

void FullPrecisionDoubleSpinBox::setDisplayDecimals(int val)
{
    displayDecimals = val;
    setDecimals(decimals());    // refreshes text and size hint
}

QString FullPrecisionDoubleSpinBox::textFromValue(double val) const
{
    QString str = locale().toString(val, 'f', displayDecimals);
    if (!isGroupSeparatorShown())
        str.remove(locale().groupSeparator());
    return str;
}

double FullPrecisionDoubleSpinBox::valueFromText(const QString & text) const
{
    // unedited display text maps back to the full-precision value
    if (text == textFromValue(value()))
        return value();
    return QDoubleSpinBox::valueFromText(text);
}

//
// AQDoubleSpinBox
//

AQDoubleSpinBox::AQDoubleSpinBox(QWidget *parent) : FullPrecisionDoubleSpinBox(parent)
{
    blocked = false;
    setAlignment(Qt::AlignRight);
    setKeyboardTracking(false);
    setFocusPolicy(Qt::StrongFocus);
}

void AQDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = true;

        if (Sys::isDarkTheme)
            setStyleSheet("background-color: #22b895");
        else
            setStyleSheet("background-color:yellow");
    }

    FullPrecisionDoubleSpinBox::focusInEvent(event);
}

void AQDoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = false;
        setStyleSheet("");
    }

    FullPrecisionDoubleSpinBox::focusOutEvent(event);
}

void AQDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
    {
        event->ignore();
        return;
    }

    FullPrecisionDoubleSpinBox::wheelEvent(event);
}

void AQDoubleSpinBox::setValue(double val)
{
    if  (blocked) return;

    blockSignals(true);
    FullPrecisionDoubleSpinBox::setValue(val);
    blockSignals(false);
}

//
// SpinSet
//

SpinSet::SpinSet(QString txt, int val, int min, int max, bool nostretch, bool labelRight, bool resetToDefaults)
{
    if (resetToDefaults)
    {
        setAlignment(Qt::Alignment());   // remove forced alignment
        setSpacing(-1);                  // restore default spacing
        setContentsMargins(-1, -1, -1, -1);
    }

    label = new QLabel(txt);

    spin = new AQSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    if (labelRight)
    {
        addWidget(spin);
        addSpacing(4);
        addWidget(label);
    }
    else
    {
        addWidget(label);
        addSpacing(4);
        addWidget(spin);
    }

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
    spin->setDisplayDecimals(val);
}





