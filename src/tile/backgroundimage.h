#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include "base/layer.h"

class  BackgroundImage : public Layer
{
public:
    BackgroundImage();
    ~BackgroundImage() override;

    void    paint(QPainter *painter) override;

    bool    import(QString filename);  // loads from new file
    bool    load(QString imageName);   // loads from existing imported file

    void    bkgdImageChanged(bool showBkgd, bool perspectiveBkgd);

    void    adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    adjustBackground();

    bool    saveAdjusted(QString newName);

    bool    isLoaded() { return _loaded; }
    QString getName()  { return bkgdName; }

    void     updateBkgdXform(const Xform & xf);
    const Xform & getBkgdXform()  { return xf_bkImg; }

    // public data
    QTransform perspective;
    bool       bShowBkgd;
    bool       bAdjustPerspective;

public slots:
    virtual void slot_moveX(int amount) override;
    virtual void slot_moveY(int amount) override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_scale(int amount) override;

    virtual void slot_mouseTranslate(QPointF pt) override;
    virtual void slot_wheel_rotate(qreal delta) override;
    virtual void slot_wheel_scale(qreal delta) override;
    virtual void slot_setCenterScreen(QPointF spt) override;

protected:
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    drawCenter(QPainter * painter) override;

private:
    View          * view;
    Configuration * config;

    QPixmap     pixmap;
    QImage      bkgdImage;
    QImage      adjustedImage;

    Xform       xf_bkImg;

    QString     bkgdName;
    bool        _loaded;
};

#endif // BACKGROUNDIMAGE_H
