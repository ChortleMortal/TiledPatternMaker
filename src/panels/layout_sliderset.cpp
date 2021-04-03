/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/layout_sliderset.h"

//
// AQSpinBox
//

AQSpinBox::AQSpinBox() : QSpinBox()
{
    blocked = false;
    setKeyboardTracking(false);
}

void AQSpinBox::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    blocked = false;
    setStyleSheet("");
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
void  AQSpinBox::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    blocked = true;
    setStyleSheet("background-color:yellow;");
}
#else
void  AQSpinBox::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    blocked = true;
    setStyleSheet("background-color:yellow;");
}
#endif

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

AQDoubleSpinBox::AQDoubleSpinBox() : QDoubleSpinBox()
{
    blocked = false;
    setKeyboardTracking(false);
}

void  AQDoubleSpinBox::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    blocked = false;
    //qDebug() << "unblocked";
    setStyleSheet("");
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
void AQDoubleSpinBox::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    blocked = true;
    //qDebug() << "blocked";
    setStyleSheet("background-color:yellow;");
}
#else
void  AQDoubleSpinBox::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    blocked = true;
    //qDebug() << "blocked";
    setStyleSheet("background-color:yellow;");
}
#endif

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

SpinSet::SpinSet(QString txt, int val, int min, int max)
{
    label = new QLabel(txt);

    spin = new AQSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);
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

DoubleSpinSet::DoubleSpinSet(QString txt, qreal val, qreal min, qreal max)
{
    label = new QLabel(txt);

    spin = new AQDoubleSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);
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
    emit sig_valueChanged(val);
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

    emit valueChanged(val);
}

void DoubleSliderSet::sliderChanged(int val)
{
    qreal val2 = (qreal)val / (qreal)scale;

    spin->blockSignals(true);
    spin->setValue(val2);
    spin->blockSignals(false);

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





