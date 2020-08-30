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

// Slider Set

SliderSet::SliderSet(QString txt, int val, int min, int max)
{
    defaultVal = val;

    label = new QLabel(txt);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(min,max);
    slider->setValue(val);

    spin = new QSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

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

// Double slider set

DoubleSliderSet::DoubleSliderSet(QString txt, qreal val, qreal min, qreal max, int mult)
{
    scale      = mult;
    defaultVal = val;

    label = new QLabel(txt);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(min * scale, max * scale);
    slider->setValue(val * scale);

    spin = new QDoubleSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);
    spin->setSingleStep(1.0/mult);

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


// Spin set

SpinSet::SpinSet(QString txt, int val, int min, int max)
{
    label = new QLabel(txt);

    spin = new QSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);

    connect(spin,   SIGNAL(valueChanged(int)), this,   SIGNAL(valueChanged(int)));
}

void SpinSet::setValue(int val)
{
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);
}

DoubleSpinSet::DoubleSpinSet(QString txt, qreal val, qreal min, qreal max)
{
    label = new QLabel(txt);

    spin = new QDoubleSpinBox();
    spin->setRange(min,max);
    spin->setValue(val);

    addWidget(label);
    addSpacing(4);
    addWidget(spin);

    connect(spin,   SIGNAL(valueChanged(qreal)), this,   SIGNAL(valueChanged(qreal)));
}

void DoubleSpinSet::setValue(qreal val)
{
    spin->blockSignals(true);
    spin->setValue(val);
    spin->blockSignals(false);
}
