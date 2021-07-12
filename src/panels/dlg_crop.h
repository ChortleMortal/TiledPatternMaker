#ifndef DLGCROP_H
#define DLGCROP_H

#include <QDialog>

class AQDoubleSpinBox;

typedef std::shared_ptr<class MapEditor>   MapEditorPtr;

class DlgCrop : public QDialog
{
    Q_OBJECT
public:
    DlgCrop(MapEditorPtr me, QWidget * parent = nullptr);
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
    MapEditorPtr mapeditor;

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
