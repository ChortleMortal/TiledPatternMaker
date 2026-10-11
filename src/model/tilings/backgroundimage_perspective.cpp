#include <QImage>
#include <QFileDialog>
#include <QPainter>
#include <QApplication>

#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/top/view_controller.h"
#include "model/tilings/backgroundimage_perspective.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

/////////
///
///  BackgroundImagePerspective
///
/////////

BackgroundImagePerspective::BackgroundImagePerspective()
{
    active = false;
}

bool BackgroundImagePerspective::startDragging(QPointF spos)
{
    if (!active)
        return false;

    if (sAccum.size() == 0)
    {
        return addPoint(spos);
    }

    if (sAccum.size() == 4)
    {
        for (EdgePtr & edge : sAccum)
        {
            if (Geo::isNear(spos, parent->keystoneModelToScreen(edge->v1->pt)))
            {
                dragVertex = edge->v1;
                return true;
            }
        }
    }

    return false;
}

bool BackgroundImagePerspective::addPoint(QPointF spos)
{
    if (!active)
        return false;

    qDebug("Perspective::addPoint");

    QPointF mpt = parent->keystoneScreenToModel(spos);
    VertexPtr vnew = std::make_shared<Vertex>(mpt);

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

bool BackgroundImagePerspective::updateDragging(QPointF spt)
{
    if (!active)
        return false;

    if (dragVertex)
    {
        dragVertex->setPt(parent->keystoneScreenToModel(spt));
        return true;
    }

    if (sAccum.size() == 4)
        return false;

    sLastDrag = parent->keystoneScreenToModel(spt);
    return true;
}

bool BackgroundImagePerspective::endDragging(QPointF spt)
{
    if (!active)
        return false;

    if (dragVertex)
    {
        dragVertex->setPt(parent->keystoneScreenToModel(spt));
        dragVertex.reset();
        return true;
    }

    if (sAccum.size() == 0)
        return false;

    // isNear tolerance is in screen pixels
    QPointF sFirst = parent->keystoneModelToScreen(sAccum.first()->v1->pt);
    if (!Geo::isNear(spt,sFirst))
    {
        addPoint(spt);
    }
    sLastDrag = QPointF();
    return true;
}

void BackgroundImagePerspective::draw(QPainter * painter)
{
    // draw accum
    if (sAccum.size() > 0)
    {
        QColor construction_color(0, 128, 0,128);
        QPen pen(construction_color,3);
        QBrush brush(construction_color);
        painter->setPen(pen);
        painter->setBrush(brush);
        for (EdgePtr & edge : sAccum)
        {
            if (edge->getType() == EDGETYPE_LINE)
            {
                QPointF p1 = parent->keystoneModelToScreen(edge->v1->pt);
                QPointF p2 = parent->keystoneModelToScreen(edge->v2->pt);
                painter->drawEllipse(p1,6,6);
                painter->drawEllipse(p2,6,6);
                painter->drawLine(p1, p2);
            }
            else if (edge->getType() == EDGETYPE_POINT)
            {
                QPointF p = parent->keystoneModelToScreen(edge->v1->pt);
                painter->drawEllipse(p,6,6);
            }
        }
        drawPerspective(painter);
    }
}

void BackgroundImagePerspective::drawPerspective(QPainter * painter)
{
    if (sAccum.size() > 0 && !sLastDrag.isNull())
    {
        // draws line while dragginhg
        QColor drag_color = QColor(206,179,102,230);
        painter->setPen(QPen(drag_color,3));
        painter->setBrush(QBrush(drag_color));
        QPointF p1 = parent->keystoneModelToScreen(sAccum.last()->v2->pt);
        QPointF p2 = parent->keystoneModelToScreen(sLastDrag);
        painter->drawLine(p1,p2);
        painter->drawEllipse(p2,10,10);
    }
}

#if 0
void Perspective::forceRedraw()
{
    emit sig_updateView();
}
#endif

void BackgroundImagePerspective::activate(bool enb, BackgroundImage * parent)
{
    if (enb)
    {
        this->parent = parent;
        savedXform   = parent->getModelXform();
        clear();
        emit Sys::systemViewController->sig_solo(parent,true);     // navigation moves only the image
    }
    else if (active && this->parent)
    {
        // pop the view back to where it was before this session's navigation
        this->parent->setModelXform(savedXform, true, Sys::nextSigid());
        emit Sys::systemViewController->sig_solo(this->parent,false);
    }

    active = enb;
}

void BackgroundImagePerspective::clear()
{
    sAccum.clear();
    sLastDrag = QPointF();
    dragVertex.reset();
}

/////////
///
///  BackgroundImageCropper
///
/////////

BackgroundImageCropper::BackgroundImageCropper()
{
    active = false;
    editor = nullptr;
}

BackgroundImageCropper::~BackgroundImageCropper()
{
    if (editor)
        delete editor;
}

bool BackgroundImageCropper::startDragging(QPointF spt)
{
    if (editor)
        delete editor;

    setMousePos(spt);
    editor = new MouseEditCrop(spt,crop,parent->getLayerTransform());

    return true;
}

bool BackgroundImageCropper::updateDragging(QPointF spt )
{
    if (editor)
    {
        setMousePos(spt);
        editor->updateDragging(spt,parent->getLayerTransform());
        return true;
    }
    return false;
}

bool BackgroundImageCropper::endDragging(QPointF spt )
{
    if (editor)
    {
        setMousePos(spt);
        editor->endDragging(spt,parent->getLayerTransform());
        delete editor;
        editor = nullptr;
        return true;
    }
    return false;
}

void BackgroundImageCropper::draw(QPainter *painter )
{
    if (active)
    {
        crop->draw(painter,parent->getLayerTransform(),true);
        if (editor)
        {
            editor->draw(painter,mousePos,parent->getLayerTransform());
        }
    }
}

void BackgroundImageCropper::activate(bool enb, BackgroundImage * parent, Crop *crop)
{
    bool wasActive = active;
    active         = enb;

    if (enb)
    {
        Q_ASSERT(parent);
        Q_ASSERT(crop);
        this->parent = parent;
        this->crop   = crop;

        if (crop->getRect().isNull())
        {
            // this sets up a default crop
            QRectF srect    = Sys::systemViewController->getViewRect();
            QPointF scenter = srect.center();
            QSizeF ssz      = srect.size();
            ssz            *= 0.7;
            srect.setSize(ssz);
            srect.moveCenter(scenter);

            QRectF mrect    = parent->screenToModel(srect);
            crop->setRect(mrect);
        }
        emit Sys::systemViewController->sig_solo(parent,true);     // navigation moves only the image
    }
    else
    {
        if (wasActive && this->parent)
            emit Sys::systemViewController->sig_solo(this->parent,false);
        this->parent = nullptr;
        this->crop   = nullptr;
    }
}

void BackgroundImageCropper::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}
