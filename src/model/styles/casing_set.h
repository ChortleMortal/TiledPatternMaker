#pragma once
#ifndef CASING_SET_H
#define CASING_SET_H

#include <QVector>
#include "model/styles/casing_side.h"
#include "sys/geometry/map.h"

typedef std::shared_ptr<class Casing> CasingPtr;
typedef std::weak_ptr<class Casing>  wCasingPtr;


class CasingSet : public QVector<CasingPtr>
{
public:
    CasingSet();

    CasingPtr find(EdgePtr &edge)   { return this->at(edge->casingIndex); }
    CNeighboursPtr   getNeighbouringCasings(VertexPtr v) { return weavings.value(v); }

    void      buildMap();
    void      validate();

    void      dump(QString str);

    MapPtr    map;
    QMap<VertexPtr,CNeighboursPtr>  weavings;
};

#endif // CASING_SET_H
