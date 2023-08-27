#pragma once
#ifndef BORDER_VIEW_H
#define BORDER_VIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class Border>          BorderPtr;
typedef std::shared_ptr<class MouseEditBorder> BorderMouseActionPtr;
typedef std::weak_ptr<class Border>            WeakBorderPtr;

class BorderView : public LayerController
{
public:
    static BorderView * getInstance();
    static void         releaseInstance();

    void          setBorder(BorderPtr border) { this->border = border; }
    virtual void  paint(QPainter * painter) override;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;
    virtual void slot_setCenter(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

    virtual eViewType iamaLayer() override { return VIEW_BORDER; }
    virtual void iamaLayerController() override {}

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

protected:
    void                  setMouseInteraction(BorderMouseActionPtr action) { mouse_interaction = action; }
    void                  resetMouseInteraction() { mouse_interaction.reset(); }
    BorderMouseActionPtr  getMouseInteraction() { return mouse_interaction; }

private:
    BorderView();
    ~BorderView() {}

    static BorderView * mpThis;
    Configuration     * config;
    WeakBorderPtr       border;
    QPointF             mousePos;             // used by menu
    bool                debugMouse;

    BorderMouseActionPtr   mouse_interaction;    // used by menu

};

#endif
