#pragma once
#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QTransform>
#include <QGraphicsPixmapItem>
#include "misc/layer_controller.h"
#include "settings/frame_settings.h"
#include "geometry/edgepoly.h"

class Perspective
{
public:
    Perspective();
    void resetPerspective();
    void startDragging(QPointF spt);
    void updateDragging(QPointF spt );
    void endDragging(QPointF spt );
    void addPoint(QPointF spt);
    void drawPerspective(QPainter *painter );

    void       setSkewMode(bool enb);
    bool       getSkewMode() { return skewMode; }
    EdgePoly & getAccum()    { return sAccum; }

protected:
    void forceRedraw();

    EdgePoly    sAccum;       // screen points

private:
    bool        skewMode;
    QPointF     spt;
    QPointF     sLastDrag;   // screen point
    QPolygonF   poly;
};

class  BackgroundImageView : public LayerController, public Perspective
{
public:
    static BackgroundImageView * getInstance();
    static void              releaseInstance();

    bool    import(QString filename);   // import into media/bkgds
    bool    load(QString imageName);    // loads from existing imported file
    void    unload();

    bool    isLoaded() { return _loaded; }
    QString getName() override;

    void    paint(QPainter *painter) override;

    void    showPixmap();
    void    createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    createAdjustedImage();

    bool    useAdjusted() { return bUseAdjusted; }
    void    setUseAdjusted(bool use);
    bool    saveAdjusted(QString newName);

    virtual const Xform  & getCanvasXform() override;
    virtual void           setCanvasXform(const Xform & xf) override;

    // public data
    QTransform perspective;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;
    virtual void slot_setCenter(QPointF spt)          override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

protected:
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

private:
    BackgroundImageView();
    ~BackgroundImageView();

    static BackgroundImageView * mpThis;

    ViewControl   * view;
    Configuration * config;

    FrameData     * frameData;

    Xform           xf_canvas;

    QPixmap         pixmap;         // is painted, created from bkgdImage or adjusted image
    QImage          bkgdImage;      // as loaded
    QImage          adjustedImage;  // as adjusted

    bool            bUseAdjusted;

    QString         bkgdName;
    bool            _loaded;
};



#endif // BACKGROUNDIMAGE_H
