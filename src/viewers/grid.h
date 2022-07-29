#ifndef GRID_H
#define GRID_H

#include "style/thick.h"

typedef std::shared_ptr<class Grid> GridPtr;
typedef std::shared_ptr<class TilingSelector>   TilingSelectorPtr;

class Grid : public Thick
{
public:
    static GridPtr getSharedInstance();
    Grid(PrototypePtr pp);  // don't use this

    void draw(GeoGraphics * gg ) override;

    void create();

    void resetStyleRepresentation()  override {};
    void createStyleRepresentation() override {};

    virtual QString getStyleDesc() const override { return "Grid" ;}

    bool nearGridPoint(QPointF spt, QPointF & foundGridPoint);

protected:
    void createGridModelUnits(QRectF r);
    void createGridModelUnitsCentered(QRectF r);
    void createGridSceneUnits(const QRectF r);
    void createGridSceneUnitsCentered(QRectF r);

private:
    static GridPtr spThis;

    Configuration * config;
    ViewControl   * view;

    MapPtr      gridMap;

    QLineF      corners[2];
};

#endif // GRID_H
