#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include "base/layer.h"

class  BackgroundImage : public Layer
{
public:
    BackgroundImage(QString imageName);
    ~BackgroundImage() override;

    void    paint(QPainter *painter) override;

    static bool import(QString filename);

    void    createPixmap();

    void    createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    createAdjustedImage();

    bool    useAdjusted() { return bUseAdjusted; }
    void    setUseAdjusted(bool use);
    bool    saveAdjusted(QString newName);

    bool    isLoaded() { return _loaded; }
    QString getName()  { return bkgdName; }

    void     updateBkgdXform(const Xform & xf);
    const Xform & getBkgdXform()  { return xf_bkImg; }

    // public data
    QTransform perspective;

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
    bool    load(QString imageName);   // loads from existing imported file
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    drawCenter(QPainter * painter) override;

private:
    View          * view;
    Configuration * config;

    QPixmap     pixmap;         // is painted, created from bkgdImage or adjusted image
    QImage      bkgdImage;      // as loaded
    QImage      adjustedImage;  // as adjusted

    bool        bUseAdjusted;

    Xform       xf_bkImg;

    QString     bkgdName;
    bool        _loaded;
};

#endif // BACKGROUNDIMAGE_H
