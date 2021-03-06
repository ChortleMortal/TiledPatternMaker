#ifndef LAYOUT_QRECTF_H
#define LAYOUT_QRECTF_H

#include <QtWidgets>

#include "panels/layout_sliderset.h"

class LayoutQRectF : public QHBoxLayout
{
    Q_OBJECT

public:
    LayoutQRectF(QString name, int decimals = 8);
    void init();

    void setWidth(qreal val) { width->setValue(val); }
    void setHeight(qreal val){ height->setValue(val); }
    void setX(qreal val)     { X->setValue(val); }
    void setY(qreal val)     { Y->setValue(val); }

    void bumpScale(qreal val) { width->setValue(getScale() + (val * width->getSingleStep())); emit width->sig_valueChanged(0); }
    void bumpRot(qreal val)   { height->setValue(getRot() + (val * height->getSingleStep()));  emit height->sig_valueChanged(0); }
    void bumpX(qreal val)     { X->setValue(getX() + (val * X->getSingleStep())); emit X->sig_valueChanged(0);}
    void bumpY(qreal val)     { Y->setValue(getY() + (val * Y->getSingleStep()));  emit Y->sig_valueChanged(0); }

    qreal getScale() { return width->value(); }
    qreal getRot()   { return height->value(); }
    qreal getX()     { return X->value(); }
    qreal getY()     { return Y->value(); }
    QString getName(){ return name;}

    void   set(QRectF r);
    QRectF get();

signals:
    void    boundaryChanged();

protected:
    DoubleSpinSet * width;
    DoubleSpinSet * height;
    DoubleSpinSet * X;
    DoubleSpinSet * Y;
    QString         name;
};

#endif // LAYOUT_TRANSFORM_H
