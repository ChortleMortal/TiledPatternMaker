#pragma once
#ifndef TRANSPARENT_WIDGET_H
#define TRANSPARENT_WIDGET_H

#include "gui/widgets/image_widget.h"

class TransparentImageWidget : public ImageWidget
{
public:
    TransparentImageWidget(QString name);
    virtual ~TransparentImageWidget(){}

    virtual void setContentSize(QSize sz) override;

    virtual QString gettypename() override { return "transp"; }

protected:
    void paintEvent(QPaintEvent * event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override;

private:
    QPoint  oldPos;
    QString title;
    bool    dragging;
    bool    onTop;
};

#endif // TRANSPARENTWIDGET_H
