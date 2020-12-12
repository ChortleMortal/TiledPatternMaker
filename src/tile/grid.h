#ifndef GRID_H
#define GRID_H

#include "base/layer.h"



class Grid : public Layer
{
public:
    Grid();

    void paint(QPainter * painter);

protected:
    void drawGridModelUnits(QPainter *painter, const QRectF &r);
    void drawGridModelUnitsCentered(QPainter *painter, QRectF &r);
    void drawGridSceneUnits(QPainter *painter, const QRectF &r);
    void drawGridSceneUnitsCentered(QPainter *painter, QRectF & r);

private:
    Configuration * config;
    View          * view;

    QPen gridPen;
};

#endif // GRID_H
