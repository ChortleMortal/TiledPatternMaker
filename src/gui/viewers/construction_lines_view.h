#pragma once

#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/viewers/layer_controller.h"

class ConstructionLinesView : public LayerController
{
    Q_OBJECT

public:
    ConstructionLinesView();
    ~ConstructionLinesView() override;

    void paint(QPainter * painter) override;

    // screen-space hit tests on model-space lines, shared with the Map Editor
    static int findLineNear(const QVector<QLineF> & lines, const QTransform & t, QPointF spt);
    static int findEndpointNear(const QLineF & line, const QTransform & t, QPointF spt);

protected:
    void draw(QPainter * painter);
    bool isEditing();

    MouseActionPtr  mouse_interaction;
    int             hoverEndpoint = -1;    // 0/1 = hovering in grab-zone of selected line's endpoint, -1 = not

    void iamaLayerController() override {}

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;
};
