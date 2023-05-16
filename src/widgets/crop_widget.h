#pragma once
#ifndef CROPWIDGET_H
#define CROPWIDGET_H

#include <QWidget>
#include <QDialog>

class QHBoxLayout;
class QButtonGroup;
class LayoutQRectF;
class SpinSet;
class DoubleSpinSet;
class QCheckBox;

typedef std::shared_ptr<class Crop> CropPtr;

class CropWidget : public QWidget
{
    Q_OBJECT

public:
    CropWidget();

    void setCrop(CropPtr crop) { this->crop = crop; }
    void refresh();

    QLayout     * createLayout();
    QHBoxLayout * vcreateAspectLayout();
    QHBoxLayout * createAspectLayout();

signals:
    void sig_cropChanged();
    void sig_cropModified();

private slots:
    void slot_cropAspect(int id);
    void slot_verticalAspect(bool checked);
    void slot_circleChanged(qreal r);
    void slot_sidesChanged(int n);
    void slot_typeSelected(int id);
    void slot_rectChanged();

private:
    CropPtr crop;

    bool    blocked;

    QButtonGroup    * cropTypes;
    QButtonGroup    * aspects;
    LayoutQRectF    * cropRectLayout;
    DoubleSpinSet   * radius;
    DoubleSpinSet   * centerX;
    DoubleSpinSet   * centerY;
    SpinSet         * numSides;
    QCheckBox       * chkVert;
};


class CropDlg : public QDialog
{
    Q_OBJECT

public:
    CropDlg(CropPtr crop);

    void setCrop(CropPtr crop);
    void refresh();
    void closeEvent(QCloseEvent * event) override;

    CropWidget * cw;

signals:
    void sig_dlg_done();

private:
    CropPtr crop;
};

#endif // CROPWIDGET_H
