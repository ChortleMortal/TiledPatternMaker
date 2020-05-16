#ifndef SCENE_H
#define SCENE_H

#include <QtCore>
#include <QtWidgets>

class View;
class WorkspaceViewer;
class Configuration;

class Scene : public QGraphicsScene
{
public:
    Scene();

    void    clear() { qFatal("Do not call this"); }

    void    setSceneRect(const QRectF & rect);
    void    setSceneRect(qreal x, qreal y, qreal w, qreal h);
    void    paintBackground(bool paint) { _paintBackground = paint; }

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

    void drawGridModelUnits(QPainter *painter, const QRectF & r);
    void drawGridSceneUnits(QPainter *painter, const QRectF & r);
    void drawGridModelUnitsCentered(QPainter *painter, QRectF & r);
    void drawGridSceneUnitsCentered(QPainter *painter, QRectF & r);

private:
    View            * view;
 	WorkspaceViewer * viewer;
    Configuration   * config;

    bool   _paintBackground;

    QPen gridPen;
};

#endif // SCENE_H
