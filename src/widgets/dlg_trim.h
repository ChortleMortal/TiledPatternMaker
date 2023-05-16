#pragma once
#ifndef DLG_TRIM_H
#define DLG_TRIM_H

#include <QDialog>

class DlgTrim : public QDialog
{
    Q_OBJECT

public:
    DlgTrim(QWidget * parent = nullptr);

protected:

signals:
    void  sig_apply(qreal valX, qreal valY);

private slots:
    void slot_ok();
    void slot_apply();

private:
    class DoubleSpinSet * trimmerX;
    class DoubleSpinSet * trimmerY;
};

#endif
