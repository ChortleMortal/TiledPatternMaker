#pragma once
#ifndef LEGACY_TILE_H
#define LEGACY_TILE_H

#include "misc/layer.h"

class LegacyTile : public Layer
{
    Q_OBJECT

public:
    LegacyTile(int Row = 0, int Col = 0);
    ~LegacyTile();

    void setTilePosition(int Row, int Col) {row = Row; col = Col;}

    int getRow() { return row;}
    int getCol() { return col;}

    void     addLayer(LayerPtr layer, int zlevel = 0);

    virtual bool doStep(int index);
    virtual void reset() {}

    static int refs;

    void tile_rotate(int amount);

    virtual const Xform &   getModelXform() override;
    virtual void            setModelXform(const Xform & xf, bool update) override;

    eViewType iamaLayer() override { return VIEW_DESIGN; }

protected:
    qreal getSLinearPos(int step, int duration);

    bool odd (int i) { return ((i&1) == 1);}
    bool even(int i) { return ((i&1) == 0);}

    int     stepIndex;

    int     instance;
    int     row;
    int     col;

private:
};

#endif // TILE_H
