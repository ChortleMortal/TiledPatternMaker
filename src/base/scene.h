#ifndef SCENE_H
#define SCENE_H

#include <QtCore>
#include <QtWidgets>

class View;

class WorkspaceViewer;

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

private:
     WorkspaceViewer * viewer;

     QPen gridPen;
};

#endif // SCENE_H
