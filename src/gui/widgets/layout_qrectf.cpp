#include "gui/widgets/layout_qrectf.h"

LayoutQRectF::LayoutQRectF(QString name, int decimals, qreal singleStep)
{
    QLabel * label = new QLabel(name);
    width   = new DoubleSpinSet("Width",    0,  0,      3840);
    height  = new DoubleSpinSet("Height",   0,  0,      2160);
    X       = new DoubleSpinSet("X",        0,  -9000,  9000);
    Y       = new DoubleSpinSet("Y",        0,  -9000,  9000);

    width->setDecimals(decimals);
    width->setSingleStep(singleStep);
    height->setDecimals(decimals);
    height->setSingleStep(singleStep);
    X->setDecimals(decimals);
    X->setSingleStep(singleStep);
    Y->setDecimals(decimals);
    Y->setSingleStep(singleStep);

    addWidget(label);
    addSpacing(5);
    addLayout(X);
    addSpacing(9);
    addLayout(Y);
    addSpacing(9);
    addLayout(width);
    addSpacing(9);
    addLayout(height);
    //addStretch();

    init();

    connect(width,  &DoubleSpinSet::valueChanged,     this,    &LayoutQRectF::rectChanged);
    connect(height, &DoubleSpinSet::valueChanged,     this,    &LayoutQRectF::rectChanged);
    connect(X,      &DoubleSpinSet::valueChanged,     this,    &LayoutQRectF::rectChanged);
    connect(Y,      &DoubleSpinSet::valueChanged,     this,    &LayoutQRectF::rectChanged);
}

void LayoutQRectF::init()
{
    blockSignals(true);
    width->setValue(0.0);
    height->setValue(0.0);
    X->setValue(0.0);
    Y->setValue(0.0);
    blockSignals(false);
}

void LayoutQRectF::set(QRectF r)
{
    blockSignals(true);
    width->setValue(r.width());
    height->setValue(r.height());
    X->setValue(r.x());
    Y->setValue(r.y());
    blockSignals(false);
}

QRectF LayoutQRectF::get()
{
    QRectF rv(X->value(),Y->value(),width->value(),height->value());
    return rv;
}
////////////////////////////////////////////////////////////////

LayoutQPointF::LayoutQPointF(QString name, int decimals)
{
    QLabel * label = new QLabel(name);
    X       = new DoubleSpinSet("X",        0,  -9000,  9000);
    Y       = new DoubleSpinSet("Y",        0,  -9000,  9000);

    X->setDecimals(decimals);
    X->setSingleStep(1.0);
    Y->setDecimals(decimals);
    Y->setSingleStep(1.0);

    addWidget(label);
    addSpacing(5);
    addLayout(X);
    addSpacing(9);
    addLayout(Y);
    //addStretch();

    init();

    connect(X, &DoubleSpinSet::valueChanged,     this,    &LayoutQPointF::pointChanged);
    connect(Y, &DoubleSpinSet::valueChanged,     this,    &LayoutQPointF::pointChanged);
}

void LayoutQPointF::init()
{
    blockSignals(true);
    X->setValue(0.0);
    Y->setValue(0.0);
    blockSignals(false);
}

void LayoutQPointF::set(QPointF r)
{
    blockSignals(true);
    X->setValue(r.x());
    Y->setValue(r.y());
    blockSignals(false);
}

QPointF LayoutQPointF::get()
{
    QPointF rv(X->value(),Y->value());
    return rv;
}
