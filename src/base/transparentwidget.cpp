/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include "base/transparentwidget.h"
#include "base/tiledpatternmaker.h"
#include "base/shared.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"


ImageWidget::ImageWidget() : QLabel()
{
    setAttribute(Qt::WA_DeleteOnClose);
    connect(theApp, &TiledPatternMaker::sig_closeAllImageViewers,this, &ImageWidget::slot_closeMe);
}

void ImageWidget::keyPressEvent( QKeyEvent *k )
{
    theApp->imageKeyPressed(k);
}

void ImageWidget::slot_closeMe()
{
    QTimer::singleShot(500, this, &ImageWidget::close);
}

TransparentWidget::TransparentWidget()
{
    // this makes it transparent
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::FramelessWindowHint);
    onTop = true;

    // this gives some ImageLayer handle to grab on to
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(20);
    setStyleSheet("color: rgba(255,0,0,64);");

    // also
    setAttribute(Qt::WA_DeleteOnClose);
}

void TransparentWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        oldPos = event->globalPos();
#else
        oldPos = event->globalPosition().toPoint();
#endif
    }
    else if (event->button() == Qt::RightButton)
    {
        close();    // deletes
    }
}

void TransparentWidget::mouseMoveEvent(QMouseEvent *event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    QPoint delta = event->globalPos() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPos();
#else
    QPoint delta = event->globalPosition().toPoint() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPosition().toPoint();
#endif
}

void TransparentWidget::keyPressEvent( QKeyEvent *k )
{
    int key = k->key();
    switch (key)
    {
    case 'Q':
        close();    // deletes
        break;
    case 'T':
        onTop = !onTop;
        setWindowFlag(Qt::WindowStaysOnTopHint,onTop);
        break;
    default:
        break;
    }
}

ImageLayer::ImageLayer() : Layer("Image")
{
    config = Configuration::getInstance();
    view   = View::getInstance();
    vcontrol = ViewControl::getInstance();

    connect(this, &ImageLayer::sig_refreshView, vcontrol, &ViewControl::slot_refreshView);

}


void ImageLayer::paint(QPainter *painter)
{
    if (pixmap.isNull())
        return;

    painter->save();
    painter->setTransform(xf_canvas.getTransform());
    QRectF rect = pixmap.rect();
    painter->drawPixmap(rect,pixmap,rect);
    painter->restore();

}

void ImageLayer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (btn == Qt::RightButton)
    {
        qDebug() << "ImageLayer -right click";

        QMenu menu(view);
        menu.addAction("Delete",this,&ImageLayer::slot_deleteAction);
        menu.exec(view->mapToGlobal(spt.toPoint()));
    }
    else
    {
        if (    config->kbdMode == KBD_MODE_XFORM_VIEW
            || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
        {
            qDebug() << getName() << config->kbdMode;
            if (view->getMouseMode() == MOUSE_MODE_CENTER && btn == Qt::LeftButton)
            {
                setCenterScreenUnits(spt);
                forceLayerRecalc();
                emit sig_refreshView();
            }
        }
    }
}

void ImageLayer::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseTranslate(QPointF pt)
{

    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + pt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + pt.y());
        forceLayerRecalc();
    }
}

void ImageLayer::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_wheel_scale(qreal delta)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceLayerRecalc();
    }
}

void ImageLayer::slot_wheel_rotate(qreal delta)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceLayerRecalc();
    }
}

void ImageLayer::slot_scale(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceLayerRecalc();
    }
}

void ImageLayer::slot_rotate(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceLayerRecalc();
    }
}

void ImageLayer:: slot_moveX(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceLayerRecalc();
    }
}

void ImageLayer::slot_moveY(int amount)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceLayerRecalc();
    }
}

void ImageLayer::slot_deleteAction()
{
    vcontrol->removeImage(this);
    emit sig_refreshView();
}

