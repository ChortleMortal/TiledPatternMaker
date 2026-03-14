#pragma once
#ifndef FILL_FACES_H
#define FILL_FACES_H

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

typedef QPair<QPointF,int>  FaceColor;      // CENTER OF  COLOR, COLOR PALETTE INDEX
typedef QVector<FaceColor>  FaceColorList;

// Key use wrapper for QPointF
struct PointKey
{
    QPointF p;

    bool operator==(const PointKey &other) const noexcept
    {
        return p == other.p;
    }
};

// qHash overload for PointKey
inline size_t qHash(const PointKey &key, size_t seed = 0) noexcept
{
    // Simple, deterministic hash combining x and y
    const auto x = qHash(key.p.x(), seed);
    return qHash(key.p.y(), x);
}

class DirectColoring : public ColorMaker
{
public:
    DirectColoring(Filled * filled);

    void setColorList(FaceColorList & list) { faceColorList = list; }
    void createFromDeprecated(DCELPtr dcel);
    void create(DCELPtr dcel);
    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

public:
    QHash<PointKey, FacePtr>    faceMap;
    FaceColorList               faceColorList;
    ColorSet                    palette;
};

#endif

