#include "panels/layout_qrectf.h"

LayoutQRectF::LayoutQRectF(QString name, int decimals)
{
    QLabel * label = new QLabel(name);
    width   = new DoubleSpinSet("Width",    0,  0,      3840);
    height  = new DoubleSpinSet("Height",   0,  0,      2160);
    X       = new DoubleSpinSet("X",        0,  -9000,  9000);
    Y       = new DoubleSpinSet("Y",        0,  -9000,  9000);

    width->setDecimals(decimals);
    width->setSingleStep(1.0);
    height->setDecimals(decimals);
    height->setSingleStep(1.0);
    X->setDecimals(decimals);
    X->setSingleStep(1.0);
    Y->setDecimals(decimals);
    Y->setSingleStep(1.0);

    addWidget(label);
    addSpacing(5);
    addLayout(X);
    addSpacing(9);
    addLayout(Y);
    addSpacing(9);
    addLayout(width);
    addSpacing(9);
    addLayout(height);
    addStretch();

    init();

    connect(width,  &DoubleSpinSet::sig_valueChanged,     this,    &LayoutQRectF::boundaryChanged);
    connect(height, &DoubleSpinSet::sig_valueChanged,     this,    &LayoutQRectF::boundaryChanged);
    connect(X,      &DoubleSpinSet::sig_valueChanged,     this,    &LayoutQRectF::boundaryChanged);
    connect(Y,      &DoubleSpinSet::sig_valueChanged,     this,    &LayoutQRectF::boundaryChanged);
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
