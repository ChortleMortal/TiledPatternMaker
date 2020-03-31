#include "layout_transform.h"
#include "geometry/Transform.h"

LayoutTransform::LayoutTransform(QString name)
{
    QLabel * label = new QLabel(name);
    scale   = new DoubleSpinSet("Scale",1.0,0.001,999);
    rot     = new DoubleSpinSet("Rot",0.0,-360.0,360.0);
    X       = new DoubleSpinSet("X",0.0,-9000,9000);
    Y       = new DoubleSpinSet("Y",0.0,-9000,9000);

    scale->setDecimals(8);
    scale->setSingleStep(0.01);
    rot->setDecimals(8);
    rot->setSingleStep(1.0);
    X->setDecimals(8);
    X->setSingleStep(1.0);
    Y->setDecimals(8);
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

    connect(scale,  &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(rot,    &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(X,      &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
    connect(Y,      &DoubleSpinSet::valueChanged,     this,    &LayoutTransform::xformChanged);
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
