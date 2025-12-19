#pragma once
#ifndef OUTLINE_CASING_H
#define OUTLINE_CASING_H

#include "model/styles/casing.h"
#include "model/styles/casing_set.h"
#include "model/styles/casing_side.h"

typedef std::shared_ptr<class OutlineCasing>     OutlineCasingPtr;

class OutlineCasingSet : public CasingSet
{
public:
    OutlineCasingSet() {}
};

class OutlineCasing : public Casing
{
public:
    OutlineCasing(CasingSet * owner, const EdgePtr&  edge, qreal width);
    ~OutlineCasing();

    void    init();
    void    set(QList<QPointF> & points);

    void setPainterPath() override;
    bool validate() override;
};

#endif
