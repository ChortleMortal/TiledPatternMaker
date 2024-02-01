#pragma once
#ifndef TRANSPARENT_WIDGET_H
#define TRANSPARENT_WIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include "widgets/image_widget.h"

typedef std::shared_ptr<class Map>  MapPtr;


class TransparentImageWidget : public ImageWidget
{
public:
    TransparentImageWidget(QString name);
    virtual ~TransparentImageWidget(){}

protected:
    void paintEvent(QPaintEvent * event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override;

private:
    QPoint  oldPos;
    bool    onTop;
    QString title;
    bool    dragging;
};

#endif // TRANSPARENTWIDGET_H
