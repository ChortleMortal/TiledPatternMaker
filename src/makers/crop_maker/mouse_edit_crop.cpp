#include "mouse_edit_crop.h"
#include "geometry/crop.h"
#include "geometry/transform.h"
#include "misc/utilities.h"

#include <QDebug>

/////////////////////////////////////////////////////////
///
///  Mouse Edit Crop
///
/////////////////////////////////////////////////////////

MouseEditCrop::MouseEditCrop(QPointF spt, CropPtr crop, QTransform t)
{
    start        = nullptr;
    end          = nullptr;
    this->crop   = crop;
    ecMode       = EC_READY;
    ecCorner     = UNDEFINED;

    QTransform tinv = t.inverted();

    if (crop->getCropType() == CROP_CIRCLE)
    {
        auto circle      = crop->getCircle();
        QPointF center   = t.map(circle.centre);
        qreal radius     = Transform::scalex(t) * circle.radius;

        Circle sc(center,radius);
        if (Utils::pointOnCircle(spt,sc,7))
        {
            start = new QPointF(tinv.map(spt));
            ecCircleMode = CM_EDGE;
        }
        else if (Utils::pointInCircle(spt,sc))
        {
            start = new QPointF(tinv.map(spt));
            ecCircleMode = CM_INSIDE;
        }
        else
        {
            ecCircleMode = CM_OUTSIDE;
        }
    }
    else
    {
        start = new QPointF(tinv.map(spt));
    }
}

void MouseEditCrop::draw(QPainter * painter,QPointF mousePos, QTransform t)
{
    crop->draw(painter,t,true);

    QRectF rect = crop->getRect();
    rect = t.mapRect(rect);
    QPointF pt  = mousePos;
    if (   Point::isNear(pt,rect.bottomRight())
        || Point::isNear(pt,rect.topRight())
        || Point::isNear(pt,rect.bottomLeft())
        || Point::isNear(pt,rect.topLeft()))
    {
        qreal radius = 8.0;
        painter->setPen(QPen(Qt::blue,1));
        painter->setBrush(Qt::blue);
        painter->drawEllipse(pt, radius, radius);
    }
}

void MouseEditCrop::updateDragging(QPointF spt, QTransform t)
{
    if (!crop)
    {
        return;
    }
    if (!start)
    {
        return;
    }

    QTransform tinv = t.inverted();

    QPointF mpt = tinv.map(spt);

    if (crop->getCropType() == CROP_RECTANGLE)
    {
        QRectF rect = crop->getRect();
        switch(ecMode)
        {
        case EC_READY:
            if (Point::isNear(spt,t.map(rect.topLeft())))
                ecCorner = TL;
            else if (Point::isNear(spt,t.map(rect.topRight())))
                ecCorner = TR;
            else if (Point::isNear(spt,t.map(rect.bottomRight())))
                ecCorner = BR;
            else if (Point::isNear(spt,t.map(rect.bottomLeft())))
                ecCorner = BL;
            if (ecCorner != UNDEFINED)
            {
                start  = new QPointF(mpt);
                ecMode = CB_RESIZE;
            }
            else if (Utils::rectContains(rect,mpt))
            {
                start  = new QPointF(mpt);
                ecMode = EC_MOVE;
            }
            break;

        case EC_MOVE:
        {
            //qDebug() << "move";
            QPointF delta = mpt - *start;
            delete start;
            start  = new QPointF(mpt);
            rect.moveTopLeft(rect.topLeft() + delta);
        }
        break;
        case CB_RESIZE:
        {
            QPointF delta = mpt - *start;
            delete start;
            start  = new QPointF(mpt);

            QSizeF oldSize = rect.size();
            QSizeF newSize;
            switch (ecCorner)
            {
            case TL:    // ok
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() - delta.y());
                rect.setSize(newSize);
                rect.moveTopLeft(mpt);
                break;
            case TR:
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() - delta.y());
                rect.setSize(newSize);
                rect.moveTopRight(mpt);
                break;
            case BL:
                newSize = QSizeF(oldSize.width() - delta.x(),oldSize.height() + delta.y());
                rect.setSize(newSize);
                rect.moveBottomLeft(mpt);
                break;
            case BR:    // ok
                newSize = QSizeF(oldSize.width() + delta.x(),oldSize.height() + delta.y());
                rect.setSize(newSize);
                rect.moveBottomRight(mpt);
                break;
            case UNDEFINED:
                break;
            }

            //qDebug() << "resize: delta" << delta << "old" << oldSize << "new" << newSize;
        }
        break;
        }
        crop->setRect(rect);
    }
    else if (crop->getCropType() == CROP_CIRCLE)
    {
        auto c = crop->getCircle();
        if (ecCircleMode == CM_EDGE)
        {
            // first find direction
            qreal s = Point::dist2(*start,c.centre);
            qreal m = Point::dist2(mpt,c.centre);
            bool sub = (s > m);
            // find delta
            qreal delta = Point::dist(mpt,*start);
            delete start;
            start  = new QPointF(mpt);
            if (sub)
                delta = -delta;
            qreal radius = c.radius + delta;
            c.setRadius(radius);
        }
        else if (ecCircleMode == CM_INSIDE)
        {
            QPointF delta = mpt - *start;
            delete start;
            start  = new QPointF(mpt);

            QPointF centre = c.centre + delta;
            c.setCenter(centre);
        }
    }
}

void MouseEditCrop::endDragging( QPointF spt, QTransform t)
{
    updateDragging(spt,t);

    //qDebug() << "end dragging";

    if (ecMode != EC_READY)
    {
        ecMode = EC_READY;
    }
    // else ignore
}

