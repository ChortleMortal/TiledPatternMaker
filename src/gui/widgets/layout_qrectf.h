#pragma once

#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/position_widget.h"

class LayoutQRectF : public QHBoxLayout
{
    Q_OBJECT

public:
    LayoutQRectF(QWidget *parent, QString name, int decimals = 8, qreal singleStep = 1.0);
    void init();

    void setWidth(qreal val) { width->setValue(val); }
    void setHeight(qreal val){ height->setValue(val); }
    void setX(qreal val)     { X->setValue(val); }
    void setY(qreal val)     { Y->setValue(val); }

    qreal getX()     { return X->value(); }
    qreal getY()     { return Y->value(); }
    QString getName(){ return name;}

    void   set(QRectF r);
    QRectF get();

signals:
    void    rectChanged();

private slots:
    void slot_adjust_pos();
    void slot_pos(Pos pos);

protected:
    DoubleSpinSet * width;
    DoubleSpinSet * height;
    DoubleSpinSet * X;
    DoubleSpinSet * Y;

private:
    QString         name;
    QWidget       * parent;
};

class LayoutQPointF : public QHBoxLayout
{
    Q_OBJECT

public:
    LayoutQPointF(QString name, int decimals = 8);
    void init();

    void setX(qreal val)     { X->setValue(val); }
    void setY(qreal val)     { Y->setValue(val); }

    qreal getX()     { return X->value(); }
    qreal getY()     { return Y->value(); }

    QString getName(){ return name;}

    void    set(QPointF r);
    QPointF get();

signals:
    void    pointChanged();

protected:
    DoubleSpinSet * X;
    DoubleSpinSet * Y;
    QString         name;
};
