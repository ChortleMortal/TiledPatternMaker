#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include "base/layer.h"
#include "settings/frame_settings.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class BackgroundImage>   BkgdImgPtr;
typedef std::shared_ptr<class Perspective>       PerspectivePtr;

class  BackgroundImage : public Layer
{
public:
    static BkgdImgPtr getSharedInstance();
    BackgroundImage();                  // don't use this

    bool    import(QString filename);   // import into media/bkgds
    bool    load(QString imageName);    // loads from existing imported file
    void    unload() { _loaded =false; }

    bool    isLoaded() { return _loaded; }
    QString getName()  { return bkgdName; }

    void    paint(QPainter *painter) override;

    void    createPixmap();
    void    createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    createAdjustedImage();

    bool    useAdjusted() { return bUseAdjusted; }
    void    setUseAdjusted(bool use);
    bool    saveAdjusted(QString newName);

    void     updateImageXform(const Xform & xf);
    const Xform & getImageXform()  { return xf_image; }

    void    setSkewMode(bool enb);
    bool    getSkewMode() { return skewMode; }

    // public data
    QTransform perspective;
    EdgePoly   sAccum;       // screen points

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

protected:
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

private:
    static BkgdImgPtr spThis;

    View          * view;
    Configuration * config;

    FrameData     * frameData;

    QPixmap         pixmap;         // is painted, created from bkgdImage or adjusted image
    QImage          bkgdImage;      // as loaded
    QImage          adjustedImage;  // as adjusted

    bool            bUseAdjusted;

    Xform           xf_image;

    QString         bkgdName;
    bool            _loaded;
    bool            skewMode;
    PerspectivePtr  mouse_interaction;
};

class Perspective
{
public:
    Perspective(QPointF spt);
    void updateDragging(QPointF spt );
    void draw(QPainter *painter );
    void endDragging(QPointF spt );
    void addPoint(QPointF spt);

private:
    BkgdImgPtr  bip;
    QPointF     spt;
    QPointF     sLastDrag;   // screen point
    QPolygonF   poly;
};


#endif // BACKGROUNDIMAGE_H
