#ifndef SCENE_H
#define SCENE_H

#include <QtCore>
#include <QtWidgets>

class View;
class WorkspaceViewer;
class Configuration;

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    Scene(View *view);

    [[noreturn]]void    clear() { qFatal("Do not call this"); }

     bool paintBackground;

signals:

public slots:


protected:
    void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

    void drawGridModelUnits(QPainter *painter, const QRectF & r);
    void drawGridSceneUnits(QPainter *painter, const QRectF & r);
    void drawGridModelUnitsCentered(QPainter *painter, QRectF & r);
    void drawGridSceneUnitsCentered(QPainter *painter, QRectF & r);

private:
 	WorkspaceViewer * viewer;
    Configuration * config;

    QPen gridPen;
};

#endif // SCENE_H
