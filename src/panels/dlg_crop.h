#ifndef DLGCROP_H
#define DLGCROP_H

#include <QDialog>
#include "panels/layout_sliderset.h"
#include "makers/map_editor/map_editor.h"

class DlgCrop : public QDialog
{
    Q_OBJECT
public:
    DlgCrop(MapEditor * me, QWidget * parent = nullptr);
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    virtual void  enterEvent(QEnterEvent *event) override;
#else
    virtual void  enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private slots:
    void    slot_timeout();
    void    slot_setCrop(qreal);

private:
    MapEditor * mapeditor;

    AQDoubleSpinBox * width;
    AQDoubleSpinBox * height;
    AQDoubleSpinBox * startX;
    AQDoubleSpinBox * startY;

    AQDoubleSpinBox * widthS;
    AQDoubleSpinBox * heightS;
    AQDoubleSpinBox * startXS;
    AQDoubleSpinBox * startYS;

    bool entered;
};

#endif // DLGCROP_H
