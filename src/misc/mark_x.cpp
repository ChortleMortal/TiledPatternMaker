#include "misc/mark_x.h"
#include "misc/layer.h"

MarkX::MarkX(QPointF a, QPen pen, int index) : Layer("MarkX",false)
{
    _a     = a;
    _pen   = pen;
    _index = index;
    huge   = false;

}

MarkX::MarkX(QPointF a, QPen pen, QString txt) : Layer("MarkX",false)
{
    _a     = a;
    _pen   = pen;
    _txt   = txt;
    huge   = false;
}

void MarkX::paint(QPainter *painter)
{
    painter->setPen(_pen);

    qreal len   = 5;
    if (huge)
    {
        len = 200;
    }

    painter->drawLine(QPointF(_a.x()-len,_a.y()),QPointF(_a.x()+len,_a.y()));
    painter->drawLine(QPointF(_a.x(),_a.y()-len),QPointF(_a.x(),_a.y()+len));

    qreal width = 120;
    QRectF arect(_a.x()+20,_a.y()-10,width,20);
    if (_txt.isEmpty())
    {
        painter->drawText(arect,QString::number(_index));
    }
    else
    {
        painter->drawText(arect,_txt);
    }
}

void MarkX::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & MarkX::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().info() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}

