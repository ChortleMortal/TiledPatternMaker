#include <QImage>
#include <QFileDialog>
#include <QPainter>

#include "gui/viewers/backgroundimageview.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/gui_modes.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "sys/sys.h"
#include "model/settings/configuration.h"
#include "gui/top/view_controller.h"
#include "gui/top/controlpanel.h"

using std::make_shared;

/*
 * The BackgroundImageView only uses the model transform and does not use
 * the Layer transform or the Cnavas transform, because the canvas transform
 * uses a scale factor and translation relative to the model Bounds.
 * These bounds do not apply to the image.
 */

BackgroundImageView::BackgroundImageView() : LayerController("Bkgd Image",true)
{
    config   = Sys::config;
    setZValue(BKGD_IMG_ZLEVEL);
}

BackgroundImageView::~BackgroundImageView()
{
}

void BackgroundImageView::scaleImage()
{
    BkgdImagePtr bip = wbip.lock();
    if (!bip)  return;

    // scales the image to the correct size but does not position it.
    qreal canvasScale = viewControl->getCanvas().getScale();
    QTransform t      = QTransform::fromScale(canvasScale,canvasScale) * bip->getImageXform().toQTransform();
    QPixmap & pixmap  = bip->getPixmap();
    scaledPixmap      = pixmap.transformed(t);
}

QPointF BackgroundImageView::calcDelta()
{
    auto & canvas = viewControl->getCanvas();
    QSizeF sz     = canvas.getSize();
    //qDebug() << "BackgroundImageView canvas size" << sz;

    QPointF view_center(sz.width()/2.0, sz.height()/2.0);
    QRectF rect(scaledPixmap.rect());
    QPointF delta  = view_center - rect.center();
    qDebug() << "BackgroundImageView delta" << delta;
    return delta;
}

void BackgroundImageView::paint(QPainter *painter)
{
    if (!config->showBackgroundImage) return;

    BkgdImagePtr bip = wbip.lock();
    if (!bip) return;

    qDebug() << "BackgroundImageView::paint";

    scaleImage();

    QPointF pt     = bip->getImageXform().getTranslate();
    QPointF delta  = calcDelta();
    delta        += pt;

    painter->drawPixmap(delta,scaledPixmap);

    drawLayerModelCenter(painter);

    drawSkew(painter);
}

void BackgroundImageView::drawSkew(QPainter * painter)
{
    if (getSkewMode())
    {
        // draw accum
        QColor construction_color(0, 128, 0,128);
        if ( sAccum.size() > 0)
        {
            QPen pen(construction_color,3);
            QBrush brush(construction_color);
            painter->setPen(pen);
            painter->setBrush(brush);
            for (EdgePtr edge : std::as_const(sAccum))
            {
                if (edge->getType() == EDGETYPE_LINE)
                {
                    QPointF p1 = edge->v1->pt;
                    QPointF p2 = edge->v2->pt;
                    painter->drawEllipse(p1,6,6);
                    painter->drawEllipse(p2,6,6);
                    painter->drawLine(p1, p2);
                }
                else if (edge->getType() == EDGETYPE_POINT)
                {
                    QPointF p = edge->v1->pt;
                    painter->drawEllipse(p,6,6);
                }
            }
            drawPerspective(painter);
        }
    }
}

// this is perspective correction
// for images where camera was not normal to the plane of the tiling
void BackgroundImageView::createBackgroundAdjustment(BkgdImagePtr img, QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    BkgdImagePtr bip = wbip.lock();
    if (!bip) return;

    QPixmap & pixmap = bip->getPixmap();
    QSize sz      = pixmap.size();
    qreal offsetX = qreal(Sys::view->width() -  sz.width()) / 2.0;
    qreal offsetY = qreal(Sys::view->height() - sz.height()) / 2.0;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);
    
    QTransform bkgdXform = t0 * getModelTransform();
    QTransform t1        = bkgdXform.inverted();

    img->correctPerspective(
        t1.map(topLeft),
        t1.map(topRight),
        t1.map(botRight),
        t1.map(botLeft));

    img->setUseAdjusted(true);     // since we have just created it, let's use it

    img->createAdjustedImage();
}

void BackgroundImageView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    auto bip = wbip.lock();
    if (bip)
    {
        bip->setImageXform(xf);
        forceLayerRecalc(update);
    }
}

const Xform & BackgroundImageView::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    auto bip = wbip.lock();
    if (bip)
    {
        return bip->getImageXform();
    }
    else
    {
        return nullXform;
    }
}

////////////////////////////////////////////////////////
///
/// Layer slots
///
////////////////////////////////////////////////////////

void BackgroundImageView::slot_setCenter(QPointF spt)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void BackgroundImageView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (startDragging(spt))
        emit sig_updateView();
}

void BackgroundImageView::slot_mouseDragged(QPointF spt)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (updateDragging(spt))
        emit sig_updateView();
}

void BackgroundImageView::slot_mouseTranslate(QPointF pt)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setModelXform(xf,true);
    }
}

void BackgroundImageView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }

void BackgroundImageView::slot_mouseReleased(QPointF spt)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (endDragging(spt))
        emit sig_updateView();
}

void BackgroundImageView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }


void BackgroundImageView::slot_wheel_scale(qreal delta)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
        scaleImage();
    }
}

void BackgroundImageView::slot_wheel_rotate(qreal delta)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void BackgroundImageView::slot_scale(int amount)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
        scaleImage();
    }
}

void BackgroundImageView::slot_rotate(int amount)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
}

void BackgroundImageView::slot_moveX(qreal amount)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
}

void BackgroundImageView::slot_moveY(qreal amount)
{
    if (!Sys::view->isActiveLayer(VIEW_BKGD_IMG)) return;

    if (    Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  Sys::guiModes->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
}

/////////
///
///  Perspective
///
/////////

Perspective::Perspective()
{
    skewMode = false;
}

bool Perspective::startDragging(QPointF spos)
{
    if (!skewMode)
        return false;

    if (sAccum.size() == 0)
    {
        return addPoint(spos);
    }

    return false;
}

bool Perspective::addPoint(QPointF spos)
{
    if (!skewMode)
        return false;

    qDebug("Perspective::addPoint");

    VertexPtr vnew = std::make_shared<Vertex>(spos);

    int size = sAccum.size();
    if (size == 0)
    {
        sAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
    }
    else if (size == 1)
    {
        EdgePtr last = sAccum.last();
        if (last->getType() == EDGETYPE_POINT)
        {
            last->setV2(vnew);
            qDebug() << "edge count =" << sAccum.size();
        }
        else
        {
            sAccum.push_back(make_shared<Edge>(last->v2,vnew));
            qDebug() << "edge count =" << sAccum.size();
        }
    }
    else if (size == 2)
    {
        EdgePtr last = sAccum.last();
        sAccum.push_back(make_shared<Edge>(last->v2,vnew));
        qDebug() << "edge count = " << sAccum.size();
        sAccum.push_back(make_shared<Edge>(vnew,sAccum.first()->v1));
        qDebug() << "completed with edge count" << sAccum.size();
        return false;
    }
    sLastDrag = QPointF();
    return true;
}

bool Perspective::updateDragging(QPointF spt)
{
    if (!skewMode)
        return false;

    sLastDrag = spt;
    return true;
}

bool Perspective::endDragging(QPointF spt)
{
    if (!skewMode)
        return false;

    if (sAccum.size() == 0)
        return false;
    
    if (!Geo::isNear(spt,sAccum.first()->v1->pt))
    {
        addPoint(spt);
    }
    sLastDrag = QPointF();
    return true;
}

void Perspective::drawPerspective(QPainter * painter)
{
    if (sAccum.size() > 0 && !sLastDrag.isNull())
    {
        // draws line while dragginhg
        QColor drag_color = QColor(206,179,102,230);
        painter->setPen(QPen(drag_color,3));
        painter->setBrush(QBrush(drag_color));
        painter->drawLine(sAccum.last()->v2->pt,sLastDrag);
        painter->drawEllipse(sLastDrag,10,10);
    }
}

#if 0
void Perspective::forceRedraw()
{
    emit sig_updateView();
}
#endif

void Perspective::setSkewMode(bool enb)
{
    skewMode = enb;
    if (enb)
    {
        sAccum.clear();
        sLastDrag = QPointF();
    }
}

