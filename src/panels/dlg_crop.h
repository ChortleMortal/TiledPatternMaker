#ifndef DLGCROP_H
#define DLGCROP_H

#include <QDialog>
#include "makers/map_editor/map_editor.h"

class DlgCrop : public QDialog
{
    Q_OBJECT
public:
    DlgCrop(MapEditor * me, QWidget * parent = nullptr);

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void    slot_timeout();
    void    slot_setCrop(qreal);

private:
    MapEditor * mapeditor;

    QDoubleSpinBox * width;
    QDoubleSpinBox * height;
    QDoubleSpinBox * startX;
    QDoubleSpinBox * startY;

    QDoubleSpinBox * widthS;
    QDoubleSpinBox * heightS;
    QDoubleSpinBox * startXS;
    QDoubleSpinBox * startYS;

    bool entered;
};

#endif // DLGCROP_H
