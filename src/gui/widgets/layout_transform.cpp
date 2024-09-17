#include "gui/widgets/layout_transform.h"
#include "sys/geometry/transform.h"

LayoutTransform::LayoutTransform(QString name, int decimals)
{
    QLabel * label = new QLabel(name);
    scale   = new DoubleSpinSet("Scale",1.0,0.001,999);
    rot     = new DoubleSpinSet("Rot",0.0,-360.0,360.0);
    X       = new DoubleSpinSet("X",0.0,-9000,9000);
    Y       = new DoubleSpinSet("Y",0.0,-9000,9000);

    scale->setDecimals(decimals);
    scale->setSingleStep(0.01);
    rot->setDecimals(decimals);
    rot->setSingleStep(1.0);
    X->setDecimals(decimals);
    X->setSingleStep(1.0);
    Y->setDecimals(decimals);
    Y->setSingleStep(1.0);

    addWidget(label);
    addSpacing(5);
    addLayout(scale);
    addSpacing(9);
    addLayout(rot);
    addSpacing(9);
    addLayout(X);
    addSpacing(9);
    addLayout(Y);
    addStretch();

    init();

    connect(scale,  &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(rot,    &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(X,      &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(Y,      &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
}

void LayoutTransform::init()
{
    blockSignals(true);
    scale->setValue(1.0);
    rot->setValue(0.0);
    X->setValue(0.0);
    Y->setValue(0.0);
    blockSignals(false);
}

void LayoutTransform::setTransform(QTransform T)
{
    blockSignals(true);
    scale->setValue(Transform::scalex(T));
    rot->setValue(Transform::rotation(T));
    X->setValue(Transform::transx(T));
    Y->setValue(Transform::transy(T));
    blockSignals(false);
}

void LayoutTransform::setTransform(const Xform & xf)
{
    blockSignals(true);
    scale->setValue(xf.getScale());
    rot->setValue(xf.getRotateDegrees());
    X->setValue(xf.getTranslateX());
    Y->setValue(xf.getTranslateY());
    blockSignals(false);
}

QTransform LayoutTransform::getQTransform()
{
    QTransform t0;
    t0 = t0.scale(scale->value(),scale->value());
    QTransform t1;
    t1.rotate(rot->value());
    QTransform t2;
    t2.translate(X->value(),Y->value());

    QTransform t = t0 * t1 * t2;
    return t;
}

Xform LayoutTransform::getXform()
{
    Xform xf;
    xf.setScale(scale->value());
    xf.setRotateDegrees(rot->value());
    xf.setTranslateX(X->value());
    xf.setTranslateY(Y->value());
    return xf;
}
