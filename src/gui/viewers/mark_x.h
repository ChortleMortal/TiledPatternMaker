#pragma once
#ifndef MARK_X_H
#define MARK_X_H

#include <QPainter>
#include "gui/viewers/layer.h"

class MarkX : public Layer
{
public:
    MarkX(QPointF a, QPen  pen, int index=0);
    MarkX(QPointF a, QPen  pen, QString txt);

    void setHuge() {huge = true;}

    void paint(QPainter *painter) override;

    eViewType iamaLayer() override { return VIEW_CENTER; }

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

private:
    bool    huge;
    QPointF _a;
    QPen    _pen;
    int     _index;
    QString _txt;
};

typedef std::shared_ptr<MarkX>  MarkXPtr;

#endif
