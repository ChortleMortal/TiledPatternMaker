#pragma once
#ifndef LAYOUT_TRANSFORM_H
#define LAYOUT_TRANSFORM_H

#include "gui/widgets/layout_sliderset.h"
#include "sys/geometry/xform.h"

class LayoutTransform : public QHBoxLayout
{
    Q_OBJECT

public:
    LayoutTransform(QString name, int decimals = 8);
    void init();

    void setScale(qreal val) { scale->setValue(val); }
    void setRot(qreal val)   { rot->setValue(val); }
    void setX(qreal val)     { X->setValue(val); }
    void setY(qreal val)     { Y->setValue(val); }

    void bumpScale(qreal val) { scale->setValue(getScale() + (val * scale->getSingleStep())); emit scale->valueChanged(0); }
    void bumpRot(qreal val)   { rot->setValue(getRot() + (val * rot->getSingleStep()));  emit rot->valueChanged(0); }
    void bumpX(qreal val)     { X->setValue(getX() + (val * X->getSingleStep())); emit X->valueChanged(0);}
    void bumpY(qreal val)     { Y->setValue(getY() + (val * Y->getSingleStep()));  emit Y->valueChanged(0); }

    qreal getScale() { return scale->value(); }
    qreal getRot()   { return rot->value(); }
    qreal getX()     { return X->value(); }
    qreal getY()     { return Y->value(); }
    QString getName(){ return name;}

    void  setTransform(QTransform T);
    void  setTransform(const Xform &xf);

    QTransform getQTransform();
    Xform      getXform();

signals:
    void    xformChanged();

protected:
    DoubleSpinSet * scale;
    DoubleSpinSet * rot;
    DoubleSpinSet * X;
    DoubleSpinSet * Y;
    QString         name;
};

#endif // LAYOUT_TRANSFORM_H
