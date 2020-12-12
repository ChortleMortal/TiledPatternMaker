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

#ifndef SLIDERSET_H
#define SLIDERSET_H

#include <QtWidgets>

class SliderSet : public QHBoxLayout
{
    Q_OBJECT

public:
    SliderSet(QString txt, int val, int min, int max);
    void setValue(int val);
    void setRange(int min, int max);
    void setValues(int val, int min, int max);
    void reset() { setValue(defaultVal); }

    int  value() { return spin->value(); }

signals:
    void valueChanged(int val);

private slots:
    void spinChanged(int val);
    void sliderChanged(int val);

private:
    int              defaultVal;
    QLabel         * label;
    QSlider        * slider;
    QSpinBox       * spin;
};

class AQDoubleSpinBox : public QDoubleSpinBox
{
public:
    AQDoubleSpinBox();
    void  leaveEvent(QEvent *event) override;
    void  enterEvent(QEvent *event) override;

    bool blocked;
};

class DoubleSliderSet : public QHBoxLayout
{
    Q_OBJECT

public:
    DoubleSliderSet(QString txt, qreal val, qreal min, qreal max, int mult);
    void setValue(qreal val);
    void setRange(qreal min, qreal qreal);
    void setValues(qreal val, qreal min, qreal max);
    void reset() { setValue(defaultVal); }
    void setPrecision(int val);
    qreal value() { return spin->value(); }

signals:
    void valueChanged(qreal val);

private slots:
    void spinChanged(qreal val);
    void sliderChanged(int val);

private:
    int            scale;
    qreal          defaultVal;

    QLabel         * label;
    QSlider        * slider;
    QDoubleSpinBox * spin;
};

class AQSpinBox : public QSpinBox
{
public:
    AQSpinBox();
    void  leaveEvent(QEvent *event) override;
    void  enterEvent(QEvent *event) override;

    bool blocked;
};

class SpinSet: public QHBoxLayout
{
    Q_OBJECT

public:
    SpinSet(QString txt, int val, int min, int max);
    void    setValue(int val);
    void    setReadOnly(bool val)   { spin->setReadOnly(val); }
    int     value()                 { return spin->value(); }
    void    setFocus()              { spin->setFocus(); }
    void    hide()                  { label->hide(); spin->hide(); }
    void    show()                  { label->show(); spin->show(); }

signals:
   void valueChanged(int val);

private:
   QLabel           * label;
   AQSpinBox        * spin;
};

class DoubleSpinSet: public QHBoxLayout
{
    Q_OBJECT

public:
    DoubleSpinSet(QString txt, qreal val, qreal min, qreal max);
    void    setLabel(QString txt)    { label->setText(txt); }
    void    setValue(qreal val);
    qreal   value()                  { return spin->value(); }
    void    setDecimals(int val)     { spin->setDecimals(val); }
    void    setReadOnly(bool val)    { spin->setReadOnly(val); }
    void    setPrecision(int val)    { spin->setDecimals(val); }
    void    setSingleStep(qreal val) { spin->setSingleStep(val); }
    qreal   getSingleStep()          { return spin->singleStep(); }

signals:
    void sig_valueChanged(qreal val);

private slots:
    void slot_valueChanged(qreal val);

private:
   QLabel           * label;
   AQDoubleSpinBox  * spin;
};

#endif // SLIDERSET_H
