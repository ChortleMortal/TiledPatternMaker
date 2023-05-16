#pragma once
#ifndef MARK_X_H
#define MARK_X_H

#include <QPainter>

#include "misc/layer.h"


// MarkX
class MarkX : public Layer
{
public:
    MarkX(QPointF a, QPen  pen, int index=0);
    MarkX(QPointF a, QPen  pen, QString txt);

    void setHuge() {huge = true;}

    void paint(QPainter *painter) override;

    virtual void iamaLayer() override {}

private:
    bool    huge;
    QPointF _a;
    QPen    _pen;
    int     _index;
    QString _txt;
};

typedef std::shared_ptr<MarkX>           MarkXPtr;

#endif
