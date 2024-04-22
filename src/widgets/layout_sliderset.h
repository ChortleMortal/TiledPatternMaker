#pragma once
#ifndef SLIDERSET_H
#define SLIDERSET_H

#include <QSpinBox>
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>

class AQSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    AQSpinBox();

    void  leaveEvent(QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    void  enterEvent(QEnterEvent *event) override;
#else
    virtual void  enterEvent(QEvent *event) override;
#endif

    void setValue(int val);

    bool blocked;
};

class AQDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    AQDoubleSpinBox();

   void  leaveEvent(QEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    void  enterEvent(QEnterEvent *event) override;
#else
   virtual void  enterEvent(QEvent *event) override;
#endif

    void setValue(double val);

    bool blocked;
};

class SpinSet: public QHBoxLayout
{
    Q_OBJECT

public:
    SpinSet(QString txt, int val, int min, int max);
    void    setValue(int val);
    void    setReadOnly(bool val)   { spin->setReadOnly(val); }
    void    setDisabled(bool val)   { spin->setDisabled(val); }
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
    void    valueChanged(qreal val);

private slots:
    void    slot_valueChanged(qreal val);

private:
   QLabel           * label;
   AQDoubleSpinBox  * spin;
};


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
    AQSpinBox      * spin;
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
    AQDoubleSpinBox * spin;
};
#endif // SLIDERSET_H
