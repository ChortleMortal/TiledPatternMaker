#pragma once
#ifndef BACKGROUNDIMAGE_VIEW_H
#define BACKGROUNDIMAGE_VIEW_H

#include <QTransform>
#include <QGraphicsPixmapItem>
#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class BackgroundImage> BkgdImagePtr;
typedef std::weak_ptr<class BackgroundImage> WekBkgdImagePtr;

class Perspective
{
public:
    Perspective();
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

class BackgroundImage;

class  BackgroundImageView : public LayerController, public Perspective
{
public:
    BackgroundImageView();
    ~BackgroundImageView();

    void         paint(QPainter *painter) override;

    void         createBackgroundAdjustment(BkgdImagePtr img, QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

    void         setImage(BkgdImagePtr bip) { wbip = bip; }
    BkgdImagePtr getImage()                 { return wbip.lock(); }

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseTranslate(QPointF pt)      override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;
    void slot_setCenter(QPointF spt)          override;

    void slot_wheel_scale(qreal delta)  override;
    void slot_wheel_rotate(qreal delta) override;

    void slot_scale(int amount)  override;
    void slot_rotate(int amount) override;
    void slot_moveX(qreal amount)  override;
    void slot_moveY(qreal amount)  override;

    eViewType iamaLayer() override { return VIEW_BKGD_IMG; }
    void iamaLayerController() override {}

protected:
    void    scaleImage();
    QPointF calcDelta();
    void    drawSkew(QPainter * painter);

private:

    WekBkgdImagePtr wbip;
    Configuration * config;
    QPixmap         scaledPixmap; // is painted, created from bkgdImage or adjusted image
    Xform           nullXform;
};

#endif // BACKGROUNDIMAGE_H
