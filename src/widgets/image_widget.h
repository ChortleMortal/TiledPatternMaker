#pragma once
#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include <QLabel>
#include <QMouseEvent>

typedef std::shared_ptr<class Map>  MapPtr;

class ImageWidget : public QLabel
{
    Q_OBJECT

public:
    ImageWidget();
    virtual ~ImageWidget();

    void keyPressEvent(QKeyEvent *k) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void sig_keyPressed(QKeyEvent *);

public slots:
    void slot_closeMe();

private:
    bool    onTop;
};

#endif
